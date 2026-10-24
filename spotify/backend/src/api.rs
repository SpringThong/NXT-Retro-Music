use serde_json::{Value,json};
use std::{path::PathBuf,time::{SystemTime,UNIX_EPOCH},os::unix::fs::OpenOptionsExt,io::Write};
pub const PLAYBACK_CLIENT_ID:&str="65b708073fc0480ea92a077233ca87bd";
#[cfg(test)]
pub const CLIENT_ID:&str="320f1ca055564990bcdf4de6fe58b397";
fn now()->u64{SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs()}
#[derive(Clone)]
pub struct Api { pub data:Value,path:PathBuf,http:reqwest::Client,session:Option<librespot_core::session::Session> }
// Respect each authorized application's cooldown without blocking OAuth refresh
// behind a Web API quota. Client IDs come only from the user's supplied login.
#[derive(Default)]
struct RateState { gate:tokio::sync::Mutex<()>, retry:std::sync::atomic::AtomicU64 }
static RATE_STATES:std::sync::LazyLock<std::sync::Mutex<std::collections::HashMap<String,std::sync::Arc<RateState>>>>=std::sync::LazyLock::new(Default::default);
#[derive(Clone,Copy)]
enum Endpoint { WebApi, OAuth }
fn retry_seconds(h:&reqwest::header::HeaderMap,attempt:u32)->u64{h.get(reqwest::header::RETRY_AFTER).and_then(|v|v.to_str().ok()).and_then(|s|s.parse().ok()).unwrap_or(2u64<<attempt).max(1)}
impl Api {
 fn rate_state(&self,endpoint:Endpoint)->std::sync::Arc<RateState>{
  let key=format!("{}:{}",self.client_id(),match endpoint{Endpoint::WebApi=>"api",Endpoint::OAuth=>"oauth"});
  RATE_STATES.lock().expect("rate state lock").entry(key).or_default().clone()
 }
 async fn send_limited(&self,request:reqwest::RequestBuilder,endpoint:Endpoint,retry:bool)->Result<reqwest::Response,String>{
  use std::sync::atomic::Ordering;
  let state=self.rate_state(endpoint);let _guard=state.gate.lock().await;
  for attempt in 0..if retry {3}else{1}{
   let wait=state.retry.load(Ordering::Relaxed).saturating_sub(now());
   if wait>0 {
    if !retry||wait>120{return Err(format!("rate_limited:{wait}"));}
    crate::emit(json!({"event":"retry_wait","seconds":wait}));tokio::time::sleep(std::time::Duration::from_secs(wait)).await;
   }
   let res=request.try_clone().ok_or("request_invalid")?.send().await.map_err(|_|"network_error")?;
   if res.status()!=reqwest::StatusCode::TOO_MANY_REQUESTS{return Ok(res);}
   let wait=retry_seconds(res.headers(),attempt);state.retry.store(now().saturating_add(wait),Ordering::Relaxed);
   if !retry||attempt==2||wait>120{return Err(format!("rate_limited:{wait}"));}
   crate::emit(json!({"event":"retry_wait","seconds":wait}));
  }
  Err("network_error".into())
 }
 async fn send_retry(&self,request:reqwest::RequestBuilder)->Result<reqwest::Response,String>{self.send_limited(request,Endpoint::WebApi,true).await}
 pub fn client_id(&self)->&str{self.data["client_id"].as_str().unwrap_or("")}
 pub fn same_file(&self,other:&Self)->bool{self.path==other.path}
 pub fn open(path:&str)->Result<Self,String>{Self::open_for(path,None)}
 pub fn playback(path:&str)->Result<Self,String>{Self::open_for(path,Some(PLAYBACK_CLIENT_ID))}
 fn open_for(path:&str,expected:Option<&str>)->Result<Self,String>{
  let p=PathBuf::from(path);let bytes=std::fs::read(&p).map_err(|_|"credentials_missing")?;
  if bytes.len()>65536{return Err("credentials_too_large".into());}
  let d:Value=serde_json::from_slice(&bytes).map_err(|_|"credentials_invalid")?;
  let id=d["client_id"].as_str().ok_or("credentials_wrong_app")?;
  if id.len()!=32||!id.bytes().all(|c|c.is_ascii_hexdigit())||expected.is_some_and(|e|e!=id){return Err("credentials_wrong_app".into());}
  if !d["access_token"].as_str().is_some_and(|v|!v.is_empty())||!d["refresh_token"].as_str().is_some_and(|v|!v.is_empty()){return Err("credentials_invalid".into());}
  Ok(Self{data:d,path:p,session:None,http:reqwest::Client::builder().timeout(std::time::Duration::from_secs(20)).build().map_err(|_|"http_init")?})
 }
 pub fn from_session(session:librespot_core::session::Session)->Result<Self,String>{
  // Same approved account and client ID as playback. Never rotate clients to
  // bypass a provider denial or quota; all HTTP limits remain shared.
  Ok(Self{data:json!({"client_id":session.client_id()}),path:PathBuf::new(),
   http:reqwest::Client::builder().timeout(std::time::Duration::from_secs(20)).build().map_err(|_|"http_init")?,session:Some(session)})
 }
 pub async fn token(&mut self)->Result<String,String>{
  if let Some(session)=&self.session {
   // auth::connect already verified Login5 for this same account/client. The
   // old Mercury keymaster is not a catalog-grant probe: it can reject token
   // issuance even after QR and playback authentication have succeeded.
   // Reuse Login5's cached/refreshable token; Web API still enforces access.
   return match tokio::time::timeout(std::time::Duration::from_secs(8),session.login5().auth_token()).await {
    Ok(Ok(token))=>Ok(token.access_token),Ok(Err(e))=>{crate::diagnostic(&format!("Library token: Login5 unavailable ({:?})",e.kind));Err("session_token_unavailable".into())},Err(_)=>Err("session_token_timeout".into())
   };
  }

  static TOKEN_LOCK:tokio::sync::Mutex<()>=tokio::sync::Mutex::const_new(());let _guard=TOKEN_LOCK.lock().await;
  // Cloned request workers use the latest rotated token from the same file.
  if let Ok(bytes)=std::fs::read(&self.path){if let Ok(data)=serde_json::from_slice::<Value>(&bytes){if data["client_id"]==self.data["client_id"]{self.data=data;}}}
  let issued=self.data["obtained_at"].as_u64().unwrap_or(0);let ttl=self.data["expires_in"].as_u64().unwrap_or(0);
  if now()+60>=issued.saturating_add(ttl){
   let client=self.data["client_id"].as_str().ok_or("credentials_invalid")?.to_owned();
   let refresh=self.data["refresh_token"].as_str().ok_or("refresh_missing")?.to_owned();
   let res=self.send_limited(self.http.post("https://accounts.spotify.com/api/token").form(&[("client_id",client.as_str()),("grant_type","refresh_token"),("refresh_token",&refresh)]),Endpoint::OAuth,false).await?;
   if !res.status().is_success(){return Err(format!("refresh_http_{}",res.status().as_u16()));}
   let fresh:Value=res.json().await.map_err(|_|"refresh_invalid")?;
   if fresh["access_token"].as_str().is_none(){return Err("refresh_invalid".into());}
   for k in ["access_token","expires_in","token_type","scope","refresh_token"]{if !fresh[k].is_null(){self.data[k]=fresh[k].clone();}}
   self.data["obtained_at"]=json!(now());
   let tmp=self.path.with_extension("tmp");
   let mut f=std::fs::OpenOptions::new().write(true).create(true).truncate(true).mode(0o600).open(&tmp).map_err(|_|"credentials_write_failed")?;
   use std::os::unix::fs::PermissionsExt;f.set_permissions(std::fs::Permissions::from_mode(0o600)).map_err(|_|"credentials_write_failed")?;
   f.write_all(self.data.to_string().as_bytes()).map_err(|_|"credentials_write_failed")?;f.sync_all().map_err(|_|"credentials_write_failed")?;
   std::fs::rename(tmp,&self.path).map_err(|_|"credentials_write_failed")?;
  }
  self.data["access_token"].as_str().map(str::to_owned).ok_or("access_token_missing".into())
 }
 pub async fn get(&mut self,path:&str,params:&[(&str,&str)])->Result<Value,String>{self.get_mode(path,params,true).await}
 // Login probes must not wait behind the library API: AP + Login5 determine
 // whether playback is authenticated. Quota failures remain visible separately.
 pub async fn get_once(&mut self,path:&str,params:&[(&str,&str)])->Result<Value,String>{self.get_mode(path,params,false).await}
 async fn get_mode(&mut self,path:&str,params:&[(&str,&str)],retry:bool)->Result<Value,String>{
  if !path.starts_with('/')||path.contains("..") {return Err("invalid_api_path".into());}
  let token=self.token().await?;
  let mut request=self.http.get(format!("https://api.spotify.com/v1{path}")).query(params).bearer_auth(token);
  if !retry {request=request.timeout(std::time::Duration::from_secs(8));}
  let res=self.send_limited(request,Endpoint::WebApi,retry).await?;
  if !res.status().is_success(){let status=res.status().as_u16();let body=res.json::<Value>().await.unwrap_or(Value::Null);return Err(api_error(status,&body));}
  res.json().await.map_err(|_|"api_invalid_json".into())
 }
 pub async fn favorite(&mut self,uri:&str,value:bool)->Result<(),String>{
  let token=self.token().await?;let method=if value{reqwest::Method::PUT}else{reqwest::Method::DELETE};
  let res=self.send_retry(self.http.request(method,"https://api.spotify.com/v1/me/library").query(&[("uris",uri)]).bearer_auth(token)).await?;
  if res.status().is_success(){Ok(())}else{let status=res.status().as_u16();let body=res.json::<Value>().await.unwrap_or(Value::Null);Err(api_error(status,&body))}
 }
 pub async fn cover(&self,url:&str,path:&str)->Result<(),String>{
  let u=reqwest::Url::parse(url).map_err(|_|"cover_invalid")?;
  if u.scheme()!="https"||u.host_str()!=Some("i.scdn.co"){return Err("cover_invalid".into());}
  let mut res=self.http.get(u).send().await.map_err(|_|"cover_network")?;
  if !res.status().is_success(){return Err("cover_http".into());}
  let mut bytes=Vec::new();while let Some(chunk)=res.chunk().await.map_err(|_|"cover_read")?{if bytes.len()+chunk.len()>8*1024*1024{return Err("cover_too_large".into());}bytes.extend_from_slice(&chunk);}
  let tmp=format!("{path}.tmp");std::fs::write(&tmp,bytes).map_err(|_|"cover_write")?;std::fs::rename(tmp,path).map_err(|_|"cover_write".to_owned())
 }
}
fn api_error(status:u16,body:&Value)->String{
 let message=body["error"]["message"].as_str().unwrap_or("").to_ascii_lowercase();
 if status==403&&(message.contains("insufficient client scope")||message.contains("insufficient scope")){"api_scope_missing".into()}
 else{format!("api_http_{status}")}
}
pub fn track(v:&Value)->Value{
 let names=v["artists"].as_array().map(|a|a.iter().filter_map(|v|v["name"].as_str()).collect::<Vec<_>>().join(", ")).unwrap_or_default();
 let cover=v["album"]["images"].as_array().and_then(|a|a.iter().find(|x|x["width"].as_u64().unwrap_or(0)<=640).or(a.last())).map(|x|x["url"].clone()).unwrap_or(Value::Null);
 json!({"title":v["name"],"artist":names,"album":v["album"]["name"],"duration_ms":v["duration_ms"],"uri":v["uri"],"cover":cover})
}
pub fn items(kind:&str,root:&Value,parent:&Value)->Vec<Value>{
 root["items"].as_array().map(|xs|xs.iter().filter_map(|x|{
  if x.is_null(){return None;}
  let item=match kind{"saved_tracks"=>&x["track"],"saved_albums"=>&x["album"],"playlist_tracks"=>if x["item"].is_null(){&x["track"]}else{&x["item"]},_=>x};
  if item.is_null(){return None;}
  match kind{
   "playlists"|"search_playlists"=>if item["id"].is_string(){Some(json!({"title":item["name"],"id":item["id"],"artist":item["owner"]["display_name"]}))}else{None},
   "search_artists"=>Some(json!({"title":item["name"],"id":item["id"],"artist":"Artist"})),
   "search_albums"|"artist_albums"|"saved_albums"=>{let artists=item["artists"].as_array().map(|xs|xs.iter().filter_map(|x|x["name"].as_str()).collect::<Vec<_>>().join(", ")).unwrap_or_default();Some(json!({"title":item["name"],"id":item["id"],"artist":artists}))},
   _=>{if !playable_track(item){return None;}let mut t=item.clone();if kind=="album_tracks"{t["album"]=parent.clone();}Some(track(&t))}
  }
 }).collect()).unwrap_or_default()
}
pub fn playable_track(item:&Value)->bool {
 item["uri"].as_str().is_some_and(|u|u.strip_prefix("spotify:track:").is_some_and(|id|id.len()==22&&id.bytes().all(|c|c.is_ascii_alphanumeric())))
 && item["is_playable"].as_bool()!=Some(false)
 && item["is_local"].as_bool()!=Some(true)
 && !item["restrictions"]["reason"].is_string()
}
#[cfg(test)]
mod tests {
 #[test]fn catalog_parses_null_playlists_and_album_tracks(){
  let root=serde_json::json!({"items":[null,{"name":"Playlist","id":"0123456789012345678901","owner":{"display_name":"User"}}]});assert_eq!(super::items("search_playlists",&root,&serde_json::Value::Null).len(),1);
  let tracks=serde_json::json!({"items":[{"uri":"spotify:track:0123456789012345678901","name":"Song","artists":[],"duration_ms":1000}]});let album=serde_json::json!({"name":"Album","images":[{"width":300,"url":"https://i.scdn.co/cover"}]});let xs=super::items("album_tracks",&tracks,&album);assert_eq!(xs[0]["album"],"Album");assert_eq!(xs[0]["cover"],"https://i.scdn.co/cover");
 }
 use super::*;
 #[test] fn metadata_preserves_scripts_and_selects_cover(){
  let v=json!({"name":"한국어 日本語 ไทย 中文","artists":[{"name":"Đặng"},{"name":"歌手"}],"album":{"name":"Album","images":[{"width":640,"url":"https://i.scdn.co/640"},{"width":300,"url":"https://i.scdn.co/300"}]},"duration_ms":123000,"uri":"spotify:track:0123456789012345678901"});
  let t=track(&v);assert_eq!(t["title"],v["name"]);assert_eq!(t["artist"],"Đặng, 歌手");assert_eq!(t["cover"],"https://i.scdn.co/640");assert_eq!(t["duration_ms"],123000);
 }
 #[test] fn credentials_reject_wrong_application(){
  let p=std::env::temp_dir().join(format!("nxt-spotify-test-{}.json",std::process::id()));std::fs::write(&p,b"{\"client_id\":\"wrong-app\"}").unwrap();
  assert!(matches!(Api::open(p.to_str().unwrap()),Err(e) if e=="credentials_wrong_app"));std::fs::remove_file(p).unwrap();
 }
 #[test] fn playback_credentials_match_session_and_are_separate_from_library(){
  assert_eq!(librespot_core::config::SessionConfig::default().client_id,PLAYBACK_CLIENT_ID);
  let p=std::env::temp_dir().join(format!("nxt-spotify-paired-{}.json",std::process::id()));
  std::fs::write(&p,json!({"client_id":PLAYBACK_CLIENT_ID,"access_token":"FAKE","refresh_token":"FAKE"}).to_string()).unwrap();
  assert!(Api::playback(p.to_str().unwrap()).is_ok());assert!(Api::open(p.to_str().unwrap()).is_ok());
  std::fs::write(&p,json!({"client_id":CLIENT_ID,"access_token":"FAKE","refresh_token":"FAKE"}).to_string()).unwrap();
  assert!(Api::open(p.to_str().unwrap()).is_ok());assert!(Api::playback(p.to_str().unwrap()).is_err());std::fs::remove_file(p).unwrap();
 }
}

