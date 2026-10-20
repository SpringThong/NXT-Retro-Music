use super::api::{self,Result,atomic_json,media_url,urn};
use serde_json::{Value,json};
use std::path::Path;

pub fn bookmarks_path()->Option<std::path::PathBuf>{std::env::var("NXT_SOUNDCLOUD_CREDENTIALS").ok().map(|p|Path::new(&p).with_file_name("bookmarks.json"))}
pub fn bookmarks()->Vec<Value>{
    let Some(p)=bookmarks_path()else{return Vec::new();};
    if std::fs::metadata(&p).ok().is_some_and(|m|m.len()>2*1024*1024){return Vec::new();}
    std::fs::read(p).ok().and_then(|s|serde_json::from_slice::<Value>(&s).ok()).and_then(|v|v.as_array().cloned()).unwrap_or_default()
        .into_iter().filter(|v|v["uri"].as_str().is_some_and(|s|urn(s,"tracks"))).take(500).collect()
}
pub fn favorite(v:&Value,change:bool)->Result<()> {
    let id=v["uri"].as_str().filter(|s|urn(s,"tracks")).ok_or("track_invalid")?;let mut rows=bookmarks();
    if change {
        rows.retain(|x|x["uri"].as_str()!=Some(id));
        if v["saved"].as_bool()==Some(true){if rows.len()>=500{return Err("bookmarks_full".into());}
            let mut t=json!({"uri":id,"id":id});
            for k in ["title","artist","album","cover","permalink","uploader"] {t[k]=json!(v[k].as_str().unwrap_or("").chars().take(500).collect::<String>());}
            t["duration_ms"]=json!(v["duration_ms"].as_u64().unwrap_or(0).min(i32::MAX as u64));rows.push(t);
        }
        atomic_json(&bookmarks_path().ok_or("credentials_save_failed")?,&json!(rows))?;
    }
    let saved=rows.iter().any(|x|x["uri"].as_str()==Some(id));
    crate::emit(json!({"event":"favorite","uri":id,"stream_id":v["stream_id"],"saved":saved,"local":true}));Ok(())
}
pub async fn cover(v:Value)->Result<()> {
    let url=reqwest::Url::parse(v["url"].as_str().ok_or("cover_invalid")?).map_err(|_|"cover_invalid")?;
    if !media_url(&url)||url.host_str()==Some("api.soundcloud.com"){return Err("cover_invalid".into());}
    let http=super::hls::client()?;let response=http.get(url).send().await.map_err(|_|"network_error")?;
    if !response.status().is_success(){return Err("cover_unavailable".into());}
    let data=api::bounded_bytes(response,4*1024*1024).await?;
    if !(data.starts_with(&[255,216,255])||data.starts_with(b"\x89PNG\r\n\x1a\n")){return Err("cover_invalid".into());}
    let path=std::env::var("NXT_SOUNDCLOUD_COVER").map_err(|_|"cover_invalid")?;
    let temp=Path::new(&path).with_extension("writing");std::fs::write(&temp,&data).map_err(|_|"cover_unavailable")?;
    std::fs::rename(temp,path).map_err(|_|"cover_unavailable")?;
    crate::emit(json!({"event":"cover","uri":v["uri"],"stream_id":v["stream_id"]}));Ok(())
}
fn normalized(s:&str)->String{s.chars().filter(|c|c.is_alphanumeric()).flat_map(char::to_lowercase).collect()}
pub fn matching_lyrics(row:&Value,request:&Value)->Option<String> {
    let title=request["title"].as_str()?;let artist=request["artist"].as_str()?;
    let duration=request["duration_ms"].as_u64()? as f64/1000.;
    if normalized(title)!=normalized(row["trackName"].as_str()?)||normalized(artist)!=normalized(row["artistName"].as_str()?)
        ||(duration-row["duration"].as_f64()?).abs()>3.{return None;}
    ["syncedLyrics","plainLyrics"].into_iter().find_map(|k|row[k].as_str().filter(|s|!s.trim().is_empty()&&s.len()<=256*1024).map(str::to_owned))
}
pub async fn lyrics(v:Value) {
    let fetch=async {
        let title=v["title"].as_str().ok_or("lyrics_unavailable")?;
        let artist=v["artist"].as_str().ok_or("lyrics_unavailable")?;
        let http=reqwest::Client::builder().timeout(std::time::Duration::from_secs(10)).user_agent("NXT-Retro-Music/1.2 (lyrics)").build().map_err(|_|"network_error")?;
        let res=http.get("https://lrclib.net/api/search").query(&[("track_name",title),("artist_name",artist)]).send().await.map_err(|_|"network_error")?;
        if !res.status().is_success(){return Err("lyrics_unavailable".into());}
        let rows=api::bounded_json(res).await?;
        let text=rows.as_array().and_then(|rows|rows.iter().find_map(|row|matching_lyrics(row,&v))).ok_or("lyrics_unavailable")?;
        let path=std::env::var("NXT_SOUNDCLOUD_LYRICS").map_err(|_|"lyrics_unavailable")?;let temp=Path::new(&path).with_extension("writing");
        std::fs::write(&temp,text).map_err(|_|"lyrics_unavailable")?;std::fs::rename(temp,path).map_err(|_|"lyrics_unavailable")?;
        Ok::<_,String>(())
    }.await;
    crate::emit(json!({"event":if fetch.is_ok(){"lyrics"}else{"lyrics_unavailable"},"uri":v["uri"],"stream_id":v["stream_id"],"source":"LRCLIB"}));
}
#[cfg(test)]mod tests {
    use super::*;
    #[test]fn lyrics_require_title_artist_and_duration_unicode(){
        let q=json!({"title":"ไทย 日本語","artist":"한국어","duration_ms":100000});
        let mut row=json!({"trackName":"ไทย 日本語","artistName":"한국어","duration":101,"syncedLyrics":"[00:00.000]Việt 中文"});
        assert!(matching_lyrics(&row,&q).is_some());row["duration"]=json!(120);assert!(matching_lyrics(&row,&q).is_none());row["duration"]=json!(100);row["artistName"]=json!("other");assert!(matching_lyrics(&row,&q).is_none());
    }
}
