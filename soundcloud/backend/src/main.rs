//! Native SoundCloud helper: public yt-dlp extraction, HLS -> NXT PCM.
mod api;
mod hls;
mod player;
mod extras;
#[cfg(test)]mod fixture;
use serde_json::{Value,json};
use std::{collections::HashMap,io::Write,sync::{Arc,Mutex,atomic::Ordering}};
use tokio::{io::{AsyncBufReadExt,BufReader},task::JoinHandle};
static OUTPUT:Mutex<()>=Mutex::new(());
pub fn emit(v:Value){let _guard=OUTPUT.lock().unwrap();let mut out=std::io::stdout().lock();let _=writeln!(out,"{v}");let _=out.flush();}
fn error(reason:&str,v:&Value){if reason!="cancelled"{emit(json!({"event":"error","reason":reason,"operation":v["cmd"],"uri":v.get("uri"),"stream_id":v.get("stream_id"),"request_id":v.get("request_id")}));}}
fn cancel(job:&mut Option<JoinHandle<()>>){if let Some(job)=job.take(){job.abort();}}
type Pages=Arc<tokio::sync::Mutex<HashMap<String,Value>>>;
async fn catalog(api:Arc<api::Api>,pages:Pages,v:&Value)->api::Result<Value>{
    let kind=v["cmd"].as_str().ok_or("command_invalid")?;
    let offset=v["offset"].as_u64().unwrap_or(0)as usize;
    if offset>20000||offset%api::PAGE!=0{return Err("page_invalid".into());}
    if kind=="saved_tracks"{let rows=extras::bookmarks();return Ok(json!({"event":"results","kind":kind,"request_id":v["request_id"],"offset":offset,"next_offset":offset+api::PAGE,"items":rows.iter().skip(offset).take(api::PAGE).collect::<Vec<_>>(),"has_next":rows.len()>offset+api::PAGE}));}
    let query=v["query"].as_str().unwrap_or("");let id=v["id"].as_str().unwrap_or("");
    let key=format!("{kind}\n{query}\n{id}\n{offset}");
    if offset>0{if let Some(rows)=pages.lock().await.get(&key){let mut rows=rows.clone();rows["request_id"]=v["request_id"].clone();return Ok(rows);}}
    let resource=api.extract(kind,query,id,offset).await?;
    let rows=resource["entries"].as_array().ok_or("api_invalid_json")?;
    if rows.len()>api::PAGE{return Err("response_too_large".into());}
    let items=rows.iter().filter_map(api::extracted_item).collect::<Vec<_>>();
    let rows=json!({"event":"results","kind":resource["nxt_kind"].as_str().unwrap_or(kind),"request_id":v["request_id"],"offset":offset,"next_offset":offset+api::PAGE,"items":items,"has_next":resource["nxt_has_next"].as_bool().unwrap_or(false)});
    let mut cache=pages.lock().await;if offset==0{cache.clear();}if cache.len()>=40{cache.clear();}cache.insert(key,rows.clone());Ok(rows)
}
#[tokio::main(worker_threads=2)]async fn main(){
    emit(json!({"event":"ready","build":"Shared-v1.9.7.6-SoundCloud","api":"yt-dlp-public-unofficial","credentials_required":false}));
    let path=std::env::var("NXT_SOUNDCLOUD_CREDENTIALS").unwrap_or_else(|_|"credentials.json".into());
    let api=match api::Api::new(path.into()){Ok(a)=>Arc::new(a),Err(e)=>{error(&e,&json!({"cmd":"login"}));return;}};
    let control=Arc::new(player::Control::new());let sink=match player::Sink::inherited(control.clone()){Ok(s)=>Arc::new(s),Err(e)=>{error(&e,&json!({"cmd":"login"}));return;}};
    let pages:Pages=Arc::new(tokio::sync::Mutex::new(HashMap::new()));
    let mut audio=None;let mut browse=None;let mut cover=None;let mut lyrics=None;let mut login=None;
    // read_line is capped after each line; the parent sends at most 4 KiB.
    let mut input=BufReader::new(tokio::io::stdin());let mut line=String::new();
    loop {
        line.clear();let got=input.read_line(&mut line).await;match got{Ok(0)|Err(_)=>break,_=>{}}
        if line.len()>8192{error("command_too_large",&json!({}));break;}
        let Ok(v)=serde_json::from_str::<Value>(&line)else{continue;};
        match v["cmd"].as_str().unwrap_or(""){
            "quit"=>break,
            "login"=>{cancel(&mut login);let a=api.clone();login=Some(tokio::spawn(async move{match a.ready().await{Ok(_)=>emit(json!({"event":"logged_in","display_name":"SoundCloud · Public","mode":"public_unofficial"})),Err(e)=>error(&e,&v)}}));},
            "search"|"search_playlists"|"playlist_tracks"|"resolve"|"saved_tracks"=>{cancel(&mut browse);let a=api.clone();let p=pages.clone();browse=Some(tokio::spawn(async move{match catalog(a,p,&v).await{Ok(rows)=>emit(rows),Err(e)=>error(&e,&v)}}));},
            "play"|"seek"=>{
                let epoch=v["stream_id"].as_u64().unwrap_or(0)as u32;if epoch==0{error("track_invalid",&v);continue;}
                control.epoch.store(epoch,Ordering::SeqCst);control.paused.store(v["paused"].as_bool().unwrap_or(false),Ordering::SeqCst);cancel(&mut audio);
                let a=api.clone();let s=sink.clone();let c=control.clone();audio=Some(tokio::spawn(async move{if let Err(e)=player::run(a,s,c,v.clone()).await{error(&e,&v);}}));
            },
            "pause"=>control.paused.store(true,Ordering::SeqCst),
            "resume"=>control.paused.store(false,Ordering::SeqCst),
            "stop"=>{control.epoch.store(0,Ordering::SeqCst);cancel(&mut audio);cancel(&mut cover);cancel(&mut lyrics);},
            "favorite"|"check_favorite"=>{if let Err(_)=extras::favorite(&v,v["cmd"]=="favorite"){emit(json!({"event":"favorite_error","uri":v["uri"],"stream_id":v["stream_id"]}));}},
            "source_link"=>{
                if let Some(url)=v["url"].as_str().and_then(|s|reqwest::Url::parse(s).ok()).filter(api::public_url){
                    if let Ok(code)=qrcode::QrCode::new(url.as_str().as_bytes()){
                        let n=code.width();if n<=57{let colors=code.to_colors();let rows=(0..n).map(|y|(0..n).map(|x|if colors[y*n+x]==qrcode::Color::Dark{'1'}else{'0'}).collect::<String>()).collect::<Vec<_>>();emit(json!({"event":"source_link","uri":v["uri"],"stream_id":v["stream_id"],"qr":rows}));}
                    }
                }
            },
            "cover"=>{cancel(&mut cover);cover=Some(tokio::spawn(async move{let _=extras::cover(v).await;}));},
            "lyrics"=>{cancel(&mut lyrics);lyrics=Some(tokio::spawn(extras::lyrics(v)));},
            _=>error("command_invalid",&v),
        }
    }
    control.epoch.store(0,Ordering::SeqCst);for job in [&mut audio,&mut browse,&mut cover,&mut lyrics,&mut login]{cancel(job);}
}
