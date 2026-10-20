# NXT Retro Music v1.8 · Release verification

The two installers are Linux ARM64 builds for TrimUI Brick Pro, with distinct Stock and NextUI launchers, system integration and OTA targets. The unified source retains the supplied v1.7.1 baseline and third-party source/license provenance. No physical Brick Pro was connected during these checks.

| Area | Checks completed | Evidence |
|---|---|---|
| Native interface | 36 themes at 1024×768 and 640×480; home/player, live lyric updates, title and avatar bounds; actual production SDL modules under ASan/UBSan | `logs/v18-ui-test.json`, `logs/v18-ui-nextui-test.json`, `qa/screens-v1.8/` |
| Profiles | Saved per-profile avatar in startup chooser; profile selection followed by mood, genre, country and Personal/For you; long labels, emoji, photo and text avatars | UI controller fixture and native frames |
| Assets | 300 icon entries, 180 decoded Fluent 3D emoji; legacy first 64 identities preserved; removed pixel avatar choice migrated | UI and package reports, resource catalogs |
| Keyboard | Actual production input/render loop: six Vietnamese Telex phrases, matching defaults for all 14 languages, Shift/Caps, Latin security fields, Hangul/Kana/Kanji/Pinyin | `logs/v18-keyboard-test.json`, `logs/v18-ime-test.json` |
| Wi-Fi transfer | Actual POSIX server: PIN auth, UTF-8 validation, header isolation, 16 MiB streaming/hash, exclusive no-overwrite installation, cancel/stop cleanup, symlink/traversal rejection | `logs/v18-transfer-test.json` |
| USB handoff | Production shell syntax and controlled firmware/mount fixture; normal unmount before storage mode, abort on busy SD, owned awake flag, original mount options/USB role restored. Endpoints and flow compared with official firmware files | `logs/v18-usb-test.json` |
| LED and output | Stock and NextUI production drivers with controlled sysfs/ALSA fixtures: live color/mode without disabling, groups/effects, retry, restore, DAC/BT/speaker routing and volume | `logs/v18-led-driver-test.json` |
| Visual work | Actual frame-budget function: paused 1 Hz, normal 10 Hz, mechanical 20 Hz, e-ink 4 Hz; spectrum limited; hidden/off visual work skipped, input polling retained, timer wrap/wake handled | `logs/nextui-frame-budget-test.json` |
| ARM64 loaders | Both final release ELFs show 1.8 and the correct ROM; help and backend ready/quit via QEMU and pinned SDK libc | `logs/v18-arm64-stock-test.json`, `logs/v18-arm64-nextui-test.json` |
| Official Stock rootfs | Actual packaged frontend, SDL drivers/resources, backend startup and bundled yt-dlp checked against the extracted official Brick Pro 1.1.1-20260717 rootfs | `logs/v18-stock-rootfs-test.json` |
| OTA | Production worker/verifier/installer uses each actual 1.8 ZIP with a 1.7.1 installed fixture: check, Drive confirmation, SHA/size verification, install, old app backup, data retained, wrong-ROM/HTML rejection, cancellation and child reaping | `logs/v18-ota-qa-stock.json`, `logs/v18-ota-qa-nextui.json` |
| Packages | Both ROM configs/ELFs, all new resources, USB helper, executable modes, correct per-ROM OTA source and complete ZIP CRC; credentials/user data excluded | `logs/v18-packages-test.json` |

Host timing numbers are diagnostics for the host renderer only. They are not device FPS, CPU load, battery life or temperature measurements. ASan leak detection is disabled because the execution namespace cannot support LeakSanitizer; these checks do not establish absence of every memory leak.

To validate on a real device: play local and online music, change themes while playing, confirm X/Y/SELECT/START and remapped button hints, use long titles/own photos, change LED live, stream/cancel a Wi-Fi file, eject USB storage cleanly, and observe device performance/temperature over a normal listening session. Physical USB transport, live account playback, audio quality and thermals remain unmeasured in this release environment.

English and Vietnamese are authored directly. The primary labels of the other 12 languages have reviewed overrides; secondary descriptions use offline catalogs prepared with pinned build-time translation. Native fonts and input are exercised in all 14 languages, but every secondary translation has not been reviewed by a native speaker. No translation model or online IME is shipped on the device.

OTA tools include exact installer size and SHA-256. Their download URL is a clearly marked publisher placeholder. No release was published to Google Drive automatically; replace the URL with the public installer file link before publishing the manifest. Manual installation works without publishing OTA.
