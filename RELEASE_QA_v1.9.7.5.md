# Release QA — NXT Retro Music v1.9.7.5

Scope: remove the v1.9.7.4 stereo-centre vocal-reduction experiment only. Preserve lyric highlighting/timing, translations, themes/fonts, indexed library, playback sources, original audio modes, DSD, dark-screen controls and the v1.9.7.3 native compositor.

## Verification

- All three ROM frontends freshly compiled with the pinned GCC 8.3 ARM64 toolchain at O2. Build-info/help loaded under QEMU, not a physical firmware session.
- Against the pre-vocal v1.9.7.3 release, 827 build inputs and all 746 resource files are byte-identical. Only version header, safe retired-action config migration and current change notes differ. Original DSP/audio policy, decoders, raw DSD and compositor match that baseline exactly. Both online backend executables are unchanged. No NxtVocal/NC_VOCAL runtime references remain.
- ASan/UBSan config tests for Stock/NextUI/NX Redux: action 22 on each of 16 keys becomes Unassigned; migration survives save/reload and profile changes; action 21 remains Stop and every other valid action retains its ID. Custom playback controls, performance/audio modes, profile appearance, font/theme choices and volumes retained.
- Native production UI tests at 640/1024: 51 themes, 60 fonts, bilingual/italic lyrics, appearance preview Apply/Cancel/save failure, real translation worker using a local service fixture; default L3 is Unassigned.
- Screen-off control/SELECT-lock and volume key tests pass on all three ROM definitions. DSD parser/packing 576 goldens plus actual worker/PCM/online policy tests pass under the NX Redux definition. Other ROM DSD engine sources are unchanged.
- Full final installers and publisher kits: CRC, sizes/SHA-256, manifest ROM/version, assets, executable bits, official feed isolation, no private data, exact inner installer.
- Production OTA worker/stager/installer with local network responses upgrades a v1.9.7.4 fixture for each ROM; rejects wrong SHA/size/ROM/HTML, preserves userdata/cache, creates old-app backup, cancels/reaps correctly. This does not publish or test a live Drive feed.

## Limits

No physical TrimUI/DAC/Bluetooth listening test, measured device FPS, temperature, battery result or live OTA publication. Existing EQ/normal audio preferences remain user-configurable; removing vocal reduction does not disable EQ.
