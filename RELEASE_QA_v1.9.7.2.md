# Release QA — v1.9.7.2

## Root cause and scope

Nxt_renderPlaying registered @now-playing, then Original NxtSkin_begin registered its visible title as another page. Each repaint therefore looked like a page change and restarted the horizontal entrance. The patch gives each playback page one identity. This is a verified software animation bug; camera videos alone do not establish physical panel scan-out tearing.

The native rendering regression fails against the exact v1.9.7.1 UI/skin files and passes against the patch at 640×480 and 1024×768. See logs/v1972-page-baseline-regression.json and logs/v1972-ui-test.json.

## Screen-off controls

Device-wide persisted option, default off. Enabled: normal mappings, seek, volume and SELECT lock/unlock remain available without waking the backlight. Power toggles the screen; wake/Power gestures remain consumed through release. Automatic timeout still blanks the display, without invoking suspend, audio pause or mute. Stock quick panel is suppressed while this dark-control mode is active and the screen is off. Existing logo exit behavior remains. Now Playing surface redraws are skipped while the screen is off; audio and queue maintenance continue.

## Verification

- Production power functions, all key classes, Power/release guards, automatic timeout, cleanup, mapped playback/seek/volume and real SELECT lock handling, ASan/UBSan, all three ROM definitions.
- Real config save/reload and per-listener isolation, default disabled, old settings migration; all three ROM definitions.
- Native renderer, 51 themes, 60 fonts, bilingual lyrics, preview Apply/Cancel/save failure, both display sizes, ASan/UBSan.
- Existing page motion/lyric logic, fine volume/ROM/HID inputs.
- DSD parser/packing: 576 independent cases; production DSD/PCM/online policy.
- All three ARM64 release frontends rebuilt with GCC 8.3.0 / O2 and tested with the pinned SDK under QEMU (--build-info, --help).
- 831 source inputs compared to verified v1.9.7.1 baseline: only the 11 reviewed UI/input/power files changed. All 746 bundled resource files and both online backend executables remain byte-identical.
- Installer and OTA publisher kits: CRC, SHA/size, matching binary, correct ROM paths, official feed, current version, executable bits and no userdata/keys.
- Production OTA worker/stager/installer against complete final installers from an installed v1.9.7.1 fixture, using local network responses: migration to official feed, custom source preservation, download confirmation, invalid SHA/size/ROM/HTML rejection, staged install and userdata preservation, cancellation/reaping.

Detailed per-ROM hashes and current evidence are in BUILD_REPORT_v1.9.7.2_*.json.

## Limits

No physical TrimUI ROM/DAC/Bluetooth boot test, measured panel tearing/FPS, thermal or battery data. Local OTA responses replace live network service in the test. No live Drive feed has been published. Singing/karaoke timing algorithms and audio backends are unchanged.
