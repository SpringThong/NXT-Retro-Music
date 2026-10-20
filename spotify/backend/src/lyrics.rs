use crate::{emit,diagnostic};
use librespot_core::{session::Session,SpotifyId};
use serde_json::{Value,json};
use std::path::Path;
const MAX_TEXT:usize=256*1024;
fn one_line(s:&str)->String{s.replace(['\r','\n']," ")}
pub fn spotify_text(v:&Value)->Option<String>{
 let lyrics=&v["lyrics"];let lines=lyrics["lines"].as_array()?;
 let synced=matches!(lyrics["syncType"].as_str(),Some("LINE_SYNCED"|"WORD_SYNCED"));
 let mut result=String::new();
 for line in lines.iter().take(512){let words=one_line(line["words"].as_str().unwrap_or(""));if words.trim().is_empty(){continue;}
  if synced {let ms=line["startTimeMs"].as_str().and_then(|s|s.parse::<u64>().ok()).or_else(||line["startTimeMs"].as_u64())?;if ms>i32::MAX as u64{return None;}result.push_str(&format!("[{:02}:{:02}.{:03}]",ms/60000,ms/1000%60,ms%1000));}
  result.push_str(&words);result.push('\n');if result.len()>MAX_TEXT{return None;}
 }
 if result.is_empty(){None}else{Some(result)}
}
fn lrclib_text(v:&Value)->Option<String>{["syncedLyrics","plainLyrics"].iter().find_map(|k|v[k].as_str().filter(|s|!s.trim().is_empty()&&s.len()<=MAX_TEXT).map(str::to_owned))}
fn normalized(s:&str)->String{s.chars().filter(|c|c.is_alphanumeric()).flat_map(char::to_lowercase).collect()}
async fn fallback(v:&Value)->Option<String>{
 let http=reqwest::Client::builder().timeout(std::time::Duration::from_secs(10)).user_agent("NXT-Retro-Music/1.0 (lyrics)").build().ok()?;
 let artist=v["artist"].as_str()?;let title=v["title"].as_str()?;let album=v["album"].as_str().unwrap_or("");let duration=v["duration_ms"].as_u64().unwrap_or(0)/1000;
 let primary=artist.split(", ").next().unwrap_or(artist);
 let params=[("artist_name",primary.to_owned()),("track_name",title.to_owned()),("album_name",album.to_owned()),("duration",duration.to_string())];
 if let Ok(res)=http.get("https://lrclib.net/api/get").query(&params).send().await {if res.status().is_success(){if let Ok(body)=res.json::<Value>().await {if let Some(text)=lrclib_text(&body){return Some(text);}}}}
 // Fallback only accepts matching title/artist and duration, never the first random hit.
 let res=http.get("https://lrclib.net/api/search").query(&[("track_name",title),("artist_name",primary)]).send().await.ok()?;if !res.status().is_success(){return None;}
 let rows=res.json::<Value>().await.ok()?;
 rows.as_array()?.iter().find_map(|x|{
  let close=(x["duration"].as_f64().unwrap_or(-1000.)-duration as f64).abs()<=3.;
  if normalized(x["trackName"].as_str().unwrap_or(""))==normalized(title)&&normalized(x["artistName"].as_str().unwrap_or(""))==normalized(primary)&&close{lrclib_text(x)}else{None}
 })
}
fn write(path:&Path,text:&str)->std::io::Result<()>{let tmp=path.with_extension("writing");std::fs::write(&tmp,text)?;std::fs::rename(tmp,path)}
pub async fn run(session:Session,id:SpotifyId,v:Value){
 let uri=v["uri"].clone();let Ok(path)=std::env::var("NXT_SPOTIFY_LYRICS") else{return;};let path=Path::new(&path);
 let cache_dir=path.parent().unwrap_or(Path::new(".")).join("lyrics-cache");let _=std::fs::create_dir_all(&cache_dir);let cache=cache_dir.join(format!("{}.txt",id.to_base62()));
 let mut source="Cache";
 let mut text=std::fs::metadata(&cache).ok().filter(|m|m.len()<=MAX_TEXT as u64).and_then(|_|std::fs::read_to_string(&cache).ok()).filter(|s|!s.is_empty()&&s.len()<=MAX_TEXT);
 if text.is_none(){source="Spotify";if let Ok(Ok(bytes))=tokio::time::timeout(std::time::Duration::from_secs(8),session.spclient().get_lyrics(&id)).await {if let Ok(body)=serde_json::from_slice::<Value>(&bytes){text=spotify_text(&body);}}}
 if text.is_none(){source="LRCLIB";text=fallback(&v).await;}
 if let Some(text)=text {let _=write(&cache,&text);if write(path,&text).is_ok(){diagnostic(&format!("Lyrics: available from {source}"));emit(json!({"event":"lyrics","uri":uri,"source":source}));return;}}
 diagnostic("Lyrics: unavailable for this track");emit(json!({"event":"lyrics_unavailable","uri":uri}));
}
#[cfg(test)]mod tests {
 #[test]fn timestamped_unicode_lyrics_preserve_milliseconds(){let v=serde_json::json!({"lyrics":{"syncType":"LINE_SYNCED","lines":[{"startTimeMs":"12345","words":"Việt 한국어 日本語 中文 ไทย"}]}});assert_eq!(super::spotify_text(&v).unwrap(),"[00:12.345]Việt 한국어 日本語 中文 ไทย\n");}
 #[test]fn plain_text_does_not_claim_sync(){let v=serde_json::json!({"lyrics":{"syncType":"UNSYNCED","lines":[{"startTimeMs":"0","words":"Test line"}]}});assert_eq!(super::spotify_text(&v).unwrap(),"Test line\n");}
 #[test]fn malformed_or_overlong_lyrics_are_rejected(){let v=serde_json::json!({"lyrics":{"syncType":"LINE_SYNCED","lines":[{"startTimeMs":"bad","words":"test"}]}});assert!(super::spotify_text(&v).is_none());let v=serde_json::json!({"lyrics":{"syncType":"UNSYNCED","lines":[{"words":"a".repeat(super::MAX_TEXT+1)}]}});assert!(super::spotify_text(&v).is_none());}
}
