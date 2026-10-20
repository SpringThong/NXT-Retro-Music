# SoundCloud public adapter — v1.2

Native C frontend → JSON/control + bounded PCM sockets → Rust HLS decoder → bundled yt-dlp + NXT extractor plugin. No personal client ID/secret, account password, OAuth token or cookies are read. The old credentials pathname is retained only to locate the per-device bookmarks/cache directory.

The plugin reuses yt-dlp SoundcloudBaseIE: public website client discovery, guest web requests, metadata and transcoding URLs. This is unofficial extraction, not the official developer API. The plugin handles tracks/playlists/search/resolve in 20-row pages and validates public status before playback. Private, blocked, preview and encrypted formats are rejected. Local bookmarks retain URNs and original permalinks.

Rust launches yt-dlp with argv (no shell), ignores external configuration, limits stdout/stderr and execution time, cancels process groups, and closes the PCM descriptor in children. It does not log signed URLs or extractor stderr. The plugin is loaded only from the Pak. Playback URLs are refreshed for each play/seek; HLS data stays bounded in RAM. AAC decoding falls back to MP3 before PCM starts.

The pinned official ARM64 onedir runtime includes Python. It avoids writing an entire interpreter to /tmp for every request. `runtime.json` records URL/SHA256. Preserve all `_internal` files and bundled third-party notices. Run `python3 tools/setup-soundcloud-runtime.py` to restore the exact runtime. Updates should change this pin, review extractor/plugin compatibility, then repeat QA; no automatic self-update during playback.

Tests:

```bash
cargo test --locked --manifest-path soundcloud/backend/Cargo.toml
python3 qa/test_soundcloud_public_plugin.py
python3 qa/test_soundcloud_ipc.py
python3 qa/test_soundcloud_pcm.py
python3 qa/test_soundcloud_background.py
python3 qa/test_render_soundcloud.py
```

`qa/test_soundcloud_live.py <host helper>` is opt-in and requires a host yt-dlp executable set with NXT_SOUNDCLOUD_YTDLP and an absolute NXT_SOUNDCLOUD_PLUGINS directory. It tests live search, decoding, seeking, playlist browsing and resolve; it stores no audio or signed media URLs. For a development HTTPS proxy only, NXT_SOUNDCLOUD_CA adds a PEM trust root while retaining TLS verification. Release binaries accept no localhost media fixture override; that is compiled only by cargo test.

Hardware playback, public catalog availability, network speed and all AAC/HLS variants are not guaranteed by host tests. Favorites are local; no account-library synchronization, offline downloads, private content or Go+ grant is included.
