//! Public SoundCloud extraction via bundled yt-dlp; no personal API credentials.
use reqwest::Url;
use serde_json::{Value, json};
use std::{path::{Path, PathBuf}, time::Duration};

pub type Result<T> = std::result::Result<T, String>;
const MAX_JSON: usize = 4 * 1024 * 1024;
pub const PAGE: usize = 20;
pub fn urn(s: &str, kind: &str) -> bool {
    s.strip_prefix(&format!("soundcloud:{kind}:"))
        .is_some_and(|tail| !tail.is_empty() && tail.len() <= 32 && tail.bytes().all(|c| c.is_ascii_digit()))
}
pub fn api_url(url: &Url) -> bool {
    url.scheme() == "https" && url.host_str() == Some("api.soundcloud.com")
        && url.port_or_known_default() == Some(443) && url.username().is_empty() && url.password().is_none()
}
pub fn public_url(url: &Url) -> bool {
    url.scheme() == "https" && matches!(url.host_str(), Some("soundcloud.com" | "www.soundcloud.com" | "on.soundcloud.com"))
        && url.port_or_known_default() == Some(443) && url.username().is_empty() && url.password().is_none()
}
pub fn media_url(url: &Url) -> bool {
    #[cfg(test)] if url.scheme()=="http"&&url.host_str()==Some("127.0.0.1"){return true;}
    url.scheme() == "https" && url.port_or_known_default() == Some(443)
        && url.username().is_empty() && url.password().is_none()
        && url.host_str().is_some_and(|h| h == "sndcdn.com" || h.ends_with(".sndcdn.com")
            || h == "api.soundcloud.com" || h == "playback.media-streaming.soundcloud.cloud")
}
pub fn atomic_json(path: &Path, value: &Value) -> Result<()> {
    use std::os::unix::fs::OpenOptionsExt;
    use std::io::Write;
    let temp = path.with_extension("writing");
    let mut file = std::fs::OpenOptions::new().create(true).truncate(true).write(true).mode(0o600)
        .open(&temp).map_err(|_| "credentials_save_failed".to_string())?;
    std::fs::set_permissions(&temp, std::os::unix::fs::PermissionsExt::from_mode(0o600)).map_err(|_| "credentials_save_failed")?;
    file.write_all(serde_json::to_string(value).map_err(|_| "credentials_save_failed")?.as_bytes()).map_err(|_| "credentials_save_failed")?;
    file.sync_all().map_err(|_| "credentials_save_failed")?;
    std::fs::rename(&temp, path).map_err(|_| "credentials_save_failed".to_string())
}
// yt-dlp is bundled for ARM64, including Python. No personal API key is read.
pub struct Api { executable: PathBuf, plugins: PathBuf, cache: PathBuf }
struct ProcessGroup(i32);
unsafe extern "C" { fn kill(pid:i32,signal:i32)->i32; fn close(fd:i32)->i32; fn prctl(option:i32,...)->i32; }
impl Drop for ProcessGroup { fn drop(&mut self){unsafe{kill(-self.0,9);}} }
impl Api {
    #[cfg(test)] pub fn fixture(executable:PathBuf,plugins:PathBuf)->Self{Self{executable,cache:plugins.join("cache"),plugins}}
    pub fn new(path: PathBuf) -> Result<Self> {
        Ok(Self {
            executable: std::env::var_os("NXT_SOUNDCLOUD_YTDLP").map(PathBuf::from).unwrap_or_else(||"./bin/tg5040/yt-dlp".into()),
            plugins: std::env::var_os("NXT_SOUNDCLOUD_PLUGINS").map(PathBuf::from).unwrap_or_else(||"./res/soundcloud".into()),
            cache: path.with_file_name("yt-dlp-cache"),
        })
    }
    async fn execute(&self,args:&[String],limit:usize)->Result<Vec<u8>> {
        use tokio::{io::AsyncReadExt,process::Command};
        use std::{os::unix::process::CommandExt,process::Stdio};
        let mut cmd=Command::new(&self.executable);
        cmd.env("PYTHONDONTWRITEBYTECODE","1").args(args).stdin(Stdio::null()).stdout(Stdio::piped()).stderr(Stdio::piped()).kill_on_drop(true);
        cmd.as_std_mut().process_group(0);
        // The extractor must not inherit the audio channel or linger after its
        // helper dies. A process group also covers the PyInstaller child.
        unsafe{cmd.pre_exec(||{close(3);prctl(1,15,0,0,0);Ok(())});}
        let mut child=cmd.spawn().map_err(|_|"extractor_missing")?;
        let _group=ProcessGroup(child.id().ok_or("extractor_failed")? as i32);
        let stdout=child.stdout.take().ok_or("extractor_failed")?;
        let stderr=child.stderr.take().ok_or("extractor_failed")?;
        let run=async {
            let read=async {
                let mut out=Vec::new();let mut err=Vec::new();
                let mut stdout=stdout.take(limit as u64+1);let a=stdout.read_to_end(&mut out);
                let mut stderr=stderr.take(65537);let b=stderr.read_to_end(&mut err);
                let (a,b)=tokio::join!(a,b);a.map_err(|_|"extractor_failed")?;b.map_err(|_|"extractor_failed")?;
                if out.len()>limit||err.len()>65536{return Err("response_too_large".into());}
                Ok::<_,String>((out,err))
            };
            let (out,err)=read.await?;
            let status=child.wait().await.map_err(|_|"extractor_failed")?;
            if !status.success(){return Err(extractor_error(&String::from_utf8_lossy(&err)));}
            Ok(out)
        };
        tokio::time::timeout(Duration::from_secs(65),run).await.map_err(|_|"extractor_timeout".to_owned())?
    }
    pub async fn ready(&self)->Result<()> {
        let bytes=self.execute(&["--ignore-config".into(),"--no-plugin-dirs".into(),"--version".into()],1024).await?;
        let version=String::from_utf8_lossy(&bytes);
        if !version.trim().starts_with("20"){return Err("extractor_failed".into());}
        if !self.plugins.join("nxt/yt_dlp_plugins/extractor/nxt_soundcloud.py").is_file(){return Err("extractor_missing".into());}
        Ok(())
    }
    pub async fn extract(&self,kind:&str,query:&str,id:&str,offset:usize)->Result<Value> {
        if offset>20000||offset%PAGE!=0{return Err("page_invalid".into());}
        let resource=if kind=="play"||kind=="playlist_tracks" {
            let resource_kind=if kind=="play"{"tracks"}else{"playlists"};
            if !urn(id,resource_kind){return Err("track_invalid".into());}
            id.rsplit(':').next().unwrap_or("")
        }else{""};
        if kind=="resolve"&&(!Url::parse(query).ok().is_some_and(|u|public_url(&u))||query.len()>500){return Err("link_invalid".into());}
        if query.len()>500{return Err("query_invalid".into());}
        let mut target=Url::parse(&format!("nxtsoundcloud:{kind}")).map_err(|_|"command_invalid")?;
        target.query_pairs_mut().append_pair("query",query).append_pair("resource",resource).append_pair("offset",&offset.to_string());
        let args=vec!["--ignore-config".into(),"--no-plugin-dirs".into(),"--plugin-dirs".into(),self.plugins.to_string_lossy().into_owned(),
            "--cache-dir".into(),self.cache.to_string_lossy().into_owned(),"--no-warnings".into(),"--no-progress".into(),
            "--socket-timeout".into(),"12".into(),"--retries".into(),"1".into(),"--extractor-retries".into(),"1".into(),
            "--extractor-args".into(),"soundcloud:formats=hls_aac,hls_mp3".into(),
            "--skip-download".into(),"--flat-playlist".into(),"--dump-single-json".into(),"--".into(),target.into()];
        serde_json::from_slice(&self.execute(&args,8*1024*1024).await?).map_err(|_|"api_invalid_json".into())
    }
    pub async fn streams(&self,id:&str)->Result<Value> {
        let info=self.extract("play","",id,0).await?;let mut streams=json!({});
        if info["id"].as_str()!=id.rsplit(':').next(){return Err("track_invalid".into());}
        if let Some(formats)=info["formats"].as_array(){
            for f in formats {
                let Some(url)=f["url"].as_str().and_then(|s|Url::parse(s).ok()).filter(media_url)else{continue;};
                let name=f["format_id"].as_str().unwrap_or("").to_lowercase();
                if name.contains("preview")||name.contains("aes")||url.path().contains("/preview/")||f["preference"].as_i64()==Some(-10){continue;}
                if f["protocol"]!="m3u8_native"{continue;}
                let key=if name.starts_with("hls_aac_"){"hls_aac_160_url"}else if name.starts_with("hls_mp3_"){"hls_mp3_128_url"}else{continue;};
                streams[key]=json!(url.as_str());
            }
        }
        if streams.as_object().is_some_and(|v|v.is_empty()){return Err("audio_format_unsupported".into());}
        Ok(streams)
    }
}
fn extractor_error(message:&str)->String {
    for reason in ["track_restricted","link_invalid","track_invalid","page_invalid","query_invalid","command_invalid"] {
        if message.contains(&format!("NXT:{reason}")){return reason.into();}
    }
    if message.contains("429"){"api_http_429".into()}
    else if message.contains("geo-restricted"){"track_restricted".into()}
    else if message.contains("403"){"api_http_403".into()}
    else if message.contains("404"){"track_unavailable".into()}
    else if message.to_lowercase().contains("timed out"){"extractor_timeout".into()}
    else {"extractor_failed".into()}
}
pub fn extracted_item(v:&Value)->Option<Value> {
    let id=v["id"].as_str()?;if id.is_empty()||id.len()>32||!id.bytes().all(|c|c.is_ascii_digit()){return None;}
    let playlist=v["nxt_playlist"].as_bool().unwrap_or(false);
    let uri=format!("soundcloud:{}:{id}",if playlist{"playlists"}else{"tracks"});
    let uploader=v["uploader"].as_str().unwrap_or("SoundCloud");
    let seconds=v["duration"].as_f64().filter(|n|n.is_finite()&&*n>=0.).unwrap_or(0.);
    Some(json!({"id":uri,"uri":uri,"title":v["title"].as_str().unwrap_or("Untitled"),
        "artist":v["artist"].as_str().unwrap_or(uploader),"album":format!("SoundCloud · {uploader}"),"uploader":uploader,
        "cover":v["nxt_cover"].as_str().unwrap_or(""),"permalink":v["nxt_permalink"].as_str().unwrap_or(""),
        "duration_ms":(seconds*1000.).min(i32::MAX as f64) as u64}))
}
pub async fn bounded_bytes(mut res: reqwest::Response, max: usize) -> Result<Vec<u8>> {
    if res.content_length().is_some_and(|n| n > max as u64) { return Err("response_too_large".into()); }
    let mut bytes = Vec::new();
    while let Some(chunk) = res.chunk().await.map_err(|_| "network_error")? {
        if bytes.len() + chunk.len() > max { return Err("response_too_large".into()); }
        bytes.extend_from_slice(&chunk);
    }
    Ok(bytes)
}
pub async fn bounded_json(res: reqwest::Response) -> Result<Value> {
    serde_json::from_slice(&bounded_bytes(res, MAX_JSON).await?).map_err(|_| "api_invalid_json".into())
}
#[cfg(test)] mod tests {
    use super::*;
    #[tokio::test] async fn cancelled_extractor_process_is_killed_and_reaped() {
        use std::os::unix::fs::PermissionsExt;
        let stamp=std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_nanos();
        let root=std::env::temp_dir().join(format!("nxt-sc-cancel-{stamp}"));std::fs::create_dir(&root).unwrap();
        let script=root.join("extractor");
        std::fs::write(&script,"#!/usr/bin/env python3\nimport os,time,pathlib,sys\npathlib.Path(sys.argv[0]+'.pid').write_text(str(os.getpid()))\ntime.sleep(60)\n").unwrap();
        std::fs::set_permissions(&script,std::fs::Permissions::from_mode(0o755)).unwrap();
        let api=std::sync::Arc::new(Api::fixture(script.clone(),root.clone()));
        let task=tokio::spawn(async move{api.execute(&[],1024).await});
        let pidfile=script.with_extension("pid");
        for _ in 0..100 {if pidfile.exists(){break;}tokio::time::sleep(Duration::from_millis(10)).await;}
        let pid=std::fs::read_to_string(&pidfile).unwrap().parse::<i32>().unwrap();
        task.abort();assert!(task.await.unwrap_err().is_cancelled());
        for _ in 0..100 {if unsafe{kill(pid,0)}==-1{break;}tokio::time::sleep(Duration::from_millis(10)).await;}
        assert_eq!(unsafe{kill(pid,0)},-1);std::fs::remove_dir_all(root).unwrap();
    }
    #[test] fn only_official_urns_and_urls() {
        assert!(urn("soundcloud:tracks:123", "tracks"));
        for bad in ["123", "soundcloud:tracks:../me", "soundcloud:tracks:", "soundcloud:tracks:1?token=x"] { assert!(!urn(bad,"tracks")); }
        assert!(api_url(&Url::parse("https://api.soundcloud.com/tracks").unwrap()));
        assert!(!api_url(&Url::parse("https://api.soundcloud.com.evil.test/tracks").unwrap()));
        assert!(!media_url(&Url::parse("https://sndcdn.com@evil.test/a").unwrap()));
        assert!(!media_url(&Url::parse("http://cf-media.sndcdn.com/a").unwrap()));
        assert!(media_url(&Url::parse("https://cf-hls-media.sndcdn.com/a").unwrap()));
        assert!(media_url(&Url::parse("https://playback.media-streaming.soundcloud.cloud/a").unwrap()));
        for url in ["http://playback.media-streaming.soundcloud.cloud/a",
            "https://playback.media-streaming.soundcloud.cloud.evil.test/a",
            "https://playback.media-streaming.soundcloud.cloud@evil.test/a",
            "https://user:password@playback.media-streaming.soundcloud.cloud/a",
            "https://playback.media-streaming.soundcloud.cloud:444/a"] {
            assert!(!media_url(&Url::parse(url).unwrap()));
        }
    }
    #[test] fn extracted_metadata_and_ids_are_bounded() {
        let row=extracted_item(&json!({"id":"1","title":"ไทย 日本語 中文 한국어 Việt","uploader":"Uploader","artist":"Artist","duration":90,"nxt_playlist":false})).unwrap();
        assert_eq!(row["uri"],"soundcloud:tracks:1");assert_eq!(row["duration_ms"],90000);assert_eq!(row["artist"],"Artist");
        assert!(extracted_item(&json!({"id":"../secret"})).is_none());
        assert_eq!(extractor_error("signed private URL not echoed NXT:track_restricted"),"track_restricted");
        assert_eq!(extractor_error("HTTP Error 429: private URL not echoed"),"api_http_429");
        assert_eq!(extractor_error("HTTP Error 403: access denied"),"api_http_403");
        assert_eq!(extractor_error("geo-restricted"),"track_restricted");
    }
}