#[cfg(test)]mod retry_tests{
 #[test]fn respects_server_cooldown(){let mut h=reqwest::header::HeaderMap::new();h.insert(reqwest::header::RETRY_AFTER,"37".parse().unwrap());assert_eq!(super::retry_seconds(&h,0),37);h.clear();assert_eq!(super::retry_seconds(&h,0),2);assert_eq!(super::retry_seconds(&h,2),8);}
 #[tokio::test]async fn actual_429_wait_then_success(){use std::io::{Read,Write};use std::sync::atomic::Ordering;let listener=std::net::TcpListener::bind("127.0.0.1:0").unwrap();let address=listener.local_addr().unwrap();let server=std::thread::spawn(move||{for i in 0..2{let(mut s,_)=listener.accept().unwrap();let mut data=[0;4096];s.read(&mut data).unwrap();s.write_all(if i==0{b"HTTP/1.1 429 Too Many Requests\r\nRetry-After: 1\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"}else{b"HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}"}).unwrap();}});let api=super::Api{data:serde_json::json!({}),path:std::path::PathBuf::new(),session:None,http:reqwest::Client::new()};let start=std::time::Instant::now();api.rate_state(super::Endpoint::WebApi).retry.store(0,Ordering::Relaxed);let r=api.send_retry(api.http.get(format!("http://{address}/test"))).await.unwrap();assert_eq!(r.status(),200);assert!(start.elapsed().as_millis()>=900);server.join().unwrap();}
}

