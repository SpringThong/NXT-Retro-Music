//! Finite HLS audio playlists. Encrypted/live/foreign-host media is rejected.
use super::api::{self, Result, media_url};
use reqwest::{Client, Url};
use std::{io::{self, Read, Seek, SeekFrom}, time::Duration};
use symphonia::core::io::MediaSource;

#[derive(Clone, Debug)] pub struct Segment {
    pub url: Url, pub duration: f64, pub range: Option<(u64,u64)>, pub map: Option<(Url,Option<(u64,u64)>)>,
}
#[derive(Clone, Debug)] pub enum Manifest { Master(Vec<Url>), Media(Vec<Segment>) }
fn attributes(s: &str) -> Vec<(&str, &str)> {
    let mut output=Vec::new();let mut begin=0;let mut quoted=false;
    for (i,c) in s.char_indices().chain(std::iter::once((s.len(),','))) {
        if c=='"' {quoted=!quoted;} else if c==','&&!quoted {
            if let Some((k,v))=s[begin..i].split_once('=') {output.push((k.trim(),v.trim().trim_matches('"')));}
            begin=i+1;
        }
    }
    output
}
fn absolute(base: &Url, path: &str) -> Result<Url> {
    let url=base.join(path).map_err(|_|"hls_invalid")?;
    if !media_url(&url) {return Err("media_url_invalid".into());}Ok(url)
}
fn byte_range(s: &str, previous_end: u64) -> Result<(u64,u64)> {
    let (length,offset)=s.split_once('@').map_or((s,None),|(n,o)|(n,Some(o)));
    let n=length.parse::<u64>().map_err(|_|"hls_invalid")?;
    let start=offset.map_or(Ok(previous_end),|x|x.parse::<u64>().map_err(|_|"hls_invalid"))?;
    if n==0||n>8*1024*1024||start.checked_add(n).is_none() {return Err("hls_invalid".into());}Ok((start,n))
}
pub fn parse(base: &Url, text: &str) -> Result<Manifest> {
    if text.len()>512*1024||!text.trim_start_matches('\u{feff}').starts_with("#EXTM3U") {return Err("hls_invalid".into());}
    let mut segments=Vec::new();let mut variants=Vec::new();let mut next_variant=false;
    let mut duration=None;let mut pending_range:Option<String>=None;let mut map=None;
    let mut previous_url:Option<Url>=None;let mut previous_end=0;let mut end=false;let mut total=0.;
    for line in text.lines().map(str::trim).filter(|s|!s.is_empty()) {
        if let Some(a)=line.strip_prefix("#EXT-X-KEY:") {
            if !attributes(a).iter().any(|(k,v)| *k=="METHOD"&&*v=="NONE") {return Err("hls_encrypted_unsupported".into());}
        } else if line.starts_with("#EXT-X-STREAM-INF:") {next_variant=true;}
        else if let Some(n)=line.strip_prefix("#EXTINF:") {
            let d=n.split(',').next().unwrap_or("").parse::<f64>().map_err(|_|"hls_invalid")?;
            if !d.is_finite()||d<=0.||d>600. {return Err("hls_invalid".into());}duration=Some(d);
        } else if let Some(r)=line.strip_prefix("#EXT-X-BYTERANGE:") {pending_range=Some(r.to_owned());}
        else if let Some(a)=line.strip_prefix("#EXT-X-MAP:") {
            let attrs=attributes(a);let u=attrs.iter().find(|(k,_)|*k=="URI").ok_or("hls_invalid")?.1;
            let r=attrs.iter().find(|(k,_)|*k=="BYTERANGE").map(|(_,v)|byte_range(v,0)).transpose()?;
            map=Some((absolute(base,u)?,r));
        } else if line=="#EXT-X-ENDLIST" {end=true;}
        else if !line.starts_with('#') {
            let url=absolute(base,line)?;
            if next_variant {variants.push(url);next_variant=false;continue;}
            let d=duration.take().ok_or("hls_invalid")?;
            let range=pending_range.take().map(|r| {
                if !r.contains('@')&&previous_url.as_ref()!=Some(&url){return Err("hls_invalid".into());}
                byte_range(&r,previous_end)
            }).transpose()?;
            previous_end=range.map_or(0,|(s,n)|s+n);previous_url=Some(url.clone());
            total+=d;if total>48.*3600.||segments.len()>=30000 {return Err("hls_invalid".into());}
            segments.push(Segment{url,duration:d,range,map:map.clone()});
        }
    }
    if !variants.is_empty()&&segments.is_empty() {Ok(Manifest::Master(variants))}
    else if !segments.is_empty()&&end&&duration.is_none() {Ok(Manifest::Media(segments))}
    else {Err("hls_live_unsupported".into())}
}
pub fn client() -> Result<Client> {
    let mut builder=Client::builder();
    // Optional extra trust root for development/company HTTPS proxies. Keep
    // normal TLS verification and public roots; never accept invalid certs.
    if let Some(path)=std::env::var_os("NXT_SOUNDCLOUD_CA") {
        let bytes=std::fs::read(path).map_err(|_|"network_ca_invalid")?;
        if bytes.len()>128*1024{return Err("network_ca_invalid".into());}
        let certificate=reqwest::Certificate::from_pem(&bytes).map_err(|_|"network_ca_invalid")?;
        builder=builder.add_root_certificate(certificate);
    }
    builder.timeout(Duration::from_secs(20)).connect_timeout(Duration::from_secs(8))
        .redirect(reqwest::redirect::Policy::custom(|a| {
            if a.previous().len()>=4||!media_url(a.url()) {a.stop()} else {a.follow()}
        })).user_agent("NXT-Retro-Music/1.2").build().map_err(|_|"network_error".into())
}
pub async fn fetch(http: &Client, url: &Url, range: Option<(u64,u64)>, token: &str, max: usize) -> Result<Vec<u8>> {
    if !media_url(url) {return Err("media_url_invalid".into());}
    let mut request=http.get(url.clone());
    // Never send the app secret. OAuth is restricted to the official API host;
    // CDN URLs carry their own short-lived signature from /streams.
    if !token.is_empty()&&api::api_url(url) {request=request.header("Authorization",format!("OAuth {token}"));}
    if let Some((start,len))=range {request=request.header("Range",format!("bytes={start}-{}",start+len-1));}
    let res=request.send().await.map_err(|_|"network_error")?;
    if !res.status().is_success() {return Err(format!("media_http_{}",res.status().as_u16()));}
    if range.is_some()&&res.status().as_u16()!=206 {return Err("media_range_unsupported".into());}
    let bytes=api::bounded_bytes(res,max).await?;
    if let Some((_,len))=range {if bytes.len()!=len as usize{return Err("media_range_invalid".into());}}
    Ok(bytes)
}
pub async fn load(http:&Client,mut url:Url,token:&str)->Result<Vec<Segment>> {
    for _ in 0..4 {
        let bytes=fetch(http,&url,None,token,512*1024).await?;
        let text=std::str::from_utf8(&bytes).map_err(|_|"hls_invalid")?;
        match parse(&url,text)? {Manifest::Media(s)=>return Ok(s),Manifest::Master(v)=>url=v.into_iter().next().ok_or("hls_invalid")?}
    }
    Err("hls_invalid".into())
}
pub fn seek_segment(segments:&[Segment],ms:u32)->(usize,f64) {
    let wanted=ms as f64/1000.;let mut offset=0.;
    for (i,s) in segments.iter().enumerate(){if offset+s.duration>wanted {return(i,(wanted-offset).max(0.));}offset+=s.duration;}
    (segments.len(),0.)
}
// Current segment plus one prefetched segment, bounded in RAM. Read spans
// boundaries so the MP3 reservoir and AAC decoder keep their state.
pub struct Source {
    http:Client,token:String,segments:Vec<Segment>,index:usize,current:std::io::Cursor<Vec<u8>>,
    map:Option<(Url,Option<(u64,u64)>)>,position:u64,handle:tokio::runtime::Handle,
    alive:Box<dyn Fn()->bool+Send+Sync>, failure:std::sync::Arc<std::sync::Mutex<Option<String>>>,
    prefetched:Option<tokio::task::JoinHandle<Result<Vec<u8>>>>,
}
async fn segment_bytes(http:Client,token:String,s:Segment,previous_map:Option<(Url,Option<(u64,u64)>)>)->Result<Vec<u8>>{
    let mut data=Vec::new();
    if s.map!=previous_map {if let Some((url,range))=&s.map {data=fetch(&http,url,*range,&token,1024*1024).await?;}}
    data.extend(fetch(&http,&s.url,s.range,&token,8*1024*1024).await?);Ok(data)
}
impl Source {
    pub fn new(http:Client,token:String,segments:Vec<Segment>,index:usize,alive:Box<dyn Fn()->bool+Send+Sync>,failure:std::sync::Arc<std::sync::Mutex<Option<String>>>)->Self {
        Self{http,token,segments,index,current:io::Cursor::new(Vec::new()),map:None,position:0,handle:tokio::runtime::Handle::current(),alive,failure,prefetched:None}
    }
    pub fn adts(&mut self)->io::Result<bool> {
        if self.current.get_ref().is_empty()&&!self.next()? {return Ok(false);}
        let bytes=self.current.get_ref();let mut offset=0;
        while bytes.get(offset..offset+3)==Some(b"ID3") {
            let Some(h)=bytes.get(offset..offset+10) else{return Ok(false);};
            if h[6..10].iter().any(|c|c&128!=0){return Ok(false);}
            let length=h[6..10].iter().fold(0usize,|n,b|(n<<7)|*b as usize);
            offset+=10+length;if offset>bytes.len(){return Ok(false);}
        }
        Ok(bytes.get(offset)==Some(&255)&&bytes.get(offset+1).is_some_and(|b|b&0xf6==0xf0))
    }
    fn next(&mut self)->io::Result<bool> {
        if !(self.alive)(){return Err(io::Error::new(io::ErrorKind::Interrupted,"cancelled"));}
        let Some(s)=self.segments.get(self.index) else{return Ok(false);};
        let data=if let Some(job)=self.prefetched.take(){self.handle.block_on(async {job.await.map_err(|_|"network_error".to_owned())?})}
            else{self.handle.block_on(segment_bytes(self.http.clone(),self.token.clone(),s.clone(),self.map.clone()))}
            .map_err(|e| {*self.failure.lock().unwrap()=Some(e);io::Error::other("stream fetch failed")})?;
        if !(self.alive)(){return Err(io::Error::new(io::ErrorKind::Interrupted,"cancelled"));}
        self.map=s.map.clone();self.index+=1;self.current=io::Cursor::new(data);
        if let Some(next)=self.segments.get(self.index){self.prefetched=Some(self.handle.spawn(segment_bytes(self.http.clone(),self.token.clone(),next.clone(),self.map.clone())));}
        Ok(true)
    }
}
impl Drop for Source{fn drop(&mut self){if let Some(job)=self.prefetched.take(){job.abort();}}}
impl Read for Source {
    fn read(&mut self,b:&mut [u8])->io::Result<usize> {
        if b.is_empty(){return Ok(0);}
        loop {let n=self.current.read(b)?;if n>0{self.position+=n as u64;return Ok(n);}if !self.next()?{return Ok(0);}}
    }
}
impl Seek for Source {
    fn seek(&mut self,p:SeekFrom)->io::Result<u64> {
        match p {SeekFrom::Current(0)=>Ok(self.position),_=>Err(io::Error::new(io::ErrorKind::Unsupported,"stream is not byte seekable"))}
    }
}
impl MediaSource for Source {fn is_seekable(&self)->bool{false}fn byte_len(&self)->Option<u64>{None}}
#[cfg(test)]mod tests {
    use super::*;
    fn base()->Url{Url::parse("https://cf-hls-media.sndcdn.com/path/list.m3u8?sig=x").unwrap()}
    #[test]fn finite_ranges_maps_seek_and_unicode_comments(){
        let Manifest::Media(s)=parse(&base(),"#EXTM3U\n# ไทย 日本語\n#EXT-X-MAP:URI=\"init.mp4\",BYTERANGE=\"20@0\"\n#EXTINF:2.5,\n#EXT-X-BYTERANGE:100@20\na.mp3\n#EXTINF:3.0,\n#EXT-X-BYTERANGE:100\na.mp3\n#EXT-X-ENDLIST\n").unwrap()else{panic!()};
        assert_eq!(s.len(),2);assert_eq!(s[1].range,Some((120,100)));assert_eq!(s[0].map.as_ref().unwrap().1,Some((0,20)));
        assert_eq!(seek_segment(&s,3000),(1,0.5));assert_eq!(seek_segment(&s,5500),(2,0.));
    }
    #[test]fn reject_live_encrypted_bad_ranges_foreign_hosts(){
        for text in ["#EXTM3U\n#EXTINF:2,\na.mp3", "#EXTM3U\n#EXT-X-KEY:METHOD=AES-128,URI=\"key\"\n", "#EXTM3U\n#EXTINF:NaN,\na.mp3\n#EXT-X-ENDLIST", "#EXTM3U\n#EXTINF:2,\nhttps://evil.test/a\n#EXT-X-ENDLIST", "#EXTM3U\n#EXTINF:2,\n#EXT-X-BYTERANGE:20\na.mp3\n#EXT-X-ENDLIST"]{assert!(parse(&base(),text).is_err());}
        let Manifest::Master(v)=parse(&base(),"#EXTM3U\n#EXT-X-STREAM-INF:BANDWIDTH=128000,CODECS=\"mp3\"\nsub.m3u8").unwrap()else{panic!()};assert_eq!(v[0].path(),"/path/sub.m3u8");
    }
}
