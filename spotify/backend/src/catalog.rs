use crate::{api::{self,Api},diagnostic};
use serde_json::{Value,json};
#[derive(Clone)]
pub struct Catalog { primary:Api,profile:Value,pub library_warning:Option<String>,pub import_warning:Option<String>,blocked:Option<String> }
async fn profile_probe(api:&mut Api)->Result<Value,String>{
 // Bound only the library request: cancelling OAuth refresh can lose a
 // rotated token. OAuth has its own bounded HTTP request.
 api.get_once("/me",&[]).await
}
impl Catalog {
 pub async fn open_verified(playback:Api,username:&str,session:&librespot_core::session::Session)->Self {
  // QR authorizes the playback session. Derive catalog access from that same
  // session rather than requiring an unrelated developer-login JSON first.
  let qr=playback.client_id()==api::PLAYBACK_CLIENT_ID;
  let mut primary=if qr {Api::from_session(session.clone()).unwrap_or(playback)} else {playback};
  let minimal=json!({"id":username,"display_name":"Spotify"});
  match profile_probe(&mut primary).await {
   Ok(profile) if same_account(&profile,Some(username))=>Self{primary,profile,library_warning:None,import_warning:None,blocked:None},
   Ok(_)=>Self{primary,profile:minimal,library_warning:Some("account_mismatch".into()),import_warning:None,blocked:Some("account_mismatch".into())},
   Err(e)=>{
    diagnostic(&format!("Library probe: {e}; verified playback remains available"));
    let profile=match tokio::time::timeout(std::time::Duration::from_secs(5),session.spclient().get_user_profile(username,Some(0),Some(0))).await {
     Ok(Ok(bytes))=>serde_json::from_slice::<Value>(&bytes).ok().and_then(|v|profile_display(&v,username)).unwrap_or(minimal),
     _=>minimal
    };
    Self{primary,profile,library_warning:Some(e),import_warning:None,blocked:None}
   }
  }
 }
 pub fn display_name(&self)->&str {self.profile["display_name"].as_str().filter(|s|!s.is_empty()).unwrap_or("Spotify")}
 async fn get(&mut self,path:&str,params:&[(&str,&str)])->Result<Value,String>{
  if let Some(reason)=&self.blocked{return Err(reason.clone());}
  self.primary.get(path,params).await
 }
 pub async fn saved(&mut self,uri:&str)->Result<bool,String>{let body=self.get("/me/library/contains",&[("uris",uri)]).await?;body[0].as_bool().ok_or_else(||"api_invalid_json".into())}
 pub async fn favorite(&mut self,uri:&str,value:bool)->Result<(),String>{if let Some(e)=&self.blocked{return Err(e.clone());}self.primary.favorite(uri,value).await}
 pub fn cover_api(&self)->Api {self.primary.clone()}
 pub async fn query(&mut self,v:&Value)->Value {
  let cmd=v["cmd"].as_str().unwrap_or("");
  let offset=bounded_offset(cmd,v["offset"].as_u64().unwrap_or(0)).to_string();
  let q=v["query"].as_str().unwrap_or("").trim();
  if cmd.starts_with("search")&&q.is_empty(){return json!({"event":"error","reason":"empty_search_query","request_id":v["request_id"]});}let id=v["id"].as_str().unwrap_or("");
  let valid_id=id.len()==22&&id.bytes().all(|c|c.is_ascii_alphanumeric());
  let search_type=match cmd{"search_playlists"=>"playlist","search_albums"=>"album","search_artists"=>"artist",_=>"track"};
  let params=[("limit",if cmd=="playlists"{"10"}else{"20"}),("offset",offset.as_str())];let mut parent=Value::Null;
  let result=match cmd {
   _ if cmd.starts_with("search")=>self.get("/search",&[("q",q),("type",search_type),("limit","10"),("offset",&offset)]).await,
   "playlists"=>self.get("/me/playlists",&params).await,
   "saved_tracks"=>self.get("/me/tracks",&params).await,
   "saved_albums"=>self.get("/me/albums",&params).await,
   _ if !valid_id=>Err("invalid_item_id".into()),
   "artist_albums"=>self.get(&format!("/artists/{id}/albums"),&params).await,
   "album_tracks"=>match self.get(&format!("/albums/{id}"),&[]).await {Ok(album)=>{parent=album;self.get(&format!("/albums/{id}/tracks"),&params).await},Err(e)=>Err(e)},
   _=>self.get(&format!("/playlists/{id}/items"),&params).await
  };
  match result {Ok(mut body)=>{
   if cmd=="playlists" {
    // A public or followed playlist is not proof of readable items. Test
    // access with this same account and client; never switch credentials.
    let candidates=body["items"].as_array().cloned().unwrap_or_default();
    let mut readable=Vec::new();
    for playlist in candidates.iter().take(10) {
     let Some(id)=playlist["id"].as_str().filter(|id|id.len()==22&&id.bytes().all(|c|c.is_ascii_alphanumeric()))else{continue;};
     if playlist["owner"]["id"]!=self.profile["id"]&&playlist["collaborative"].as_bool()!=Some(true){continue;}
     match self.primary.get_once(&format!("/playlists/{id}/items"),&[("limit","20"),("offset","0")]).await {
      Ok(tracks) if !api::items("playlist_tracks",&tracks,&Value::Null).is_empty()=>readable.push(playlist.clone()),
      Ok(_)=>{},
      Err(e) if e=="api_http_403"||e=="api_http_404"=>{},
      Err(e)=>return json!({"event":"error","reason":e,"request_id":v["request_id"]}),
     }
    }
    body["items"]=json!(readable);
   }
   let root=if cmd.starts_with("search"){&body[format!("{search_type}s")]}else{&body};
   json!({"event":"results","kind":cmd,"items":api::items(cmd,root,&parent),"offset":v["offset"].as_u64().unwrap_or(0),"next_offset":root["next"].as_str().and_then(|n|reqwest::Url::parse(n).ok()).and_then(|u|u.query_pairs().find(|(k,_)|k=="offset").and_then(|(_,v)|v.parse::<u64>().ok())).unwrap_or(v["offset"].as_u64().unwrap_or(0)+if cmd.starts_with("search"){10}else{20}),"has_next":root["next"].is_string()&&(!cmd.starts_with("search")||v["offset"].as_u64().unwrap_or(0)<990),"request_id":v["request_id"]})
  },Err(e)=>json!({"event":"error","reason":if cmd=="playlist_tracks"&&e=="api_http_403"{"playlist_access_restricted"}else{&e},"request_id":v["request_id"]})}
 }
}
fn same_account(profile:&Value,username:Option<&str>)->bool {username.is_some_and(|name|!name.is_empty()&&profile["id"].as_str()==Some(name))}
#[cfg(test)]mod tests {
 #[test]fn compatibility_login_never_mixes_accounts(){let p=serde_json::json!({"id":"account-a"});assert!(super::same_account(&p,Some("account-a")));assert!(!super::same_account(&p,Some("account-b")));assert!(!super::same_account(&p,None));assert!(!super::same_account(&serde_json::json!({"id":""}),Some("")));}
}

fn bounded_offset(kind:&str,offset:u64)->u64 { offset.min(if kind.starts_with("search"){990}else{10000}) }
fn profile_display(v:&Value,username:&str)->Option<Value>{
 let name=v["display_name"].as_str().or_else(||v["name"].as_str()).filter(|s|!s.trim().is_empty()&&*s!=username)?;
 Some(json!({"id":username,"display_name":name}))
}
#[cfg(test)] mod v110_tests {
 #[test]fn search_pages_stay_within_spotify_window(){assert_eq!(super::bounded_offset("search_artists",1000),990);assert_eq!(super::bounded_offset("search",u64::MAX),990);assert_eq!(super::bounded_offset("artist_albums",1000),1000);}
 #[test]fn profile_name_is_not_an_opaque_account_id(){let p=serde_json::json!({"name":"Spring Thông"});assert_eq!(super::profile_display(&p,"31opaque").unwrap()["display_name"],"Spring Thông");assert!(super::profile_display(&serde_json::json!({"name":"31opaque"}),"31opaque").is_none());}
}
