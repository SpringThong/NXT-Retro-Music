# NXT Retro Music v1.10 — verification / kiểm tra

## Delivered builds

Stock, NextUI and NX Redux have separate freshly compiled ARM64 frontends and the freshly compiled Spotify helper (Cargo package 1.10.0). The installer and OTA kit of each ROM contain identical installer bytes. Official OTA feed URLs are unchanged.

- Frontend build and strict QEMU loader checks: https://github.com/SpringThong/NXT-Retro-Music/actions/runs/40086941227
- Spotify helper build and 21 passing Rust tests: https://github.com/SpringThong/NXT-Retro-Music/actions/runs/40086156392
- Per-ROM evidence: BUILD_REPORT_v1.10_Stock.json, BUILD_REPORT_v1.10_NextUI.json, BUILD_REPORT_v1.10_NXRedux.json and logs/v110-*.

## Passed checks

- ARM64 executable identity; real --build-info and --help commands under QEMU, exit status zero, version and ROM label asserted.
- B exit policy compiled from the production function with audio/UI stubs: paused/stopped closes; playing prompts; background choice overrides the saved background preference for this exit only; stop and B close; X returns to the main menu.
- Bounded PCM queue: startup/refill threshold, stale-track epochs, EOF tail, short SoundCloud tail, queue reset and close; 5 ms anti-click ramps. ASan and UBSan passed. LeakSanitizer was disabled because of restricted host /proc access.
- Production OTA version comparison: 1.10 is newer than 1.9.7.7 and equals 1.10.0.
- All six ZIPs: full CRC, executable modes, current build fingerprints, correct ROM paths/settings libraries, clean state, no user credentials, current version and exact installer digest in each OTA kit.
- Preservation against v1.9.7.7: 746 resource files, fonts/artwork/themes, unchanged DSD/bit-perfect, translation, library indexing, CPU/LED and OTA source code; existing SoundCloud/yt-dlp runtime retained. This is a byte comparison, not a device regression test.

## Required physical/account testing

No physical TrimUI, Spotify Premium account, real QR authorization, live artist search or online playback session was available for this build. Successful compilation and simulated checks do not establish that Spotify grants catalog scopes to every account or that every service/network stutter is resolved. Existing region, subscription and service restrictions still apply. No FPS, temperature or battery claim was measured.

Try QR login without importing a PC JSON first; check account name, library, artist search and playback. On each ROM, compare Spotify/SoundCloud/Drive playback under the same Wi-Fi and test next/seek/pause/resume, B-background, B-stop and paused-B-exit. Existing user credentials/settings are not bundled and are not intentionally deleted by the installer.

## OTA publishing / đăng OTA

The three official feed links remain embedded. These files are offline publisher kits; the live Google Drive manifests have not been replaced. Upload the matching new installer, use that ROM's CHUAN_BI_OTA.html to generate update.json with its real download link, then upload a new version onto the existing official update.json file to preserve its Drive ID. Do not replace a feed with a ZIP or mix ROMs.

## Tóm tắt tiếng Việt

Đã build mới đủ ba ROM và kiểm tra phần mềm như trên. Chưa thử tài khoản Spotify thật hoặc máy thật; cần kiểm tra QR không dùng JSON PC, tìm nghệ sĩ, nghe online và các nhánh nút B. B khi đang phát hỏi nghe nền hoặc đóng app; X trong hộp thoại về menu. Khi pause/dừng, B ở trình phát hoặc menu gốc đóng app. Các màn con vẫn dùng B để quay lại. Thoát hẳn là đóng có dọn dẹp tiến trình, không cưỡng bức SIGKILL. Chưa đưa v1.10 lên ba feed OTA đang hoạt động.
