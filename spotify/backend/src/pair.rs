use crate::{api,emit,diagnostic};
use librespot_oauth::{DeviceAuthClientBuilder,OAuthError};
use serde_json::json;
use std::{io::Write,os::unix::fs::OpenOptionsExt,time::{Instant,SystemTime,UNIX_EPOCH}};
pub async fn run(){
 diagnostic("Pair: requesting Spotify device authorization");
 let client=match DeviceAuthClientBuilder::new(api::PLAYBACK_CLIENT_ID,vec!["streaming","user-read-private","playlist-read-private","playlist-read-collaborative","user-library-read","user-library-modify"]).build(){Ok(c)=>c,Err(_)=>{emit(json!({"event":"error","reason":"pair_setup_failed"}));return;}};
 let auth=match tokio::time::timeout(std::time::Duration::from_secs(20),client.request_device_code_async()).await{
  Ok(Ok(a))=>a,Ok(Err(e))=>{diagnostic(&format!("Pair: device request failed: {}",safe_reason(&e)));emit(json!({"event":"error","reason":"pair_request_failed"}));return;},Err(_)=>{diagnostic("Pair: device request timed out");emit(json!({"event":"error","reason":"pair_request_failed"}));return;}
 };
 // Only a public user code/verification URL leaves this helper. Device code and tokens stay private.
 let url=auth.url();let qr=qr_rows(url);
 emit(json!({"event":"pair_code","code":auth.user_code(),"url":url,"qr":qr}));
 let token=match tokio::time::timeout(std::time::Duration::from_secs(600),client.poll_for_token_async(&auth)).await{
  Ok(Ok(t))=>t,Ok(Err(e))=>{diagnostic(&format!("Pair: approval failed: {}",safe_reason(&e)));emit(json!({"event":"error","reason":"pair_approval_failed"}));return;},Err(_)=>{diagnostic("Pair: approval timed out after 10 minutes");emit(json!({"event":"error","reason":"pair_approval_failed"}));return;}
 };
 let path=match std::env::var("NXT_SPOTIFY_PLAYBACK_CREDS"){Ok(p)=>p,Err(_)=>{emit(json!({"event":"error","reason":"pair_save_failed"}));return;}};
 let ttl=token.expires_at.saturating_duration_since(Instant::now()).as_secs();
 let data=json!({"client_id":api::PLAYBACK_CLIENT_ID,"access_token":token.access_token,"refresh_token":token.refresh_token,"expires_in":ttl,"obtained_at":SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs(),"token_type":token.token_type,"scope":token.scopes.join(" ")});
 if save(&path,&data.to_string()).is_err(){emit(json!({"event":"error","reason":"pair_save_failed"}));return;}
 diagnostic("Pair: playback credentials saved; no tokens in logs");emit(json!({"event":"paired"}));
}
fn safe_reason(e:&OAuthError)->&'static str{
 let description=match e{OAuthError::DeviceCode{e}|OAuthError::ExchangeDeviceCode{e}=>e.as_str(),_=>return "oauth_setup_error"};
 // Error strings can contain response bodies. Log only known classifications.
 for reason in ["unauthorized_client","invalid_client","invalid_scope","access_denied","expired_token","invalid_request","slow_down","certificate","timed out","connection","dns"]{if description.contains(reason){return reason;}}
 "oauth_error"
}
pub fn qr_rows(url:&str)->Vec<String>{
 match qrcode::QrCode::new(url.as_bytes()){
  Ok(q) if q.width()<=57=>{let w=q.width();(0..w).map(|y|(0..w).map(|x|if q[(x,y)]==qrcode::Color::Dark{'1'}else{'0'}).collect()).collect()},_=>Vec::new()
 }
}
fn save(path:&str,data:&str)->std::io::Result<()>{
 let tmp=format!("{path}.tmp");let mut f=std::fs::OpenOptions::new().write(true).create(true).truncate(true).mode(0o600).open(&tmp)?;
 // Existing .tmp files must also have private permissions.
 std::fs::set_permissions(&tmp,std::fs::Permissions::from_mode(0o600))?;
 f.write_all(data.as_bytes())?;f.sync_all()?;std::fs::rename(tmp,path)
}
use std::os::unix::fs::PermissionsExt;
#[cfg(test)]mod tests{
 #[test]fn saved_pair_credentials_are_private_and_atomic(){let p=std::env::temp_dir().join(format!("nxt-pair-{}.json",std::process::id()));let tmp=format!("{}.tmp",p.display());std::fs::write(&tmp,"old").unwrap();super::save(p.to_str().unwrap(),"new").unwrap();use std::os::unix::fs::PermissionsExt;assert_eq!(std::fs::metadata(&p).unwrap().permissions().mode()&0o777,0o600);assert_eq!(std::fs::read_to_string(&p).unwrap(),"new");assert!(!std::path::Path::new(&tmp).exists());std::fs::remove_file(p).unwrap();}
 #[test]fn qr_url_builds_square_binary_matrix(){let rows=super::qr_rows("https://spotify.com/pair?code=ABCDEF");assert!(!rows.is_empty());for row in &rows{assert_eq!(row.len(),rows.len());assert!(row.bytes().all(|x|x==b'0'||x==b'1'));}}
 #[test]fn pairing_diagnostics_never_forward_response_secrets(){let e=super::OAuthError::DeviceCode{e:"FAKE-PRIVATE-RESPONSE".into()};assert_eq!(super::safe_reason(&e),"oauth_error");let e=super::OAuthError::ExchangeDeviceCode{e:"access_denied: FAKE-PRIVATE-RESPONSE".into()};assert_eq!(super::safe_reason(&e),"access_denied");}
}
