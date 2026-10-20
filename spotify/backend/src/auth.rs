use crate::{api::Api, diagnostic, emit};
use librespot_core::{authentication::Credentials, config::SessionConfig, session::Session};
use serde_json::json;

pub struct Verified {
    pub session: Session,
    pub credentials: Api,
    pub username: String,
}

#[derive(Clone, Copy)]
enum LoginSource { Auto, Pc, Qr }
impl LoginSource {
    fn parse(source: Option<&str>) -> Result<Self, String> {
        match source.unwrap_or("auto") {
            "auto" => Ok(Self::Auto), "pc" => Ok(Self::Pc), "qr" => Ok(Self::Qr),
            _ => Err("invalid_login_source".into()),
        }
    }
}
fn select_credentials(source: LoginSource, paired: Option<Api>, pc: Option<Api>) -> Result<Api, String> {
    match source {
        LoginSource::Auto => paired.or(pc).ok_or("pair_required".into()),
        LoginSource::Pc => pc.ok_or("pc_credentials_missing".into()),
        LoginSource::Qr => paired.ok_or("pair_required".into()),
    }
}
pub async fn connect(source: Option<&str>) -> Result<Verified, String> {
    let source = LoginSource::parse(source)?;
    let paired_path = std::env::var("NXT_SPOTIFY_PLAYBACK_CREDS").ok();
    let pc_path = std::env::var("NXT_SPOTIFY_CREDENTIALS").ok();
    // The user's most recent explicit import/pair action chooses playback.
    // Never switch identities or client IDs after a server rejection.
    let paired = paired_path.as_deref().and_then(|p| Api::playback(p).ok());
    let pc = pc_path.as_deref().and_then(|p| Api::open(p).ok());
    let mut credentials = select_credentials(source, paired, pc)?;
    diagnostic(match source {LoginSource::Auto=>"Login source: Auto", LoginSource::Pc=>"Login source: explicit PC import", LoginSource::Qr=>"Login source: approved QR"});
    emit(json!({"event":"login_progress","stage":"token"}));
    let token = credentials.token().await.map_err(|e| format!("playback_{e}"))?;
    let cfg = SessionConfig { client_id: credentials.client_id().to_owned(), ..Default::default() };
    let session = Session::new(cfg, None);
    emit(json!({"event":"login_progress","stage":"ap"}));
    match tokio::time::timeout(std::time::Duration::from_secs(25), session.connect(Credentials::with_access_token(&token), false)).await {
        Ok(Ok(_)) => diagnostic("Login: AP authentication accepted"),
        Ok(Err(e)) => { diagnostic(&format!("Login: AP rejected ({:?})",e.kind)); session.shutdown(); return Err("authentication_failed".into()); }
        Err(_) => { session.shutdown(); return Err("playback_network_timeout".into()); }
    }
    emit(json!({"event":"login_progress","stage":"login5"}));
    match tokio::time::timeout(std::time::Duration::from_secs(20), session.login5().auth_token()).await {
        Ok(Ok(_)) => diagnostic("Login: Login5 accepted; playback authenticated"),
        Ok(Err(e)) => { diagnostic(&format!("Login: Login5 rejected ({:?})",e.kind)); session.shutdown(); return Err("playback_auth_rejected".into()); }
        Err(_) => { session.shutdown(); return Err("playback_auth_timeout".into()); }
    }
    let username = session.username();
    if username.is_empty() { session.shutdown(); return Err("authentication_failed".into()); }
    Ok(Verified { session, credentials, username })
}

#[cfg(test)] mod tests {
    use super::*;
    fn fixture(path: &std::path::Path, id: &str) -> Api {
        std::fs::write(path, json!({"client_id":id,"access_token":"FAKE-DO-NOT-USE","refresh_token":"FAKE-DO-NOT-USE"}).to_string()).unwrap();
        Api::open(path.to_str().unwrap()).unwrap()
    }
    #[test] fn explicit_pc_import_is_not_shadowed_by_saved_qr() {
        let dir=std::env::temp_dir().join(format!("nxt-auth-source-{}",std::process::id()));std::fs::create_dir_all(&dir).unwrap();
        let qr=fixture(&dir.join("qr.json"),crate::api::PLAYBACK_CLIENT_ID);
        let pc=fixture(&dir.join("pc.json"),crate::api::CLIENT_ID);
        assert!(select_credentials(LoginSource::Pc,Some(qr.clone()),Some(pc.clone())).unwrap().same_file(&pc));
        assert!(select_credentials(LoginSource::Qr,Some(qr.clone()),Some(pc.clone())).unwrap().same_file(&qr));
        assert!(select_credentials(LoginSource::Auto,Some(qr.clone()),Some(pc.clone())).unwrap().same_file(&qr));
        assert!(select_credentials(LoginSource::Auto,None,Some(pc.clone())).unwrap().same_file(&pc));
        assert!(select_credentials(LoginSource::Pc,Some(qr.clone()),None).is_err());
        assert!(select_credentials(LoginSource::Qr,None,Some(pc)).is_err());
        std::fs::remove_dir_all(dir).unwrap();
    }
    #[test] fn login_source_accepts_only_explicit_known_modes() {
        assert!(LoginSource::parse(None).is_ok());assert!(LoginSource::parse(Some("pc")).is_ok());
        assert!(LoginSource::parse(Some("rotate")).is_err());
    }
}
