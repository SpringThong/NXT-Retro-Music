use super::*;
use protobuf::{MessageField, well_known_types::any::Any};
use std::sync::{Arc, Mutex};

#[derive(Clone)]
struct Mock { steps: Arc<Mutex<VecDeque<(Method, String, Result<Vec<u8>, String>)>>>, calls: Arc<Mutex<Vec<(Method, String, Option<Vec<u8>>, String)>>> }
impl Mock {
    fn new(steps: Vec<(Method, String, Result<Vec<u8>, String>)>) -> Self { Self { steps: Arc::new(Mutex::new(steps.into())), calls: Default::default() } }
    fn done(&self) { assert!(self.steps.lock().unwrap().is_empty(), "not all expected native requests were made"); }
    fn count(&self) -> usize { self.calls.lock().unwrap().len() }
}
impl Service for Mock {
    async fn request(&self, method: Method, path: &str, body: Option<Vec<u8>>, content_type: &str) -> Result<Vec<u8>, String> {
        let step = self.steps.lock().unwrap().pop_front().expect("unexpected request; no fallback to developer Web API allowed");
        assert_eq!(method, step.0); assert_eq!(path, step.1);
        self.calls.lock().unwrap().push((method, path.into(), body, content_type.into())); step.2
    }
}
fn step(method: Method, path: impl Into<String>, body: impl AsRef<[u8]>) -> (Method, String, Result<Vec<u8>, String>) { (method, path.into(), Ok(body.as_ref().to_vec())) }
fn id(n: u8) -> String { id_from_gid(&[n;16]).unwrap() }
fn uri(kind: &str, n: u8) -> String { format!("spotify:{kind}:{}", id(n)) }
fn artist(n: u8, name: &str) -> meta::Artist { meta::Artist { gid: Some(vec![n;16]), name: Some(name.into()), ..Default::default() } }
fn album(n: u8) -> meta::Album { meta::Album { gid: Some(vec![n;16]), name: Some("Album 中文 ไทย".into()), artist: vec![artist(200,"Spring Thông")], ..Default::default() } }
fn track(n: u8) -> meta::Track { meta::Track { gid: Some(vec![n;16]), name: Some(format!("Bài {n} 日本語 한국어")), duration: Some(180000), artist: vec![artist(200,"Spring Thông")], album: MessageField::some(album(201)), ..Default::default() } }
fn metadata_response(kind: ExtensionKind, entities: Vec<(String, Vec<u8>)>) -> Vec<u8> {
    ext::BatchedExtensionResponse { extended_metadata: vec![ext::EntityExtensionDataArray { extension_kind: EnumOrUnknown::new(kind), extension_data: entities.into_iter().map(|(entity_uri,bytes)| librespot_protocol::entity_extension_data::EntityExtensionData { entity_uri, header: MessageField::some(librespot_protocol::entity_extension_data::EntityExtensionDataHeader { status_code:200,..Default::default() }), extension_data: MessageField::some(Any { value:bytes,..Default::default() }), ..Default::default() }).collect(), ..Default::default() }], ..Default::default() }.write_to_bytes().unwrap()
}
fn tracks_response(ns: &[u8]) -> Vec<u8> { metadata_response(ExtensionKind::TRACK_V4, ns.iter().rev().map(|&n|(uri("track",n),track(n).write_to_bytes().unwrap())).collect()) }
fn root_path(offset: u64) -> String { format!("/playlist/v2/user/listener/rootlist?decorate=revision,attributes,length,owner,capabilities,status_code&from={offset}&length={PAGE}") }
fn list_bytes(uris: &[String], names: &[&str], offset: i32, truncated: bool, length: i32) -> Vec<u8> {
    playlist::SelectedListContent { length:Some(length), contents: MessageField::some(playlist::ListItems { pos:Some(offset), truncated:Some(truncated), items:uris.iter().map(|s|playlist::Item { uri:Some(s.clone()),..Default::default() }).collect(), meta_items:names.iter().map(|name|playlist::MetaItem { attributes:MessageField::some(playlist::ListAttributes { name:Some((*name).into()),..Default::default() }),owner_username:Some("listener".into()),..Default::default() }).collect(),..Default::default() }),..Default::default() }.write_to_bytes().unwrap()
}
fn collected(items: Vec<(String,bool)>, token: &str) -> Vec<u8> {
    collection::PageResponse { items:items.into_iter().map(|(uri,is_removed)|collection::CollectionItem { uri,is_removed,added_at:123,..Default::default() }).collect(),next_page_token:token.into(),..Default::default() }.write_to_bytes().unwrap()
}

