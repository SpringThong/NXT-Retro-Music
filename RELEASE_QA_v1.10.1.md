# NXT Retro Music v1.10.1 — validation / kiểm tra

## Changes and intended behavior

Back in the shared local/Drive playlist player, local resume, Spotify/SoundCloud Now Playing and the startup saved-song card returns to the main menu without changing play/pause state. Only root-menu Back invokes the existing exit policy: while playing, choose background playback or stop and fully close; paused/stopped closes immediately. This uses orderly full-process cleanup rather than SIGKILL, preserving saved state. Nested settings/library Back remains unchanged.

Spotify QR catalog token acquisition now uses the same verified account/client Login5 session as playback, including its token cache and refresh. It no longer calls Mercury keymaster to request another scoped token. This removes the known source of the reported `session_token_denied` error before library HTTP requests. Genuine HTTP 403 / insufficient-scope responses remain failures and are shown distinctly. PC JSON login remains available; no identity/client switching, quota or region bypass is added.

## Actual checks passed

- All three fresh ARM64 frontends and the fresh ARM64 Spotify helper compiled in run https://github.com/SpringThong/NXT-Retro-Music/actions/runs/40090328786, commit 2e213a737cb39b05d4670b744b4aa4c36743ad0c.
- Each frontend ran real --build-info and --help commands under QEMU with the pinned SDK; exit codes, v1.10.1 and the correct ROM labels were asserted.
- Production Back/exit functions compiled with deterministic audio/UI stubs: startup/paused/playing player Back returns home with no dialog, pause or quit; root paused/stopped closes; root playing asks background/close; saved background preference unchanged. Static route guard confirms only the root menu calls the exit prompt.
- 23 Rust tests passed, including empty Login5 session failure without a keymaster/PC fallback and genuine scope-denial classification. Existing rate-limit, account-selection, metadata, pairing, lyrics and PCM tests remain passing.
- Online PCM queue fixture passed. Its underlying queue, ramps and player-core code are byte-identical to v1.10.
- Production OTA numeric-version test confirms 1.10.1 is newer than 1.10 and 1.9.x.
- All six release ZIPs passed complete CRC, executable permissions, clean user state, matching fresh binaries/source fingerprints, correct ROM paths and exact installer hashes inside each matching OTA publisher kit.
- Preservation vs v1.10: 896 source files compared, 746 image/font/resource files unchanged; DSD/DSP, translation/cache, CPU/LED, library/OTA and online buffering code unchanged. SoundCloud/yt-dlp helper runtimes and all three official OTA feed URLs preserved. This comparison does not replace physical-device regression testing.

## Limits and device test

No physical TrimUI, Spotify account or live library/streaming session was available. Successful compilation and fixtures do not establish actual Spotify library access for a real account. The original token-acquisition error is addressed, but the service can still refuse permissions. FPS, heat and battery use were not measured. The pre-existing application-information page and historical change entries are preserved; current v1.10.1 behavior is documented in the bundled release guide and changelog.

Test on the actual ROM: open app → Now Playing → B reaches main menu. With paused/stopped audio, main-menu B closes. With music playing, main-menu B offers background or close; check both choices and reopen background playback. Reconnect Spotify using QR without importing PC JSON first, then test display name, saved tracks, playlists, albums, artist search and playback. If denied, share NXT_Spotify_Log.txt with tokens/credentials excluded.

## OTA

Installer and OTA kit files are prepared; the live feeds have not been published/replaced. Keep the same three official Google Drive JSON files. Upload the matching new installer, generate update.json with the real ZIP URL using that ROM's CHUAN_BI_OTA.html and upload a new version onto the existing JSON file. Do not mix ROM packages.

## Tiếng Việt

Đã build mới đủ ba ROM và bộ phát Spotify, kiểm tra phần mềm/ZIP như trên. B ở Đang phát về menu chính; chỉ B ở menu chính mới quyết định thoát/nghe nền, còn pause/dừng thì thoát hẳn. QR dùng lại token Login5 của cùng phiên đã xác thực; giữ nhập JSON PC dự phòng và các tính năng v1.10. Chưa thử máy hoặc tài khoản Spotify thật, chưa đo nhiệt độ/pin/FPS và chưa đăng feed OTA đang hoạt động.