#[cfg(test)] mod fix1_tests {
 use super::*;
 #[test] fn accepts_authorized_pc_client_and_rejects_empty_tokens() {
  let path=std::env::temp_dir().join(format!("nxt-pc-client-{}.json",std::process::id()));
  let valid=json!({"client_id":"0123456789abcdef0123456789abcdef","access_token":"FAKE","refresh_token":"FAKE"});
  std::fs::write(&path,valid.to_string()).unwrap();assert!(Api::open(path.to_str().unwrap()).is_ok());
  assert!(Api::playback(path.to_str().unwrap()).is_err());
  let mut bad=valid;bad["refresh_token"]=json!("");std::fs::write(&path,bad.to_string()).unwrap();
  assert!(matches!(Api::open(path.to_str().unwrap()),Err(e) if e=="credentials_invalid"));std::fs::remove_file(path).unwrap();
 }
 #[tokio::test] async fn login_probe_is_fast_and_library_cooldown_does_not_block_oauth() {
  use std::io::{Read,Write};use std::sync::atomic::Ordering;
  let listener=std::net::TcpListener::bind("127.0.0.1:0").unwrap();let address=listener.local_addr().unwrap();
  let server=std::thread::spawn(move||{for i in 0..2 {let (mut socket,_)=listener.accept().unwrap();let mut bytes=[0;4096];socket.read(&mut bytes).unwrap();
   socket.write_all(if i==0 {b"HTTP/1.1 429 Too Many Requests\r\nRetry-After: 37\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"}else{b"HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}"}).unwrap();}});
  let a=Api{data:json!({"client_id":"feed0123456789abcdef0123456789ab"}),path:PathBuf::new(),session:None,http:reqwest::Client::new()};
  let state=a.rate_state(Endpoint::WebApi);state.retry.store(0,Ordering::Relaxed);let start=std::time::Instant::now();
  let first=a.send_limited(a.http.get(format!("http://{address}/library")),Endpoint::WebApi,false).await;
  assert!(matches!(first,Err(e) if e.starts_with("rate_limited:")));assert!(start.elapsed().as_secs()<3);
  let second=a.send_limited(a.http.get(format!("http://{address}/library")),Endpoint::WebApi,false).await;
  assert!(matches!(second,Err(e) if e.starts_with("rate_limited:")));
  let result=a.send_limited(a.http.post(format!("http://{address}/oauth")),Endpoint::OAuth,false).await.unwrap();assert_eq!(result.status(),200);
  assert!(!std::sync::Arc::ptr_eq(&state,&a.rate_state(Endpoint::OAuth)));
  let other=Api{data:json!({"client_id":"abcd0123456789abcdef0123456789ab"}),..a.clone()};
  assert!(!std::sync::Arc::ptr_eq(&state,&other.rate_state(Endpoint::WebApi)));
  assert!(state.retry.load(Ordering::Relaxed)>now());server.join().unwrap();
 }
}

