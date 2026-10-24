// SPDX-License-Identifier: GPL-3.0-only
// QR-session client-library protocol. Wire formats/routes cross-checked against
// the bundled librespot SDK and devgianlu/go-librespot's collection/rootlist code.
// No developer Web API calls, borrowed app IDs, cookies or identity switching.
use crate::api::Api;
use base64::Engine;
use librespot_core::{session::Session, SpotifyId};
use librespot_protocol::{collection2v2 as collection, extended_metadata as ext,
    extension_kind::ExtensionKind, metadata as meta, playlist4_external as playlist};
use protobuf::{EnumOrUnknown, Message};
use reqwest::Method;
use serde_json::{json, Value};
use std::{collections::{HashSet, VecDeque}, future::Future, time::{Duration, Instant}};

const PAGE: usize = 20;
const PROTO: &str = "application/x-protobuf";
const COLLECTION: &str = "application/vnd.collection-v2.spotify.proto";

pub trait Service: Clone + Send + Sync {
    fn request(&self, method: Method, path: &str, body: Option<Vec<u8>>, content_type: &str)
        -> impl Future<Output = Result<Vec<u8>, String>> + Send;
}
#[derive(Clone)]
pub struct LiveService(Api);
impl Service for LiveService {
    async fn request(&self, method: Method, path: &str, body: Option<Vec<u8>>, content_type: &str) -> Result<Vec<u8>, String> {
        self.0.native_bytes(method, path, body, content_type).await
    }
}
#[derive(Clone)]
struct Entry { key: String, value: Value, at: Instant, bytes: usize }
#[derive(Clone)]
struct Cache { entries: VecDeque<Entry>, bytes: usize, limit: usize, capacity: usize, ttl: Duration }
impl Cache {
    fn new(capacity: usize, limit: usize, ttl: u64) -> Self {
        Self { entries: VecDeque::new(), bytes: 0, limit, capacity, ttl: Duration::from_secs(ttl) }
    }
    fn remove(&mut self, at: usize) -> Entry { let e = self.entries.remove(at).unwrap(); self.bytes -= e.bytes; e }
    fn get(&mut self, key: &str) -> Option<Value> {
        let index = self.entries.iter().position(|e| e.key == key)?;
        let e = self.remove(index);
        if e.at.elapsed() >= self.ttl { return None; }
        let value = e.value.clone(); self.bytes += e.bytes; self.entries.push_back(e); Some(value)
    }
    fn put(&mut self, key: String, value: Value) {
        if let Some(i) = self.entries.iter().position(|e| e.key == key) { self.remove(i); }
        let bytes = key.len() + value.to_string().len();
        if bytes > self.limit { return; }
        while !self.entries.is_empty() && (self.bytes + bytes > self.limit || self.entries.len() >= self.capacity) { self.remove(0); }
        self.bytes += bytes; self.entries.push_back(Entry { key, value, bytes, at: Instant::now() });
    }
    fn clear(&mut self) { self.entries.clear(); self.bytes = 0; }
}

