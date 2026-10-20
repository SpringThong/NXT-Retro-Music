use super::{api::{Api,Result,urn},hls};
use serde_json::{Value,json};
use std::{io::{self,Read},os::fd::FromRawFd,os::unix::net::UnixDatagram,sync::{Arc,Mutex,atomic::{AtomicBool,AtomicU32,Ordering}},time::Duration};
use symphonia::core::{audio::{AudioBufferRef,SampleBuffer},codecs::{CodecParameters,DecoderOptions,CODEC_TYPE_AAC},errors::Error,formats::Packet,io::MediaSourceStream,probe::Hint};

pub struct Control {pub epoch:AtomicU32,pub paused:AtomicBool}
impl Control {
    pub fn new()->Self{Self{epoch:AtomicU32::new(0),paused:AtomicBool::new(false)}}
    pub fn alive(&self,epoch:u32)->bool{epoch!=0&&self.epoch.load(Ordering::SeqCst)==epoch}
    fn wait(&self,epoch:u32)->bool {
        while self.alive(epoch)&&self.paused.load(Ordering::SeqCst){std::thread::sleep(Duration::from_millis(10));}self.alive(epoch)
    }
}
pub struct Sink {socket:UnixDatagram,control:Arc<Control>}
impl Sink {
    #[cfg(test)]pub fn fixture(socket:UnixDatagram,control:Arc<Control>)->Self{Self{socket,control}}
    pub fn inherited(control:Arc<Control>)->Result<Self>{
        let fd=std::env::var("NXT_SOUNDCLOUD_PCM_FD").ok().and_then(|s|s.parse::<i32>().ok()).filter(|n|*n==3).ok_or("pcm_channel_missing")?;
        let socket=unsafe{UnixDatagram::from_raw_fd(fd)};socket.set_nonblocking(true).map_err(|_|"pcm_channel_invalid")?;
        Ok(Self{socket,control})
    }
    fn packet(&self,data:&[u8],epoch:u32)->Result<()> {
        let mut stalled=std::time::Instant::now();
        loop {
            if !self.control.alive(epoch){return Err("cancelled".into());}
            if self.control.paused.load(Ordering::SeqCst){stalled=std::time::Instant::now();std::thread::sleep(Duration::from_millis(10));continue;}
            match self.socket.send(data) {
                Ok(n) if n==data.len()=>return Ok(()),
                Err(e) if matches!(e.kind(),io::ErrorKind::WouldBlock|io::ErrorKind::Interrupted)=>{
                    if self.control.paused.load(Ordering::SeqCst){stalled=std::time::Instant::now();}
                    if stalled.elapsed()>Duration::from_secs(20){return Err("pcm_channel_stalled".into());}
                    std::thread::sleep(Duration::from_millis(4));
                },_=>return Err("pcm_channel_closed".into()),
            }
        }
    }
    pub fn send(&self,epoch:u32,rate:u32,samples:&[f32])->Result<()> {
        if samples.len()%2!=0||!(8000..=192000).contains(&rate){return Err("audio_format_unsupported".into());}
        for chunk in samples.chunks(2048){
            let mut packet=Vec::with_capacity(16+chunk.len()*4);
            packet.extend(epoch.to_le_bytes());packet.extend((chunk.len() as u32).to_le_bytes());
            packet.extend(rate.to_le_bytes());packet.extend(0u32.to_le_bytes());
            for &v in chunk{packet.extend(if v.is_finite(){v.clamp(-1.,1.)}else{0.}.to_le_bytes());}
            self.packet(&packet,epoch)?;
        }Ok(())
    }
    fn eof(&self,epoch:u32)->Result<()> {
        let mut bytes=Vec::new();bytes.extend(epoch.to_le_bytes());bytes.extend(0u32.to_le_bytes());bytes.extend(0u32.to_le_bytes());bytes.extend(1u32.to_le_bytes());self.packet(&bytes,epoch)
    }
}
pub fn stereo(audio:AudioBufferRef<'_>)->Result<(u32,Vec<f32>)> {
    let spec=*audio.spec();let channels=spec.channels.count();
    if !(1..=2).contains(&channels)||!(8000..=192000).contains(&spec.rate){return Err("audio_format_unsupported".into());}
    let mut output=SampleBuffer::<f32>::new(audio.capacity() as u64,spec);output.copy_interleaved_ref(audio);
    let samples=if channels==2{output.samples().to_vec()}else{output.samples().iter().flat_map(|&s|[s,s]).collect()};
    Ok((spec.rate,samples))
}
fn decode_media(source:Box<dyn symphonia::core::io::MediaSource>,extension:&str,mut output:impl FnMut(u32,Vec<f32>)->Result<()>)->Result<()> {
    let mut hint=Hint::new();hint.with_extension(extension);
    let media=MediaSourceStream::new(source,Default::default());
    let probed=symphonia::default::get_probe().format(&hint,media,&Default::default(),&Default::default()).map_err(|_|"audio_format_unsupported")?;
    let mut format=probed.format;let track=format.default_track().ok_or("audio_format_unsupported")?;
    let id=track.id;let mut decoder=symphonia::default::get_codecs().make(&track.codec_params,&Default::default()).map_err(|_|"audio_format_unsupported")?;
    let mut failures=0;
    loop {
        let packet=match format.next_packet(){Ok(p)=>p,Err(Error::IoError(e)) if e.kind()==io::ErrorKind::UnexpectedEof=>break,Err(_)=>return Err("decode_failed".into())};
        if packet.track_id()!=id{continue;}
        match decoder.decode(&packet) {
            Ok(audio)=>{failures=0;let (rate,samples)=stereo(audio)?;output(rate,samples)?;},
            Err(Error::DecodeError(_))=>{failures+=1;if failures>8{return Err("decode_failed".into());}},
            Err(_)=>return Err("decode_failed".into()),
        }
    }Ok(())
}
#[cfg(test)]pub fn decode_fixture_mp3(bytes:Vec<u8>,output:impl FnMut(u32,Vec<f32>)->Result<()>)->Result<()>{decode_media(Box::new(io::Cursor::new(bytes)),"mp3",output)}
// SoundCloud AAC HLS may contain packed ADTS instead of fragmented MP4.
// Only AAC-LC, one raw data block and 1/2 channels are supported here.
fn decode_adts(mut source:impl Read,mut output:impl FnMut(u32,Vec<f32>)->Result<()>)->Result<()> {
    let mut decoder=None;let mut format=None;let mut ts=0u64;
    loop {
        let mut first=[0u8;3];let mut got=0;
        while got<3{let n=source.read(&mut first[got..]).map_err(|_|"decode_failed")?;if n==0{if got==0{return Ok(());}return Err("decode_failed".into());}got+=n;}
        if &first==b"ID3" {
            let mut rest=[0u8;7];source.read_exact(&mut rest).map_err(|_|"decode_failed")?;
            if rest[3..].iter().any(|c|c&128!=0){return Err("decode_failed".into());}
            let length=rest[3..].iter().fold(0u64,|n,b|(n<<7)|*b as u64);
            if length>1024*1024{return Err("response_too_large".into());}
            let count=io::copy(&mut source.by_ref().take(length),&mut io::sink()).map_err(|_|"decode_failed")?;
            if count!=length{return Err("decode_failed".into());}continue;
        }
        let mut h=[0u8;7];h[..3].copy_from_slice(&first);source.read_exact(&mut h[3..]).map_err(|_|"decode_failed")?;
        if h[0]!=255||h[1]&0xf6!=0xf0||h[6]&3!=0{return Err("audio_format_unsupported".into());}
        let profile=(h[2]>>6)+1;let rate_index=(h[2]>>2)&15;let channels=((h[2]&1)<<2)|(h[3]>>6);
        const RATES:[u32;13]=[96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350];
        let rate=*RATES.get(rate_index as usize).ok_or("audio_format_unsupported")?;
        if profile!=2||!(1..=2).contains(&channels){return Err("audio_format_unsupported".into());}
        let length=((h[3]&3)as usize)<<11|(h[4]as usize)<<3|(h[5]as usize)>>5;
        let header=if h[1]&1==1{7}else{9};if length<header||length>8192{return Err("decode_failed".into());}
        if header==9{let mut crc=[0;2];source.read_exact(&mut crc).map_err(|_|"decode_failed")?;}
        let this=(profile,rate_index,channels);
        if decoder.is_none(){
            let mut params=CodecParameters::new();params.for_codec(CODEC_TYPE_AAC).with_sample_rate(rate);
            params.extra_data=Some(vec![(profile<<3)|(rate_index>>1),((rate_index&1)<<7)|(channels<<3)].into_boxed_slice());
            decoder=Some(symphonia::default::get_codecs().make(&params,&DecoderOptions::default()).map_err(|_|"audio_format_unsupported")?);format=Some(this);
        }else if format!=Some(this){return Err("audio_format_unsupported".into());}
        let mut payload=vec![0;length-header];source.read_exact(&mut payload).map_err(|_|"decode_failed")?;
        let packet=Packet::new_from_boxed_slice(0,ts,1024,payload.into_boxed_slice());ts+=1024;
        let audio=decoder.as_mut().unwrap().decode(&packet).map_err(|_|"decode_failed")?;
        let (rate,samples)=stereo(audio)?;output(rate,samples)?;
    }
}
pub async fn run(api:Arc<Api>,sink:Arc<Sink>,control:Arc<Control>,v:Value)->Result<()> {
    let id=v["uri"].as_str().ok_or("track_invalid")?.to_owned();if !urn(&id,"tracks"){return Err("track_invalid".into());}
    let epoch=v["stream_id"].as_u64().filter(|n|*n>0&&*n<=u32::MAX as u64).ok_or("track_invalid")? as u32;
    let position=v["position_ms"].as_u64().unwrap_or(0).min(i32::MAX as u64) as u32;
    let streams=api.streams(&id).await?;
    let token=String::new();let http=hls::client()?;
    let mut last="track_unavailable".to_string();
    for (field,codec,bitrate) in [("hls_aac_160_url","AAC",160),("hls_mp3_128_url","MP3",128)] {
        if !control.alive(epoch){return Err("cancelled".into());}
        let Some(url)=streams[field].as_str().and_then(|s|reqwest::Url::parse(s).ok())else{continue;};
        let segments=match hls::load(&http,url,&token).await{Ok(s)=>s,Err(e)=>{last=e;continue;}};
        let (index,skip)=hls::seek_segment(&segments,position);
        if index==segments.len(){sink.eof(epoch)?;return Ok(());}
        let c=control.clone();let error=Arc::new(Mutex::new(None));let error_read=error.clone();
        let source=hls::Source::new(http.clone(),token.clone(),segments,index,Box::new(move||c.alive(epoch)),error);
        let s=sink.clone();let c=control.clone();let uri=id.clone();let sent=Arc::new(AtomicBool::new(false));let emitted=sent.clone();
        let decoded=tokio::task::spawn_blocking(move|| {
            let mut skipped=0.;let mut source=source;
            let mut output=|rate:u32,samples:Vec<f32>|->Result<()> {
                if !c.wait(epoch){return Err("cancelled".into());}
                let count=samples.len()/2;let remaining=(skip-skipped).max(0.);
                let omit=((remaining*rate as f64).ceil()as usize).min(count);skipped+=omit as f64/rate as f64;
                if omit==count{return Ok(());}
                if !emitted.swap(true,Ordering::SeqCst){crate::emit(json!({"event":"playing","uri":uri,"stream_id":epoch,"position_ms":position,"sample_rate":rate,"codec":codec,"bitrate_kbps":bitrate}));}
                s.send(epoch,rate,&samples[omit*2..])
            };
            if codec=="AAC"&&source.adts().map_err(|_|"decode_failed")? {decode_adts(source,&mut output)}
            else{decode_media(Box::new(source),if codec=="AAC"{"mp4"}else{"mp3"},&mut output)}
        }).await.map_err(|_|"decode_failed")?;
        if !control.alive(epoch){return Err("cancelled".into());}
        if let Some(reason)=error_read.lock().unwrap().take(){return Err(reason);}
        match decoded {
            Ok(()) if sent.load(Ordering::SeqCst)=>{sink.eof(epoch)?;return Ok(());},
            Ok(())=>last="track_unavailable".into(),
            Err(e)=>{if sent.load(Ordering::SeqCst)||!matches!(e.as_str(),"audio_format_unsupported"|"decode_failed"){return Err(e);}last=e;}
        }
    }
    Err(last)
}
#[cfg(test)]mod tests {
    use super::*;
    #[test]fn genuine_mp3_and_adts_decode_to_stereo_pcm(){
        let root=std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../qa/media");
        let mut frames=0;let mut peak=0f32;
        let mut output=|rate,u:Vec<f32>|{assert_eq!(rate,44100);assert_eq!(u.len()%2,0);frames+=u.len()/2;peak=u.iter().fold(peak,|p,v|p.max(v.abs()));Ok(())};
        decode_media(Box::new(std::fs::File::open(root.join("korea.mp3")).unwrap()),"mp3",&mut output).unwrap();
        assert!(frames>44100&&peak>0.01);frames=0;peak=0.;
        decode_adts(std::fs::File::open(root.join("test.aac")).unwrap(),|rate,u|{assert!([44100,48000].contains(&rate));frames+=u.len()/2;peak=u.iter().fold(peak,|p,v|p.max(v.abs()));Ok(())}).unwrap();assert!(frames>44100&&peak>0.01);
    }
    #[test]fn pcm_header_stereo_and_cancel_backpressure(){
        let (tx,rx)=UnixDatagram::pair().unwrap();tx.set_nonblocking(true).unwrap();let control=Arc::new(Control::new());control.epoch.store(7,Ordering::SeqCst);
        let sink=Sink{socket:tx,control:control.clone()};sink.send(7,48000,&[0.25,-0.5]).unwrap();let mut b=[0;24];assert_eq!(rx.recv(&mut b).unwrap(),24);assert_eq!(&b[..16],&[7,0,0,0,2,0,0,0,128,187,0,0,0,0,0,0]);
        assert_eq!(f32::from_le_bytes(b[16..20].try_into().unwrap()),0.25);
        control.epoch.store(8,Ordering::SeqCst);assert_eq!(sink.send(7,44100,&[0.,0.]).unwrap_err(),"cancelled");
    }
}
