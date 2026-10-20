# v1.9.7.4 — direct on-device vocal reduction

## Behavior and limits

User requested a live toggle on TrimUI without PC or online service. This implements stereo-centre cancellation, not AI source separation. Strong replaces both output channels with half the stereo difference, removing centre content and centre instruments. Preserve bass adds a 120 Hz one-pole low-pass of the centre. Reverberant/wide/backing vocals may remain. Strong wet output is dual mono, avoiding anti-phase cancellation on a single speaker.

Requested state/method is session-only and defaults off/Strong in a new process. A free legacy L3 slot gains the new action once, with a persisted migration marker; custom mappings and later explicit unassignment are retained. START → first row toggles directly; native Playback settings provides method/help. K status and song/output information reflect enabled/bypassed/mono state. Global mapped input applies in menus/full lyrics too, while respecting lock and screen-off controls.

The existing DSP calls the new effect before EQ/gain/limiting. Steady off returns immediately without changing samples. On/off and method changes ramp over 20 ms. Steady strong processing avoids per-sample ramp calculations. There is no model, FFT, file/network I/O, allocation, or extra lock in the effect callback. Only the audio thread owns float filter state; UI request/method/source/status values are atomic.

Mono detection is conservative: a new source with no stereo content retains its original. Once stereo is observed in that source, a centre-only vocal break stays reduced instead of restoring the singer. Existing playback generation informs source changes even with the display off.

PCM temporarily uses Normal while enabled, keeping the saved sound policy. Disabling restores that policy. Normal/online playback is not reloaded by a toggle. Raw-PCM transport switching uses the unchanged engine position/state-preserving reload and may briefly interrupt a DAC. Original Native/DoP DSD is never converted; enabling on a selected DSD file is rejected; changing from active Karaoke to DSD bypasses the effect. Online/Radio/Podcast source checks do not wrongly reject an old local DSD path.

## Current checks

- ASan/UBSan synthetic signal tests for all three ROM definitions: 8/44.1/48/96/192 kHz; exact removal of a centred sine with stereo-side accompaniment retained; dual-mono output; centre-only vocal passages; new mono source bypass; 20 ms step bounds; low-bass preservation and voice-band attenuation; concurrent atomic requests; finite/clamped full-DSP output.
- Settled disabled production DSP compares byte-for-byte against exact v1.9.7.3 DSP code across five rates, DSP enabled/disabled, EQ, balance, width, crossfeed and ReplayGain.
- Production vocal controller and audio policy with explicit transport/metadata fixtures: Normal/Manual/Auto restore, playing/paused states, stable repeated ticks, no route reopen in Normal/online/speaker/BT, DFF rejection/bypass, active Spotify/SoundCloud/Radio/Podcast despite stale local DSD.
- Config migration/save/reload: old free L3 gains Karaoke, custom L3 preserved, explicitly unassigned L3 remains free after saving; device/profile behavior unchanged.
- Production power and app input routing for all three definitions: dark controls, Power/release guards, timeout, mapped playback/seek/volume and new L3 action, SELECT lock/unlock. Existing fine volume and ROM/HID handling.
- Native UI at 640×480 and 1024×768: 51 themes/60 fonts, K status, first-row live toggle, Strong/Preserve bass menu, no settings save on effect toggles, EQ/lyrics/profile/preview regressions. The native renderer's hardware/audio operations remain fixtures.
- Retained page no-displacement and clean compositor checks; lyric timestamps and DSD parser/packing (576 cases), worker and old audio policy regressions.
- Rebuilt all three ARM64 frontends with GCC 8.3.0/O2; actual --build-info/--help checked under QEMU and pinned SDK.
- Exact v1.9.7.3 source baseline SHA-256 df9e0adb02ee1bd8ee98bd9e06129c2f7ee54acece48af5ba93c9225457cc0d5. Compare 834 current inputs: nine reviewed version/UI/config/DSP/policy inputs changed, three new effect/controller files. All 746 resource files and both online backend executables remain byte-identical; decoder/raw DSD/native compositor source retained.
- Final installer/OTA kits: CRC, executable modes, exact binary bytes, SHA/size/ROM/version, official feeds and userdata exclusion. Production OTA worker/stager/installer from a v1.9.7.3 installed fixture using local service responses, including invalid manifest/hash/size/ROM/HTML, Drive confirmation, preserved userdata and cancellation/reaping.

Detailed current hashes and evidence: BUILD_REPORT_v1.9.7.4_*.json and logs/v1974-*.json.

## Not measured

No physical TrimUI/ROM/DAC/Bluetooth playback or real-song listening test. No guarantee of complete vocal removal, zero-gap raw transport changes, measured CPU load/FPS, temperature or battery savings. No live OTA source was published.