#[cfg(test)] mod playable_tests {
 use super::*;
 #[test] fn rejects_market_product_explicit_and_local_tracks(){
  let good=json!({"uri":"spotify:track:0123456789012345678901","is_playable":true});assert!(playable_track(&good));
  for reason in ["market","product","explicit"]{let mut t=good.clone();t["restrictions"]=json!({"reason":reason});assert!(!playable_track(&t));}
  let mut t=good.clone();t["is_playable"]=json!(false);assert!(!playable_track(&t));t=good.clone();t["is_local"]=json!(true);assert!(!playable_track(&t));
  for uri in ["spotify:episode:0123456789012345678901","spotify:track:../a","https://example.test"]{t=good.clone();t["uri"]=json!(uri);assert!(!playable_track(&t));}
 }
}

#[cfg(test)] mod v1101_session_tests {
 use super::*;
 #[tokio::test] async fn empty_session_reports_login5_unavailable_without_keymaster_or_pc_fallback(){
  let session=librespot_core::session::Session::new(Default::default(),None);
  let client=session.client_id();let mut api=Api::from_session(session).unwrap();
  assert_eq!(api.client_id(),client);assert!(api.path.as_os_str().is_empty());
  let start=std::time::Instant::now();
  assert_eq!(api.token().await.unwrap_err(),"session_token_unavailable");
  assert!(start.elapsed()<std::time::Duration::from_secs(2));
 }
 #[test] fn server_scope_denial_is_distinct_from_token_transport_failure(){
  assert_eq!(api_error(403,&json!({"error":{"message":"Insufficient client scope"}})),"api_scope_missing");
  assert_eq!(api_error(403,&json!({"error":{"message":"Forbidden"}})),"api_http_403");
  assert_eq!(api_error(401,&json!({"error":{"message":"Expired token"}})),"api_http_401");
  assert_eq!(api_error(429,&Value::Null),"api_http_429");
 }
}
