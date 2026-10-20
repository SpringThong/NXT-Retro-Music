# Release QA — NXT Retro Music v1.9.7.6

Fix public SoundCloud playback rejected as Go+/region: the guest API returns MONETIZE for playable public tracks, and current full-length AAC manifests use playback.media-streaming.soundcloud.cloud. The previous plugin rejected MONETIZE; the previous native backend rejected that CDN and mislabeled empty supported formats/HTTP 403 as track restrictions.

## Verification

- Reproduced the old permission predicate rejecting a public MONETIZE/BLACKBOX fixture. Live guest search returned zero app rows before the fix, twenty after. Two tracks yielded full unencrypted AAC HLS formats. A live 25-segment fMP4 manifest from the new CDN had ENDLIST, an initialization map and no encryption key. No signed URL/audio is stored in QA outputs.
- Real plugin tests retain private, SNIP/BLOCK, access preview/blocked, streamable false and SUB_HIGH_TIER rejection; previews and encrypted formats still filtered. Playlist metadata hydration and pagination retained.
- Native Rust backend: 12 tests pass with pinned Rust 1.99.0 and locked Cargo dependencies. Test the exact new HTTPS CDN, reject lookalike hosts/credentials/non443, keep preview rejection, genuine AAC/MP3 decoding, HLS/PCM, 52-item pagination, cancellation/reaping. Empty supported formats now reports unsupported format; HTTP 403 remains an access error rather than asserting Go+/region.
- Cross-compiled actual ARM64 helper with pinned GCC SDK linker, loaded its current ready event under QEMU. Full QEMU live transport hit an environment network error; it is not reported as passed.
- Full live flow succeeded using the same production Rust sources compiled natively for x86_64: live 20-row guest search, AAC 44.1 kHz decode to non-silent PCM, seek to 30 seconds and decode again, resolve original public link. A previously discovered PUBLIC web client cache was seeded to avoid repeated cold discovery timeouts in this network. A host-only development proxy CA was supplied with TLS verification retained; no extra CA is packaged in the app. No personal login/key/cookies or saved audio.
- All three frontend builds, actual ARM64 loader checks, native SoundCloud C IPC/player/lifecycle regression and final installer/OTA verification pass. OTA production worker/stager/installer upgrades a v1.9.7.5 fixture, preserves userdata/cache, rejects bad SHA/size/ROM/HTML, backs up the old app, cancels/reaps. Network replies for OTA tests are local fixtures, not a published Drive release.
- 827 frontend build inputs, 745 other resource files and Spotify backend byte-identical to v1.9.7.5. Changes: version header, current change notes, SoundCloud message/log label, one Python permission predicate, native SoundCloud CDN/error handling and helper build label. Original DSP/raw DSD, lyrics/rendering, screen-off controls, font/theme/profile/cache code unchanged. Vocal reduction remains removed.

## Limits

No physical TrimUI/ROM/DAC/Bluetooth listening test, measured device FPS/temperature/battery or live OTA publication. Live success covers one full decoding/seek flow and two public source probes, not every SoundCloud track/country/account. Genuine subscription, private, preview and territorial limits still apply.