#[tokio::test]
async fn decorated_rootlist_is_one_request_skips_folders_and_warm_cache_rebinds_request_id() {
    let data=list_bytes(&["spotify:start-group:folder".into(),uri("playlist",1),"spotify:end-group:folder".into()], &["Folder","Nhạc Việt 中文",""],0,false,3);
    let mock=Mock::new(vec![step(Method::GET,root_path(0),data)]); let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());
    n.probe().await.unwrap();let a=n.query(&json!({"cmd":"playlists","request_id":77})).await;
    assert_eq!(a["event"],"results");assert_eq!(a["request_id"],77);assert_eq!(a["items"].as_array().unwrap().len(),1);assert_eq!(a["items"][0]["title"],"Nhạc Việt 中文");assert_eq!(a["next_offset"],3);assert_eq!(a["has_next"],false);
    let b=n.query(&json!({"cmd":"playlists","request_id":78})).await;assert_eq!(b["request_id"],78);assert_eq!(mock.count(),1);mock.done();
}
#[tokio::test]
async fn rootlist_uses_raw_positions_and_does_not_eagerly_fetch_playlist_contents() {
    let mock=Mock::new(vec![step(Method::GET,root_path(0),list_bytes(&[uri("playlist",1),"spotify:end-group:folder".into()],&["One",""],0,true,3)),step(Method::GET,root_path(2),list_bytes(&[uri("playlist",2)],&["Two"],2,false,3))]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());
    let a=n.query(&json!({"cmd":"playlists"})).await;assert_eq!(a["next_offset"],2);assert_eq!(a["has_next"],true);
    let b=n.query(&json!({"cmd":"playlists","offset":2})).await;assert_eq!(b["items"][0]["title"],"Two");assert_eq!(mock.count(),2);mock.done();
}
#[tokio::test]
async fn collection_filters_removals_and_other_types_batches_metadata_and_keeps_provider_order() {
    let mock=Mock::new(vec![step(Method::POST,"/collection/v2/paging",collected(vec![(uri("track",1),true),(uri("album",2),false),(uri("track",3),false)],"next")),step(Method::POST,"/collection/v2/paging",collected(vec![(uri("track",4),false)],"")),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&[3,4]))]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let v=n.query(&json!({"cmd":"saved_tracks","request_id":9})).await;
    assert_eq!(v["event"],"results");assert_eq!(v["items"].as_array().unwrap().len(),2);assert_eq!(v["items"][0]["uri"],uri("track",3));assert_eq!(v["items"][1]["uri"],uri("track",4));assert_eq!(v["items"][0]["artist"],"Spring Thông");assert_eq!(v["has_next"],false);assert!(v["items"][0]["_artists"].is_null());
    let calls=mock.calls.lock().unwrap();let first=collection::PageRequest::parse_from_bytes(calls[0].2.as_ref().unwrap()).unwrap();let second=collection::PageRequest::parse_from_bytes(calls[1].2.as_ref().unwrap()).unwrap();assert_eq!(first.username,"listener");assert_eq!(first.limit,100);assert_eq!(second.pagination_token,"next");assert_eq!(calls[0].3,COLLECTION);drop(calls);mock.done();
}
#[tokio::test]
async fn saved_album_is_a_saved_album_not_an_album_inferred_from_liked_tracks() {
    let data=metadata_response(ExtensionKind::ALBUM_V4,vec![(uri("album",2),album(2).write_to_bytes().unwrap())]);
    let mock=Mock::new(vec![step(Method::POST,"/collection/v2/paging",collected(vec![(uri("track",1),false),(uri("album",2),false)],"")),step(Method::POST,"/extended-metadata/v0/extended-metadata",data)]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let v=n.query(&json!({"cmd":"saved_albums"})).await;
    assert_eq!(v["items"].as_array().unwrap().len(),1);assert_eq!(v["items"][0]["id"],id(2));assert_eq!(v["items"][0]["title"],"Album 中文 ไทย");mock.done();
}
#[tokio::test]
async fn collection_cursor_preserves_surplus_matches_without_loading_the_whole_library() {
    let items=(1..=25).map(|n|(uri("track",n),false)).collect();
    let mock=Mock::new(vec![step(Method::POST,"/collection/v2/paging",collected(items,"")),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&(1..=20).collect::<Vec<_>>())),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&(21..=25).collect::<Vec<_>>()))]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let a=n.query(&json!({"cmd":"saved_tracks"})).await;assert_eq!(a["items"].as_array().unwrap().len(),20);assert_eq!(a["has_next"],true);
    let b=n.query(&json!({"cmd":"saved_tracks","offset":20})).await;assert_eq!(b["items"].as_array().unwrap().len(),5);assert_eq!(b["items"][0]["uri"],uri("track",21));assert_eq!(b["has_next"],false);assert_eq!(mock.count(),3);mock.done();
}
#[tokio::test]
async fn contains_and_favorite_use_authenticated_collection_messages_and_invalidate_saved_pages() {
    let mock=Mock::new(vec![step(Method::POST,"/collection/v2/contains",collection::ContainsResponse { found:vec![true],..Default::default() }.write_to_bytes().unwrap()),step(Method::POST,"/collection/v2/write",[])]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());assert!(n.saved(&uri("track",1)).await.unwrap());
    n.pages.put("old".into(),json!(1));n.cursors.put("old".into(),json!(1));n.favorite(&uri("track",1),false).await.unwrap();assert!(n.pages.get("old").is_none());assert!(n.cursors.get("old").is_none());
    let calls=mock.calls.lock().unwrap();let write=collection::WriteRequest::parse_from_bytes(calls[1].2.as_ref().unwrap()).unwrap();assert_eq!(write.username,"listener");assert_eq!(write.set,"collection");assert!(write.items[0].is_removed);assert_eq!(write.items[0].added_at,0);drop(calls);mock.done();
}
#[tokio::test]
async fn native_permission_or_cooldown_error_is_not_reported_as_an_empty_success_or_switched_to_web_api() {
    for reason in ["native_http_403","rate_limited:86400"] {
        let mock=Mock::new(vec![(Method::GET,root_path(0),Err(reason.into()))]);let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let v=n.query(&json!({"cmd":"playlists","request_id":14})).await;
        assert_eq!(v["event"],"error");assert_eq!(v["reason"],reason);assert_eq!(v["request_id"],14);assert_eq!(mock.count(),1);mock.done();
    }
}
#[tokio::test]
async fn search_decodes_gid_and_derives_deduplicated_artists_from_matching_tracks() {
    let gid=base64::engine::general_purpose::STANDARD.encode([1u8;16]);
    let context=json!({"unknown_future_field":true,"pages":[{"tracks":[{"gid":gid},{"uri":uri("track",2)},{"uri":"spotify:local:not-a-track"}]}]});
    let mock=Mock::new(vec![step(Method::GET,"/context-resolve/v1/spotify:search:Spring+Th%C3%B4ng",context.to_string()),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&[1,2]))]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let v=n.query(&json!({"cmd":"search_artists","query":"Spring Thông"})).await;
    assert_eq!(v["event"],"results");assert_eq!(v["items"].as_array().unwrap().len(),1);assert_eq!(v["items"][0]["title"],"Spring Thông");assert_eq!(v["items"][0]["id"],id(200));mock.done();
}
#[tokio::test]
async fn search_follows_only_same_service_context_pages_and_preserves_page_cursor() {
    let context=json!({"pages":[{"tracks":[{"uri":uri("track",1)}],"next_page_url":"hm://context-resolve/v1/page/two"}]});
    let more=json!({"tracks":[{"uri":uri("track",2)}]});
    let mock=Mock::new(vec![step(Method::GET,"/context-resolve/v1/spotify:search:test",context.to_string()),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&[1])),step(Method::GET,"/context-resolve/v1/page/two",more.to_string()),step(Method::POST,"/extended-metadata/v0/extended-metadata",tracks_response(&[2]))]);
    let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());let a=n.query(&json!({"cmd":"search","query":"test"})).await;assert_eq!(a["has_next"],true);
    let b=n.query(&json!({"cmd":"search","query":"test","offset":20})).await;assert_eq!(b["items"][0]["uri"],uri("track",2));assert_eq!(b["has_next"],false);mock.done();
}
#[test]
fn context_url_guard_rejects_foreign_hosts_wrong_services_and_loops_have_bounded_state() {
    for bad in ["https://example.org/context-resolve/v1/page","https://api.spotify.com/v1/me","hm://collection/v2/write","/context-resolve/v1/../me","https://user:pass@spclient.wg.spotify.com/context-resolve/v1/page"] {assert!(context_path(bad).is_none(),"{bad}");}
    let mut state=json!({"uris":[],"links":[]});assert!(append_context(&mut state,&json!({"pages":[{"page_url":"https://example.org/secret"}]})).is_err());
}
#[test]
fn cache_is_bounded_by_entry_count_bytes_and_expiry() {
    let mut c=Cache::new(2,100,2);c.put("a".into(),json!("one"));c.put("b".into(),json!("two"));c.put("c".into(),json!("three"));assert!(c.get("a").is_none());assert!(c.bytes<=100);assert!(c.entries.len()<=2);
    c.entries[0].at=Instant::now()-Duration::from_secs(3);let k=c.entries[0].key.clone();assert!(c.get(&k).is_none());c.put("big".into(),json!("x".repeat(1000)));assert!(c.get("big").is_none());
}
#[tokio::test]
async fn empty_or_invalid_input_and_expired_page_do_not_make_network_requests() {
    let mock=Mock::new(vec![]);let mut n=NativeCatalog::with_service(mock.clone(),"listener".into());
    for req in [json!({"cmd":"search","query":"  "}),json!({"cmd":"playlist_tracks","id":"../bad"}),json!({"cmd":"saved_tracks","offset":20})] {assert_eq!(n.query(&req).await["event"],"error");}
    assert!(n.favorite("spotify:local:bad",true).await.is_err());assert_eq!(mock.count(),0);
}