#[derive(Clone)]
pub struct NativeCatalog<S: Service = LiveService> {
    service: S, username: String, pages: Cache, metadata: Cache, cursors: Cache,
}
impl NativeCatalog<LiveService> {
    pub fn new(session: Session) -> Result<Self, String> {
        let username = session.username();
        Ok(Self::with_service(LiveService(Api::from_session(session)?), username))
    }
}
impl<S: Service> NativeCatalog<S> {
    fn with_service(service: S, username: String) -> Self {
        Self { service, username, pages: Cache::new(24, 512 * 1024, 120),
            metadata: Cache::new(192, 1024 * 1024, 900), cursors: Cache::new(24, 512 * 1024, 180) }
    }
    async fn request(&self, method: Method, path: &str, body: Option<Vec<u8>>, content_type: &str) -> Result<Vec<u8>, String> {
        tokio::time::timeout(Duration::from_secs(12), self.service.request(method, path, body, content_type))
            .await.map_err(|_| "native_timeout".to_owned())?
    }
    pub async fn profile(&self) -> Result<Value, String> {
        let path = format!("/user-profile-view/v3/profile/{}?playlist_limit=0&artist_limit=0", segment(&self.username));
        let b = self.request(Method::GET, &path, None, "application/json").await?;
        let v: Value = serde_json::from_slice(&b).map_err(|_| "native_profile_invalid")?;
        let name = v["name"].as_str().or_else(|| v["display_name"].as_str()).filter(|s| !s.trim().is_empty()).unwrap_or("Spotify");
        Ok(json!({"id": self.username, "display_name": if name == self.username {"Spotify"} else {name}}))
    }
    pub async fn probe(&mut self) -> Result<(), String> {
        let (rows, next, more) = self.root_page(0).await?;
        self.pages.put(query_key("playlists", "", "", 0), result("playlists", 0, next, more, rows));
        Ok(())
    }
    pub async fn query(&mut self, request: &Value) -> Value {
        let command = request["cmd"].as_str().unwrap_or("");
        let offset = request["offset"].as_u64().unwrap_or(0).min(10000);
        let query = if command.starts_with("search") {request["query"].as_str().unwrap_or("").trim()} else {""};
        let id = request["id"].as_str().unwrap_or("");
        if command.starts_with("search") && query.is_empty() { return error("empty_search_query", request); }
        let key = query_key(command, query, id, offset);
        let outcome = if let Some(cached) = self.pages.get(&key) { Ok(cached) } else {
            match tokio::time::timeout(Duration::from_secs(28), self.query_inner(command, query, id, offset)).await {
                Ok(Ok(v)) => { self.pages.put(key, v.clone()); Ok(v) },
                Ok(Err(e)) => Err(e), Err(_) => Err("native_timeout".into())
            }
        };
        match outcome {
            Ok(mut v) => { v["request_id"] = request["request_id"].clone(); crate::diagnostic(&format!("Native library: {command}, offset={offset}, items={}", v["items"].as_array().map_or(0, Vec::len))); v },
            Err(e) => { crate::diagnostic(&format!("Native library: {command} failed: {e}")); error(&e, request) }
        }
    }
    async fn query_inner(&mut self, kind: &str, q: &str, id: &str, offset: u64) -> Result<Value, String> {
        match kind {
            "playlists" => { let (rows, next, more) = self.root_page(offset).await?; Ok(result(kind, offset, next, more, rows)) },
            "search_playlists" => {
                let mut at = offset; let mut more = false; let mut matches = Vec::new();
                for _ in 0..3 {
                    let (rows, next, has_next) = self.root_page(at).await?;
                    matches.extend(rows.into_iter().filter(|r| r["title"].as_str().unwrap_or("").to_lowercase().contains(&q.to_lowercase())));
                    at = next; more = has_next;
                    if !matches.is_empty() || !more { break; }
                }
                Ok(result(kind, offset, at, more, matches))
            },
            "saved_tracks" | "saved_albums" => self.collection_page(kind, offset).await,
            "playlist_tracks" if valid_id(id) => {
                let path = format!("/playlist/v2/playlist/{id}?from={offset}&length={PAGE}&decorate=revision,attributes,length");
                let b = self.request(Method::GET, &path, None, PROTO).await?;
                let p = playlist::SelectedListContent::parse_from_bytes(&b).map_err(|_| "native_playlist_invalid")?;
                let list = p.contents.get_or_default();
                let uris: Vec<_> = list.items.iter().filter_map(|i| valid_uri(i.uri(), "track")).collect();
                let next = offset + list.items.len() as u64;
                let more = !list.items.is_empty() && (list.truncated() || next < p.length().max(0) as u64);
                let rows = self.entities(&uris, "track").await?;
                Ok(result(kind, offset, next, more, rows))
            },
            "album_tracks" | "artist_albums" if valid_id(id) => {
                let entity_kind = if kind == "album_tracks" {"album"} else {"artist"};
                let uri = format!("spotify:{entity_kind}:{id}");
                self.entities(std::slice::from_ref(&uri), entity_kind).await?;
                let parent = self.metadata.get(&uri).ok_or("native_metadata_unavailable")?;
                let field = if kind == "album_tracks" {"_tracks"} else {"_albums"};
                let all: Vec<_> = parent[field].as_array().into_iter().flatten().filter_map(|v| v.as_str().map(str::to_owned)).collect();
                let begin = (offset as usize).min(all.len()); let end = (begin + PAGE).min(all.len());
                let rows = self.entities(&all[begin..end], if kind == "album_tracks" {"track"} else {"album"}).await?;
                Ok(result(kind, offset, end as u64, end < all.len(), rows))
            },
            "search" | "search_artists" | "search_albums" => self.search(kind, q, offset).await,
            "album_tracks" | "artist_albums" | "playlist_tracks" => Err("invalid_item_id".into()),
            _ => Err("native_command_unknown".into()),
        }
    }
    async fn root_page(&mut self, offset: u64) -> Result<(Vec<Value>, u64, bool), String> {
        let path = format!("/playlist/v2/user/{}/rootlist?decorate=revision,attributes,length,owner,capabilities,status_code&from={offset}&length={PAGE}", segment(&self.username));
        let b = self.request(Method::GET, &path, None, PROTO).await?;
        let p = playlist::SelectedListContent::parse_from_bytes(&b).map_err(|_| "native_rootlist_invalid")?;
        let list = p.contents.get_or_default(); let mut rows = Vec::new();
        for (n, item) in list.items.iter().enumerate() {
            let Some(uri) = valid_uri(item.uri(), "playlist") else { continue; };
            let meta = list.meta_items.get(n); let attrs = meta.map(|m| m.attributes.get_or_default());
            if meta.is_some_and(|m| m.status_code() >= 400) || attrs.is_some_and(|a| a.deleted_by_owner()) { continue; }
            let name = attrs.map(|a| a.name()).filter(|s| !s.is_empty()).map(str::to_owned).unwrap_or_else(|| format!("Playlist · {}", &uri[uri.len()-6..]));
            let owner = meta.map(|m| m.owner_username()).unwrap_or("");
            rows.push(json!({"title":name,"id":uri.rsplit(':').next(),"uri":uri,"artist":owner}));
        }
        let next = offset + list.items.len() as u64;
        let more = !list.items.is_empty() && (list.truncated() || next < p.length().max(0) as u64);
        Ok((rows, next, more))
    }
    async fn collection_page(&mut self, kind: &str, offset: u64) -> Result<Value, String> {
        let key = format!("collection:{kind}:{offset}");
        let mut state = if offset == 0 {json!({"buffer":[],"token":"","done":false})} else {self.cursors.get(&key).ok_or("native_page_expired")?};
        let prefix = if kind == "saved_tracks" {"track"} else {"album"};
        let mut pending: Vec<String> = state["buffer"].as_array().into_iter().flatten().filter_map(|v| v.as_str().map(str::to_owned)).collect();
        for _ in 0..2 {
            if pending.len() >= PAGE || state["done"] == true { break; }
            let req = collection::PageRequest { username: self.username.clone(), set: "collection".into(), pagination_token: state["token"].as_str().unwrap_or("").to_owned(), limit: 100, ..Default::default() };
            let b = self.request(Method::POST, "/collection/v2/paging", Some(req.write_to_bytes().map_err(|_| "native_encode")?), COLLECTION).await?;
            let page = collection::PageResponse::parse_from_bytes(&b).map_err(|_| "native_collection_invalid")?;
            let next = page.next_page_token;
            if !next.is_empty() && state["token"] == next { return Err("native_page_loop".into()); }
            for item in page.items { if !item.is_removed { if let Some(uri) = valid_uri(&item.uri, prefix) { if !pending.contains(&uri) { pending.push(uri); } } } }
            state["done"] = json!(next.is_empty()); state["token"] = json!(next);
        }
        let uris: Vec<_> = pending.drain(..pending.len().min(PAGE)).collect();
        state["buffer"] = json!(pending);
        let more = !state["buffer"].as_array().unwrap().is_empty() || state["done"] != true;
        let next = offset + uris.len().max(1) as u64;
        if more { self.cursors.put(format!("collection:{kind}:{next}"), state); }
        let rows = self.entities(&uris, prefix).await?;
        Ok(result(kind, offset, next, more, rows))
    }
    async fn entities(&mut self, uris: &[String], kind: &str) -> Result<Vec<Value>, String> {
        let missing: Vec<_> = uris.iter().filter(|uri| self.metadata.get(uri).is_none()).cloned().collect();
        if !missing.is_empty() {
            let extension_kind = match kind {"track" => ExtensionKind::TRACK_V4, "album" => ExtensionKind::ALBUM_V4, _ => ExtensionKind::ARTIST_V4};
            let request = ext::BatchedEntityRequest { entity_request: missing.iter().take(PAGE).map(|uri| ext::EntityRequest { entity_uri: uri.clone(), query: vec![ext::ExtensionQuery { extension_kind: EnumOrUnknown::new(extension_kind), ..Default::default() }], ..Default::default() }).collect(), ..Default::default() };
            let b = self.request(Method::POST, "/extended-metadata/v0/extended-metadata", Some(request.write_to_bytes().map_err(|_| "native_encode")?), PROTO).await?;
            let response = ext::BatchedExtensionResponse::parse_from_bytes(&b).map_err(|_| "native_metadata_invalid")?;
            for group in response.extended_metadata {
                if group.extension_kind.enum_value_or_default() != extension_kind { continue; }
                for entity in group.extension_data {
                    if entity.header.get_or_default().status_code != 200 || !missing.contains(&entity.entity_uri) { continue; }
                    if let Some(data) = entity.extension_data.as_ref() {
                        if let Some(v) = decode_entity(kind, &entity.entity_uri, &data.value) { self.metadata.put(entity.entity_uri.clone(), v); }
                    }
                }
            }
        }
        let rows: Vec<_> = uris.iter().filter_map(|uri| self.metadata.get(uri)).collect();
        if !uris.is_empty() && rows.is_empty() { return Err("native_metadata_unavailable".into()); }
        Ok(rows)
    }
    async fn search(&mut self, kind: &str, query: &str, offset: u64) -> Result<Value, String> {
        let cursor_key = format!("search:{kind}:{query}:{offset}");
        let mut state = if offset == 0 {
            let uri = format!("spotify:search:{}", segment(query).replace("%20", "+"));
            let path = format!("/context-resolve/v1/{uri}");
            let b = self.request(Method::GET, &path, None, "application/json").await?;
            let v: Value = serde_json::from_slice(&b).map_err(|_| "native_context_invalid")?;
            let mut state = json!({"uris":[],"links":[],"seen_links":[],"seen_entities":[],"scanned":0});
            append_context(&mut state, &v)?; state
        } else { self.cursors.get(&cursor_key).ok_or("native_page_expired")? };
        let mut rows = Vec::new();
        for _ in 0..2 {
            let uris = state["uris"].as_array_mut().ok_or("native_context_invalid")?;
            let take: Vec<String> = uris.drain(..uris.len().min(PAGE)).filter_map(|v| v.as_str().map(str::to_owned)).collect();
            if !take.is_empty() {
                let tracks = self.entities(&take, "track").await?;
                if kind == "search" { rows = tracks; } else {
                    let mut seen: HashSet<String> = state["seen_entities"].as_array().into_iter().flatten().filter_map(|v| v.as_str().map(str::to_owned)).collect();
                    for track in tracks {
                        let values: Vec<Value> = if kind == "search_artists" {track["_artists"].as_array().cloned().unwrap_or_default()} else {vec![track["_album"].clone()]};
                        for value in values {
                            if let Some(id) = value["id"].as_str().filter(|id| valid_id(id)) { if seen.insert(id.to_owned()) { rows.push(value); } }
                        }
                    }
                    state["seen_entities"] = json!(seen.into_iter().take(1000).collect::<Vec<_>>());
                }
                state["scanned"] = json!(state["scanned"].as_u64().unwrap_or(0) + take.len() as u64);
                if !rows.is_empty() { break; }
            }
            let links = state["links"].as_array_mut().ok_or("native_context_invalid")?;
            if links.is_empty() { break; }
            let url = links.remove(0).as_str().ok_or("native_context_invalid")?.to_owned();
            if state["seen_links"].as_array().unwrap().iter().any(|v| v == &url) { return Err("native_page_loop".into()); }
            state["seen_links"].as_array_mut().unwrap().push(json!(url));
            let path = context_path(&url).ok_or("native_invalid_path")?;
            let b = self.request(Method::GET, &path, None, "application/json").await?;
            let v: Value = serde_json::from_slice(&b).map_err(|_| "native_context_invalid")?;
            append_context(&mut state, &v)?;
        }
        let more = state["scanned"].as_u64().unwrap_or(0) < 1000 && (!state["uris"].as_array().unwrap().is_empty() || !state["links"].as_array().unwrap().is_empty());
        let next = offset + PAGE as u64;
        if more { self.cursors.put(format!("search:{kind}:{query}:{next}"), state); }
        Ok(result(kind, offset, next, more, rows))
    }
    pub async fn saved(&self, uri: &str) -> Result<bool, String> {
        if valid_uri(uri, "track").is_none() { return Err("invalid_track_uri".into()); }
        let request = collection::ContainsRequest { username: self.username.clone(), set: "collection".into(), items: vec![collection::CollectionItem {uri: uri.into(), ..Default::default()}], ..Default::default() };
        let b = self.request(Method::POST, "/collection/v2/contains", Some(request.write_to_bytes().map_err(|_| "native_encode")?), COLLECTION).await?;
        let response = collection::ContainsResponse::parse_from_bytes(&b).map_err(|_| "native_collection_invalid")?;
        if response.found.len() != 1 { return Err("native_collection_invalid".into()); }
        Ok(response.found[0])
    }
    pub async fn favorite(&mut self, uri: &str, value: bool) -> Result<(), String> {
        if valid_uri(uri, "track").is_none() { return Err("invalid_track_uri".into()); }
        let request = collection::WriteRequest { username: self.username.clone(), set: "collection".into(), items: vec![collection::CollectionItem { uri: uri.into(), is_removed: !value, added_at: if value {std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap_or_default().as_secs().min(i32::MAX as u64) as i32} else {0}, ..Default::default() }], ..Default::default() };
        self.request(Method::POST, "/collection/v2/write", Some(request.write_to_bytes().map_err(|_| "native_encode")?), COLLECTION).await?;
        self.pages.clear(); self.cursors.clear(); Ok(())
    }
}

