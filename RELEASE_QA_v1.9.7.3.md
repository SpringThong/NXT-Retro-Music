# v1.9.7.3 release QA

## Scope

User video IMG_0466.mp4 shows remaining artifacts during Now Playing ↔ full lyrics/menu navigation; user confirms all menu pages. v1.9.7.2 fixed recurring Original redraw identity, but retained a whole-page entrance slide. The new patch removes framebuffer movement/snapshot for all page navigation. Lyrics, vinyl and spectrum keep their existing animation clocks.

Both full-surface and layer-only native presentation now use one compositor: reset the window target, viewport and clip; clear the window backbuffer; compose the six textures in the same order; present exactly once. The layer-only path retains the latest uploaded base. This avoids relying on previous window-backbuffer contents and keeps the base and GPU overlays in the same coordinates.

## Actual checks

- No-displacement contract at 1024×768 and 640×480, all 51 theme identifiers and eight page classes, ten repeated frames per page, clipped/translucent surfaces, screen off/hidden and timestamp wrap. Old v1.9.7.2 page-motion implementation fails this contract; current implementation passes.
- Exact production native compositor exercised on SDL software renderer under ASan/UBSan, with previous-page pixels, stale render target, clip and viewport deliberately introduced. Full and layer-only frames match expected pixels and present exactly once. This does not emulate physical GPU scan-out.
- Native production UI: 51 themes, 60 fonts, bilingual full lyrics and Now Playing, original album layout, preview Apply/Cancel/save-failure at both sizes, ASan/UBSan.
- Real page/lyric timing logic and retained Enhanced-LRC word timing.
- Real power and app input routing: Screen-off controls, Power wake/release guards, timeout, SELECT lock/unlock, mapped play/skip/seek/volume; all three ROM definitions, ASan/UBSan.
- Config migration/save/reload/profile isolation and fine volume/ROM/HID inputs on all three definitions.
- All three ARM64 frontends rebuilt with pinned GCC 8.3.0/O2; actual release --help and --build-info checked under QEMU with pinned SDK.
- Verified baseline source SHA-256 3884ae8be59f2e008e64664f178b4fdf7ece49ad3beb2fe26244754b7c715ad6. Compare all 831 build inputs: only nxt.h, nxt_screens.c, nxt_motion.c/h and common generic_video.c changed. All 746 resources and both online backend executables are byte-identical. Audio/DSD/network/OTA engines and dark controls are unchanged.
- All three complete installer/OTA kit archives: CRC, hashes, sizes, version, ROM isolation, official feeds and executable modes. Production OTA worker/stager/installer tested from an installed v1.9.7.2 fixture, with local service responses, preserving userdata; validates SHA/size/ROM/HTML rejection, Drive confirmation, cancellation and child reaping.

Per-ROM BUILD_REPORT_v1.9.7.3_*.json binds checks to final binaries and archives.

## Limits

No physical TrimUI/ROM/DAC/Bluetooth test or panel tearing/FPS/thermal/battery measurements. Video frames can also combine consecutive display states during camera exposure; software checks cannot establish the physical cause of every visible artifact. No live OTA feed was uploaded.
