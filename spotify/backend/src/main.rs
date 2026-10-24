mod api;
mod auth;
mod pcm;
mod pair;
mod catalog;
mod native_catalog;
mod lyrics;
use std::sync::{Arc,atomic::{AtomicU32,Ordering}};
// NXT direct Spotify player. JSON stdin/stdout; no Connect discovery/control service.
use librespot_core::{SpotifyUri, session::Session};
use librespot_playback::{config::{Bitrate,PlayerConfig},mixer::NoOpVolume,player::{Player,PlayerEvent}};
use serde_json::{Value,json};
use tokio::io::{AsyncBufReadExt,BufReader};
struct AudioLog;
impl log::Log for AudioLog{
 fn enabled(&self,m:&log::Metadata)->bool{m.level()<=log::Level::Warn&&(m.target().starts_with("librespot_playback::player")||m.target().starts_with("librespot_playback::audio_backend"))}
 fn log(&self,r:&log::Record){if self.enabled(r.metadata()){diagnostic(&format!("Playback: {}",r.args()));}}
 fn flush(&self){}
}
static AUDIO_LOG:AudioLog=AudioLog;
fn emit(v:Value){use std::io::Write;let mut output=std::io::stdout().lock();let _=writeln!(output,"{v}");}
fn diagnostic(message:&str){
 use std::{io::Write,os::unix::fs::OpenOptionsExt};
 eprintln!("Spotify v{}: {message}",env!("CARGO_PKG_VERSION"));
 if let Ok(path)=std::env::var("NXT_SPOTIFY_DIAG"){if let Ok(mut f)=std::fs::OpenOptions::new().create(true).append(true).mode(0o600).open(path){let _=writeln!(f,"{message}");}}
}
fn playback(s:Session,generation:Arc<AtomicU32>)->Result<Arc<Player>,String>{
 let cfg=PlayerConfig{gapless:false,bitrate:Bitrate::Bitrate320,position_update_interval:Some(std::time::Duration::from_secs(1)),..Default::default()};
 let socket=pcm::PcmSink::channel()?;
 let p=Player::new(cfg,s.clone(),Box::new(NoOpVolume),move||Box::new(pcm::PcmSink::new(socket.try_clone().expect("PCM socket clone"),generation.clone())));
 let mut events=p.get_player_event_channel();
 tokio::spawn(async move{while let Some(e)=events.recv().await{match e{
  PlayerEvent::TrackChanged{audio_item}=>{
   let (artist,album)=match &audio_item.unique_fields {
    librespot_metadata::audio::item::UniqueFields::Track{artists,album,..}=>(artists.iter().map(|a|a.name.as_str()).collect::<Vec<_>>().join(", "),album.clone()),
    _=>(String::new(),String::new())
   };
   let cover=audio_item.covers.iter().find(|c|c.width>0&&c.width<=640).or(audio_item.covers.last()).map(|c|c.url.as_str());
   emit(json!({"event":"metadata","uri":audio_item.uri,"title":audio_item.name,"artist":artist,"album":album,"duration_ms":audio_item.duration_ms,"cover":cover}));
  },
  PlayerEvent::Playing{position_ms,track_id,..}=>emit(json!({"event":"playing","position_ms":position_ms,"uri":track_id.to_uri()})),
  PlayerEvent::Paused{position_ms,track_id,..}=>emit(json!({"event":"paused","position_ms":position_ms,"uri":track_id.to_uri()})),
  PlayerEvent::PositionChanged{position_ms,track_id,..}=>emit(json!({"event":"position","position_ms":position_ms,"uri":track_id.to_uri()})),
  PlayerEvent::EndOfTrack{track_id,..}=>emit(json!({"event":"ended","uri":track_id.to_uri()})),
  PlayerEvent::Unavailable{track_id,..}=>emit(json!({"event":"error","reason":"track_unavailable","uri":track_id.to_uri()})),_=>{}
 }}});
 Ok(p)
}

