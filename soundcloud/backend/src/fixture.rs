//! Local HTTP fixtures compiled only by cargo test. Never in the release binary.
use crate::{api, catalog, player};
use serde_json::{json,Value};
use std::{io::{Read,Write},net::{TcpListener,TcpStream},path::{Path,PathBuf},sync::{Arc,Mutex,atomic::{AtomicBool,AtomicUsize,Ordering}},time::Duration};
struct Server {url:reqwest::Url,requests:Arc<Mutex<Vec<String>>>,tokens:Arc<AtomicUsize>,stop:Arc<AtomicBool>,thread:Option<std::thread::JoinHandle<()>>}
fn reply(stream:&mut TcpStream,status:u16,body:&[u8]){let head=format!("HTTP/1.1 {status} OK\r\nContent-Length: {}\r\nConnection: close\r\n\r\n",body.len());let _=stream.write_all(head.as_bytes());let _=stream.write_all(body);}
fn asset(name:&str)->Vec<u8>{std::fs::read(Path::new(env!("CARGO_MANIFEST_DIR")).join("../../qa/media").join(name)).unwrap()}
impl Server {
    fn start()->Self {
        let listener=TcpListener::bind("127.0.0.1:0").unwrap();listener.set_nonblocking(true).unwrap();
        let origin=format!("http://{}/",listener.local_addr().unwrap());let base=origin.clone();
        let requests=Arc::new(Mutex::new(Vec::new()));let seen=requests.clone();let tokens=Arc::new(AtomicUsize::new(0));let token_count=tokens.clone();let stop=Arc::new(AtomicBool::new(false));let ended=stop.clone();
        let thread=std::thread::spawn(move||while !ended.load(Ordering::SeqCst) {
            let (mut stream,_)=match listener.accept(){Ok(s)=>s,Err(_)=>{std::thread::sleep(Duration::from_millis(2));continue;}};
            stream.set_read_timeout(Some(Duration::from_secs(3))).unwrap();let mut data=Vec::new();let mut buf=[0;4096];
            loop {let Ok(n)=stream.read(&mut buf)else{break;};if n==0{break;}data.extend_from_slice(&buf[..n]);
                if let Some(h)=data.windows(4).position(|w|w==b"\r\n\r\n"){
                    let header=String::from_utf8_lossy(&data[..h]);let len=header.lines().find_map(|l|l.to_ascii_lowercase().strip_prefix("content-length:").map(str::trim).and_then(|x|x.parse::<usize>().ok())).unwrap_or(0);
                    if data.len()>=h+4+len{break;}
                }
                if data.len()>65536{break;}
            }
            let request=String::from_utf8_lossy(&data).into_owned();let target=request.split_whitespace().nth(1).unwrap_or("").to_owned();seen.lock().unwrap().push(request.clone());
            let object=if target=="/oauth/token"{let i=token_count.fetch_add(1,Ordering::SeqCst);std::thread::sleep(Duration::from_millis(50));json!({"access_token":format!("FAKE-TOKEN-{i}"),"refresh_token":format!("FAKE-REFRESH-{i}"),"expires_in":3600})}
                else if target=="/tracks/soundcloud:tracks:1"||target=="/tracks/soundcloud:tracks:3"{json!({"urn":"soundcloud:tracks:1","access":"playable","streamable":true})}
                else if target=="/tracks/soundcloud:tracks:2"{json!({"urn":"soundcloud:tracks:2","access":"preview","streamable":true})}
                else if target=="/tracks/soundcloud:tracks:1/streams"{json!({"hls_aac_160_url":format!("{base}media/aac.m3u8"),"hls_mp3_128_url":format!("{base}media/mp3.m3u8"),"preview_mp3_128_url":"https://evil.test/preview"})}
                else if target=="/tracks/soundcloud:tracks:3/streams"{json!({"hls_mp3_128_url":format!("{base}media/mp3.m3u8")})}
                else if target.starts_with("/playlists/"){
                    let start=if target.contains("cursor=next"){51}else{1};let count=if start==1{50}else{2};let rows=(start..start+count).map(|n|json!({"urn":format!("soundcloud:tracks:{n}"),"title":format!("日本語 ไทย 한국어 中文 {n}"),"access":"playable","duration":3000,"user":{"username":"Uploader"}})).collect::<Vec<_>>();
                    json!({"collection":rows,"next_href":if start==1{json!("https://api.soundcloud.com/playlists/soundcloud:playlists:1/tracks?cursor=next")}else{Value::Null}})
                }else if target=="/media/aac.m3u8" {reply(&mut stream,200,b"#EXTM3U\n#EXTINF:30,\na.aac\n#EXT-X-ENDLIST\n");continue;}
                else if target=="/media/mp3.m3u8" {reply(&mut stream,200,b"#EXTM3U\n#EXTINF:15,\nfirst.mp3\n#EXTINF:15,\nsecond.mp3\n#EXT-X-ENDLIST\n");continue;}
                else if target=="/media/a.aac"{reply(&mut stream,200,&asset("test.aac"));continue;}
                else if target=="/media/a.mp3"{reply(&mut stream,200,&asset("korea.mp3"));continue;}
                else if target=="/media/first.mp3"||target=="/media/second.mp3"{let file=asset("korea.mp3");let split=file.len()/2;reply(&mut stream,200,if target.contains("first"){&file[..split]}else{&file[split..]});continue;}
                else {reply(&mut stream,404,b"{}");continue;};
            reply(&mut stream,200,object.to_string().as_bytes());
        });
        Self{url:reqwest::Url::parse(&origin).unwrap(),requests,tokens,stop,thread:Some(thread)}
    }
}
impl Drop for Server{fn drop(&mut self){self.stop.store(true,Ordering::SeqCst);if let Some(t)=self.thread.take(){t.join().unwrap();}}}
fn credentials(server:&Server)->(Arc<api::Api>,PathBuf) {
    static SERIAL:AtomicUsize=AtomicUsize::new(0);let stamp=std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_nanos();let path=std::env::temp_dir().join(format!("nxt-sc-test-{stamp}-{}",SERIAL.fetch_add(1,Ordering::SeqCst)));std::fs::create_dir(&path).unwrap();
    let script=path.join("yt-dlp");
    let text=r###"#!/usr/bin/env python3
import sys,json,urllib.parse
if '--version' in sys.argv:print('2026.08.19');raise SystemExit
u=urllib.parse.urlsplit(sys.argv[-1]);q=urllib.parse.parse_qs(u.query);kind=u.path
resource=q.get('resource',[''])[0];offset=int(q.get('offset',['0'])[0])
if kind=='play':
 if resource=='2':print('NXT:track_restricted',file=sys.stderr);raise SystemExit(1)
 if resource in ('4','5','6'):
  host='playback.media-streaming.soundcloud.cloud' if resource!='5' else 'playback.media-streaming.soundcloud.cloud.evil.test'
  print(json.dumps({'id':resource,'formats':[{'format_id':'hls_aac_96k','protocol':'m3u8_native','url':'https://'+host+'/test.m3u8','preference':-10 if resource=='6' else None}]}));raise SystemExit
 codec='aac' if resource=='1' else 'mp3'
 print(json.dumps({'id':resource,'formats':[{'format_id':'hls_'+codec+'_160k','protocol':'m3u8_native','url':'ORIGINmedia/'+codec+'.m3u8'}]}))
else:
 rows=[{'id':str(i),'title':'Việt 日本語 ไทย 한국어 中文 '+str(i),'uploader':'Uploader','duration':90} for i in range(offset+1,min(offset+21,53))]
 print(json.dumps({'entries':rows,'nxt_kind':kind,'nxt_has_next':offset+20<52}))
"###.replace("ORIGIN",server.url.as_str());
    std::fs::write(&script,text).unwrap();std::fs::set_permissions(&script,std::fs::Permissions::from_mode(0o755)).unwrap();
    (Arc::new(api::Api::fixture(script,path.clone())),path)
}
use std::os::unix::fs::PermissionsExt;
#[tokio::test]async fn public_cloud_cdn_is_admitted_without_loosening_preview_or_host_checks(){
    let server=Server::start();let (api,path)=credentials(&server);
    let streams=api.streams("soundcloud:tracks:4").await.unwrap();
    assert_eq!(streams["hls_aac_160_url"],"https://playback.media-streaming.soundcloud.cloud/test.m3u8");
    assert_eq!(api.streams("soundcloud:tracks:5").await.unwrap_err(),"audio_format_unsupported");
    assert_eq!(api.streams("soundcloud:tracks:6").await.unwrap_err(),"audio_format_unsupported");
    std::fs::remove_dir_all(path).unwrap();
}
#[tokio::test(flavor="multi_thread",worker_threads=2)]async fn mp3_hls_prefetch_crosses_segments_without_pcm_loss(){
    let server=Server::start();let (api,path)=credentials(&server);let control=Arc::new(player::Control::new());control.epoch.store(12,Ordering::SeqCst);
    let (tx,rx)=std::os::unix::net::UnixDatagram::pair().unwrap();tx.set_nonblocking(true).unwrap();rx.set_read_timeout(Some(Duration::from_secs(10))).unwrap();let sink=Arc::new(player::Sink::fixture(tx,control.clone()));
    let receiver=std::thread::spawn(move||{let mut frames=0;let mut packet=[0;8208];loop{let n=rx.recv(&mut packet).unwrap();assert_eq!(u32::from_le_bytes(packet[..4].try_into().unwrap()),12);if packet[12]==1{break;}assert_eq!(u32::from_le_bytes(packet[8..12].try_into().unwrap()),44100);frames+=(n-16)/8;}frames});
    player::run(api,sink,control,json!({"uri":"soundcloud:tracks:3","stream_id":12})).await.unwrap();let received=receiver.join().unwrap();let mut direct=0;
    player::decode_fixture_mp3(asset("korea.mp3"),|_,samples|{direct+=samples.len()/2;Ok(())}).unwrap();assert_eq!(received,direct);assert!(received>44100);
    let requests=server.requests.lock().unwrap();assert!(requests.iter().any(|s|s.contains("GET /media/first.mp3")));assert!(requests.iter().any(|s|s.contains("GET /media/second.mp3")));drop(requests);std::fs::remove_dir_all(path).unwrap();
}
#[tokio::test(flavor="multi_thread",worker_threads=2)]async fn extractor_playlist_pagination_keeps_fifty_two_rows_and_cache(){
    let server=Server::start();let (api,path)=credentials(&server);let pages=Arc::new(tokio::sync::Mutex::new(std::collections::HashMap::<String,Value>::new()));
    let mut ids=Vec::new();for page in 0..3{let v=json!({"cmd":"playlist_tracks","id":"soundcloud:playlists:1","offset":page*20,"request_id":page});let result=catalog(api.clone(),pages.clone(),&v).await.unwrap();for row in result["items"].as_array().unwrap(){ids.push(row["uri"].as_str().unwrap().to_owned());}assert_eq!(result["has_next"],page<2);}
    assert_eq!(ids.len(),52);assert_eq!(ids[0],"soundcloud:tracks:1");assert_eq!(ids[51],"soundcloud:tracks:52");
    let again=catalog(api,pages,&json!({"cmd":"playlist_tracks","id":"soundcloud:playlists:1","offset":20,"request_id":99})).await.unwrap();assert_eq!(again["request_id"],99);
    assert!(!path.join("credentials.json").exists());assert_eq!(server.tokens.load(Ordering::SeqCst),0);std::fs::remove_dir_all(path).unwrap();
}
#[tokio::test(flavor="multi_thread",worker_threads=2)]async fn actual_http_hls_aac_to_pcm_and_restricted_track_rejection(){
    let server=Server::start();let (api,path)=credentials(&server);let control=Arc::new(player::Control::new());control.epoch.store(9,Ordering::SeqCst);
    let (tx,rx)=std::os::unix::net::UnixDatagram::pair().unwrap();tx.set_nonblocking(true).unwrap();rx.set_read_timeout(Some(Duration::from_secs(10))).unwrap();
    let sink=Arc::new(player::Sink::fixture(tx,control.clone()));let received=std::thread::spawn(move||{let mut frames=0;let mut peak=0f32;let mut b=[0u8;8208];loop{let n=rx.recv(&mut b).unwrap();assert_eq!(u32::from_le_bytes(b[..4].try_into().unwrap()),9);if u32::from_le_bytes(b[12..16].try_into().unwrap())==1{break;}let samples=u32::from_le_bytes(b[4..8].try_into().unwrap())as usize;assert_eq!(n,16+samples*4);let rate=u32::from_le_bytes(b[8..12].try_into().unwrap());assert!([44100,48000].contains(&rate));frames+=samples/2;for s in b[16..n].chunks_exact(4){peak=peak.max(f32::from_le_bytes(s.try_into().unwrap()).abs());}}(frames,peak)});
    player::run(api.clone(),sink.clone(),control.clone(),json!({"uri":"soundcloud:tracks:1","stream_id":9})).await.unwrap();let (frames,peak)=received.join().unwrap();assert!(frames>44100&&peak>0.01);
    assert_eq!(player::run(api,sink,control,json!({"uri":"soundcloud:tracks:2","stream_id":9})).await.unwrap_err(),"track_restricted");let req=server.requests.lock().unwrap();assert_eq!(server.tokens.load(Ordering::SeqCst),0);assert!(!req.iter().any(|r|r.to_lowercase().contains("authorization:")));assert!(req.iter().any(|r|r.contains("GET /media/a.aac")));assert!(!req.iter().any(|r|r.contains("GET /media/a.mp3")));drop(req);std::fs::remove_dir_all(path).unwrap();
}
