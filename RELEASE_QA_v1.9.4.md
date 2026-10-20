# NXT Retro Music v1.9.4 — release verification

Shared source for Stock, NextUI and NX Redux, based on the delivered unified v1.9.3 source (SHA-256 83fd7cb0b44c13ff88a1dbe9566e890972a69a3c9b6cf49f3e6677cf4ab30f68).

## Changes

Auralis is appended as theme 51; the original 50 names/palettes/assets keep their indices. Full-cover aspect-fill album art has one prepared dark backdrop per image/generation/size/view. Original and translated cues stay paired during a bounded vertical transition; the current cue and upcoming cue have separate labels. Translated lines are italic in Now Playing and full lyrics, with separate regular/italic font caches. Long sentences fit one row without marquee. MiSans Semibold 4.009 from Xiaomi is included unmodified with its full license/credit and appended as font 17.

Physical, custom-mapped and HID volume controls use real 0..100 raw levels in 1% steps while retaining the ROM's 0..20 settings ABI and old config migration. Each sink keeps its own fine level. Saves are debounced; route restoration reapplies the saved level. DAC mixer precision still depends on its hardware and ROM. Native DSD does not gain a PCM/software-volume processing path. The overlapping app-drawn firmware hardware-hint layer is removed from Now Playing.

## Passed software checks

- Three fresh GCC 8.3.0 AArch64/O2 frontend builds from the same tree, with distinct ROM adapters and current input fingerprints. Actual release binaries run --build-info and --help under QEMU with the pinned ARM64 sysroot.
- Native production UI under ASan/UBSan at 1024x768 and 640x480: all 50 original players, theme gallery wrap/apply/cancel, new Auralis bilingual/full lyrics/long sentence/no-cover states, cue transition frames, original upright font isolation, actual translation worker with local HTTP/curl fixtures. Render PNGs are in qa/screens-v1.9.4. No AI mockup is used as the native validation image.
- Actual volume controller with ALSA/ROM fixtures on all three defines: 85 -> 86 -> 87 raw values, per-sink restore, mute/clamp, external shared-volume changes, debounced saves, cached hardware mixer selection, card change and existing PCM fallback policy. Actual global/custom/HID key routes reach the fine controller. Config tests preserve old audio/appearance/LED choices and support theme 51/font 17 without moving device volume into listener preferences.
- Production DSD/PCM worker and policy under ASan/UBSan for all three defines: 576 independent DSF/DFF parser/packing cases per target, eight byte-identical transports, partial writes/EAGAIN, pause/seek/EOF/removal, exact-rate rejection, explicit Native/DoP and online-to-Normal policy. No physical DAC is simulated as a successful live device test.
- Production library scheduler/views: cached rows before scan, one startup incremental check, no scan on re-entry, stable idle views, hidden pause, upload debounce and favorites. Persistent index regression: warm zero writes/probes, nested changes, partial copies, cancellation, missing card, rollback and bounded 2048-track fixture.
- Baseline preservation audit checks all 821 original frontend inputs, all 667 original resources, unchanged online backend bytes, original 50 names/palettes, and each final binary fingerprint. The 21 modified original frontend files are explicitly allow-listed in qa/test_v194_preservation.py; the new Auralis pair of files is added. Translation/cache, DSD transport, audio-mode policy, library/index, CPU, LED, NX integration and ROM settings ABI implementations are unchanged.
- Final installer/OTA kit checks for all three ROMs: ZIP CRC and executable permissions, target-specific paths/libraries, no private data or foreign feed, exact MiSans hash/license, all resources, config/version, actual binary bytes, exact installer hash/size in OTA metadata and offline publisher HTML.
- Production OTA worker/package installer under ASan/UBSan uses each final full installer with a v1.9.3 installed-version fixture: official source migration, explicit custom source retention, Drive confirmation, size/SHA and ROM/HTML rejection, staging/install, preserved ready state, previous-app backup, listener/config/resume/cache/library/credentials preservation and cancellation/reaping.

## Limits

These are build, renderer, byte-comparison and mocked hardware/service checks on a host. Physical ROM boot, volume key repeat from each firmware, live Bluetooth/DAC/DSD playback, account services, temperature and battery life still need on-device testing. Rendering uses the existing performance budgets; this release adds no work to the decoder/audio callback. It cannot guarantee a measured cooling or frame-rate improvement without the device.

The official OTA URLs are embedded and checked. No live Drive feed has been uploaded or replaced by this build. Publish the exact per-ROM installer, generate update.json with its real public ZIP URL, and replace the contents of the same existing official Drive JSON file while retaining its ID. Templates containing REPLACE_WITH_PUBLIC_ZIP_FILE_ID are publisher templates only.