#[tokio::main]
async fn main(){
 let _=log::set_logger(&AUDIO_LOG);log::set_max_level(log::LevelFilter::Warn);
 let mut input=BufReader::new(tokio::io::stdin()).lines();
 let mut player:Option<Arc<Player>>=None;
 let mut session:Option<Session>=None;
 let mut api:Option<Arc<tokio::sync::Mutex<catalog::Catalog>>>=None;
 let mut query_task:Option<tokio::task::JoinHandle<()>>=None;
 let mut lyrics_task:Option<tokio::task::JoinHandle<()>>=None;
 let mut favorite_task:Option<tokio::task::JoinHandle<()>>=None;
 let mut pair_task:Option<tokio::task::JoinHandle<()>>=None;
 let mut cover_task:Option<tokio::task::JoinHandle<()>>=None;
 let generation=Arc::new(AtomicU32::new(0));
 diagnostic("Backend build: NXT-v1.10.2-NativeCatalog");
 emit(json!({"event":"ready","experimental":false,"connect_service":false,"version":"1.10.2","build":"NXT-v1.10.2-NativeCatalog"}));
 while let Ok(Some(line))=input.next_line().await{
  if line.len()>16384{emit(json!({"event":"error","reason":"request_too_large"}));continue;}
  let Ok(v)=serde_json::from_str::<Value>(&line) else{emit(json!({"event":"error","reason":"invalid_json"}));continue;};
  match v["cmd"].as_str().unwrap_or(""){
   "login"=>{
    // A reconnect must retire workers and the player belonging to the previous
    // identity. Library HTTP errors never turn a verified AP session into logout.
    if let Some(task)=query_task.take(){task.abort();}
    if let Some(task)=favorite_task.take(){task.abort();}
    if let Some(task)=lyrics_task.take(){task.abort();}
    if let Some(task)=cover_task.take(){task.abort();}
    if let Some(p)=player.take(){p.stop();drop(p);}if let Some(s)=session.take(){s.shutdown();}api=None;
    match auth::connect(v["source"].as_str()).await {
     Ok(verified)=>{
      emit(json!({"event":"login_progress","stage":"library"}));
      let a=catalog::Catalog::open_verified(verified.credentials,&verified.username,&verified.session).await;
      let name=a.display_name().to_owned();let warning=a.library_warning.clone();let import_warning=a.import_warning.clone();let native_catalog=a.uses_native();
      session=Some(verified.session);api=Some(Arc::new(tokio::sync::Mutex::new(a)));
      emit(json!({"event":"logged_in","display_name":name,"needs_upgrade":false,"playback_ready":true,"library_ready":warning.is_none(),"library_warning":warning,"import_warning":import_warning,"native_catalog":native_catalog}));
     },
     Err(e)=>{diagnostic(&format!("Login: playback authentication failed: {e}"));emit(json!({"event":"error","reason":e,"operation":"login"}));}
    }
   },
   "pair"=>{
    if let Some(task)=pair_task.take(){task.abort();}
    if let Some(task)=query_task.take(){task.abort();}
    if let Some(task)=lyrics_task.take(){task.abort();}
    if let Some(task)=favorite_task.take(){task.abort();}
    if let Some(task)=cover_task.take(){task.abort();}
    if let Some(p)=player.take(){p.stop();drop(p);}if let Some(s)=session.take(){s.shutdown();}api=None;
    pair_task=Some(tokio::spawn(pair::run()));
   },
   "cancel_pair"=>{if let Some(task)=pair_task.take(){task.abort();}diagnostic("Pair: cancelled by user");},
   "play"=>{
    if api.is_none(){emit(json!({"event":"error","reason":"not_logged_in"}));continue;}
    let uri=match v["uri"].as_str().and_then(|u|SpotifyUri::from_uri(u).ok()){Some(uri @ SpotifyUri::Track{..})=>uri,_=>{emit(json!({"event":"error","reason":"invalid_track_uri"}));continue;}};
    if let Some(task)=lyrics_task.take(){task.abort();}
    if player.is_none(){
     let Some(s)=session.as_ref().filter(|s|!s.is_invalid()) else{emit(json!({"event":"error","reason":"pair_required","operation":"play"}));continue;};
     match playback(s.clone(),generation.clone()){Ok(p)=>player=Some(p),Err(e)=>{diagnostic(&format!("Playback initialization failed: {e}"));emit(json!({"event":"error","reason":e,"operation":"play"}));continue;}}
    }
    generation.store(v["stream_id"].as_u64().unwrap_or(0) as u32,Ordering::Release);
    if let Some(p)=&player{p.load(uri,v["autoplay"].as_bool().unwrap_or(true),v["position_ms"].as_u64().unwrap_or(0).min(u32::MAX as u64) as u32);emit(json!({"event":"loading"}));}
   },
   "search"|"search_playlists"|"search_albums"|"search_artists"|"playlists"|"playlist_tracks"|"album_tracks"|"artist_albums"|"saved_tracks"|"saved_albums"=>{
    if let Some(a)=&api {if let Some(task)=query_task.take(){task.abort();}let a=a.clone();query_task=Some(tokio::spawn(async move{emit(a.lock().await.query(&v).await);}));}
    else{emit(json!({"event":"error","reason":"not_logged_in"}));}
   },
   "cover"=>{if let Some(a)=&api{if let(Some(url),Ok(path))=(v["url"].as_str(),std::env::var("NXT_SPOTIFY_COVER")){
    if let Some(task)=cover_task.take(){task.abort();}
    let a=a.clone();let url=url.to_owned();let uri=v["uri"].clone();
    cover_task=Some(tokio::spawn(async move{let image=a.lock().await.cover_api();if image.cover(&url,&path).await.is_ok(){emit(json!({"event":"cover","uri":uri}));}}));
   }}},
   "lyrics"=>{if let Some(s)=&session {if let Some(librespot_core::SpotifyUri::Track{id})=v["uri"].as_str().and_then(|u|SpotifyUri::from_uri(u).ok()){
    if let Some(task)=lyrics_task.take(){task.abort();}lyrics_task=Some(tokio::spawn(lyrics::run(s.clone(),id,v)));
   }}},
   "favorite"|"check_favorite"=>{if let Some(a)=&api {
    let Some(uri)=v["uri"].as_str().filter(|u|matches!(SpotifyUri::from_uri(u),Ok(SpotifyUri::Track{..}))).map(str::to_owned) else{continue;};
    if let Some(task)=favorite_task.take(){task.abort();}let a=a.clone();let change=v["cmd"]=="favorite";let saved=v["saved"].as_bool().unwrap_or(false);
    favorite_task=Some(tokio::spawn(async move{let mut a=a.lock().await;let result=if change {a.favorite(&uri,saved).await.map(|_|saved)}else{a.saved(&uri).await};match result {
     Ok(value)=>emit(json!({"event":"favorite","uri":uri,"saved":value})),Err(e)=>emit(json!({"event":"favorite_error","uri":uri,"reason":e}))
    }}));
   }},
   "pause"=>{if let Some(p)=&player{p.pause();}},
   "resume"=>{if let Some(p)=&player{p.play();}},
   "seek"=>{if let(Some(p),Some(ms))=(&player,v["position_ms"].as_u64()){if ms<=u32::MAX as u64{p.seek(ms as u32);}}},
   "stop"=>{if let Some(p)=&player{p.stop();}},
   "quit"=>break,
   _=>emit(json!({"event":"error","reason":"unknown_command"}))
  }
 }
 if let Some(task)=query_task{task.abort();}
 if let Some(task)=lyrics_task{task.abort();}
 if let Some(task)=favorite_task{task.abort();}
 if let Some(task)=pair_task{task.abort();}
 if let Some(task)=cover_task{task.abort();}
 if let Some(p)=player{p.stop();drop(p);}
 if let Some(s)=session{s.shutdown();}
 emit(json!({"event":"closed"}));
}