fn valid_id(id: &str) -> bool { id.len() == 22 && id.bytes().all(|b| b.is_ascii_alphanumeric()) }
fn valid_uri(uri: &str, kind: &str) -> Option<String> { let prefix = format!("spotify:{kind}:"); let id = uri.strip_prefix(&prefix)?; valid_id(id).then(|| uri.to_owned()) }
fn segment(s: &str) -> String { let mut out = String::new(); for b in s.bytes() { if b.is_ascii_alphanumeric() || b == b'-' || b == b'_' {out.push(b as char)} else {out.push_str(&format!("%{b:02X}"))} } out }
fn query_key(kind: &str, q: &str, id: &str, offset: u64) -> String { json!([kind, q, id, offset]).to_string() }
fn present(row: Value) -> Value { let mut out = serde_json::Map::new(); for key in ["title","artist","album","duration_ms","uri","cover","id"] { if !row[key].is_null() {out.insert(key.into(),row[key].clone());} } Value::Object(out) }
fn result(kind: &str, offset: u64, next: u64, more: bool, rows: Vec<Value>) -> Value { json!({"event":"results","kind":kind,"offset":offset,"next_offset":next,"has_next":more && next > offset,"items":rows.into_iter().map(present).collect::<Vec<_>>()}) }
fn error(reason: &str, req: &Value) -> Value { json!({"event":"error","reason":reason,"request_id":req["request_id"]}) }
fn id_from_gid(gid: &[u8]) -> Option<String> { SpotifyId::from_raw(gid).ok().map(|id| id.to_base62()) }
fn artist_row(artist: &meta::Artist) -> Value { json!({"id":id_from_gid(artist.gid()),"title":artist.name(),"artist":"Artist"}) }
fn image_url(images: &[meta::Image]) -> Value {
    images.iter().find(|i| i.file_id().len() == 20 && i.width() >= 300 && i.width() <= 640).or_else(|| images.iter().find(|i| i.file_id().len() == 20))
        .map(|i| json!(format!("https://i.scdn.co/image/{}",i.file_id().iter().map(|b|format!("{b:02x}")).collect::<String>()))).unwrap_or(Value::Null)
}
fn album_row(album: &meta::Album) -> Value {
    let names = album.artist.iter().map(|a| a.name()).collect::<Vec<_>>().join(", ");
    json!({"id":id_from_gid(album.gid()),"title":album.name(),"artist":names,"cover":image_url(if album.cover.is_empty(){&album.cover_group.get_or_default().image}else{&album.cover})})
}
fn decode_entity(kind: &str, uri: &str, bytes: &[u8]) -> Option<Value> {
    match kind {
        "track" => {
            let t = meta::Track::parse_from_bytes(bytes).ok()?;
            if t.name().is_empty() { return None; }
            let album = t.album.get_or_default();
            Some(json!({"uri":uri,"title":t.name(),"artist":t.artist.iter().map(|a|a.name()).collect::<Vec<_>>().join(", "),"album":album.name(),"duration_ms":t.duration(),"cover":image_url(if album.cover.is_empty(){&album.cover_group.get_or_default().image}else{&album.cover}),"_artists":t.artist.iter().map(artist_row).collect::<Vec<_>>(),"_album":album_row(album)}))
        },
        "album" => {
            let a = meta::Album::parse_from_bytes(bytes).ok()?; let mut row = album_row(&a); row["uri"] = json!(uri);
            row["_tracks"] = json!(a.disc.iter().flat_map(|d| d.track.iter()).filter_map(|t|id_from_gid(t.gid()).map(|id|format!("spotify:track:{id}"))).collect::<Vec<_>>()); Some(row)
        },
        "artist" => {
            let a = meta::Artist::parse_from_bytes(bytes).ok()?; let mut row = artist_row(&a); row["uri"] = json!(uri);
            let mut seen = HashSet::new();
            row["_albums"] = json!(a.album_group.iter().chain(&a.single_group).chain(&a.compilation_group).filter_map(|g|g.album.first()).filter_map(|a|id_from_gid(a.gid())).filter(|id|seen.insert(id.clone())).map(|id|format!("spotify:album:{id}")).collect::<Vec<_>>()); Some(row)
        }, _ => None
    }
}
fn context_path(uri: &str) -> Option<String> {
    let path = if let Some(p) = uri.strip_prefix("hm://") {format!("/{p}")} else if uri.starts_with('/') {uri.to_owned()} else {
        let u = reqwest::Url::parse(uri).ok()?;
        if u.scheme() != "https" || !u.host_str()?.ends_with(".spotify.com") || !u.username().is_empty() || u.password().is_some() || u.port().is_some_and(|p|p!=443) {return None;}
        format!("{}{}",u.path(),u.query().map(|q|format!("?{q}")).unwrap_or_default())
    };
    (path.starts_with("/context-resolve/v1/") && !path.contains("..") && !path.contains(['\\','\r','\n','#'])).then_some(path)
}
fn append_context(state: &mut Value, v: &Value) -> Result<(), String> {
    let pages: Vec<&Value> = if let Some(pages) = v["pages"].as_array() { pages.iter().collect() } else if v["tracks"].is_array() { vec![v] } else { return Err("native_context_invalid".into()); };
    for page in pages {
        let tracks = page["tracks"].as_array();
        if let Some(tracks) = tracks {
            for t in tracks {
                if t["metadata"]["is_local"] == "true" || t["metadata"]["is_playable"] == "false" {continue;}
                let uri = if let Some(uri) = t["uri"].as_str() {valid_uri(uri,"track")} else {
                    t["gid"].as_str().and_then(|s|base64::engine::general_purpose::STANDARD.decode(s).ok()).and_then(|gid|id_from_gid(&gid)).map(|id|format!("spotify:track:{id}"))
                };
                if let Some(uri) = uri {state["uris"].as_array_mut().ok_or("native_context_invalid")?.push(json!(uri));}
            }
        }
        for field in if tracks.is_some_and(|t|!t.is_empty()) {vec!["next_page_url","nextPageUrl"]} else {vec!["page_url","pageUrl","next_page_url","nextPageUrl"]} {
            if let Some(link) = page[field].as_str().filter(|s|!s.is_empty()) {
                let link = context_path(link).ok_or("native_invalid_path")?;
                if !state["links"].as_array().ok_or("native_context_invalid")?.iter().any(|v|v==&link) {state["links"].as_array_mut().unwrap().push(json!(link));}
            }
        }
    }
    if state["uris"].as_array().unwrap().len() > 1000 || state["links"].as_array().unwrap().len() > 32 {return Err("native_context_large".into());}
    Ok(())
}

#[cfg(test)]
mod tests;
