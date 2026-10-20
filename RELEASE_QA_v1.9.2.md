# v1.9.2 NX Redux — verification

- ARM64 native build, fingerprint matches final sources and installer.
- Actual ELF loader --build-info / --help under QEMU with pinned SDK: pass.
- 576 independent DSF/DFF binary goldens (mono/stereo, eight source rates, Native and DoP); production worker/policy with ASan/UBSan: pass.
- Explicit Auto Native → DoP: exact DoP output bytes; Native preferred when available; paused load writes nothing: pass.
- Opened device0 format failure retained when later hw ports are missing: pass.
- DAC aliases scoped to exact USB identity tuple, Unicode/reset/validation and bounded 32-record file: pass.
- Actual sysfs walk and atomic device-log export with fixture USB/ALSA tree: pass. Includes USB raw names, VID/PID, kernel, DSD source and stream/hw_params.
- Full real installer through production OTA worker and installer, v1.9.1 installed baseline, Drive confirmation fixture, SHA/size/wrong-ROM/HTML validation, source migration/custom-source retention, cancellation/reaping and user data preservation: pass.
- CPU/Groq TXT, LED and native Home bridge fixture regressions: pass.
- Official NX Redux feed embedded; no personal keys/user data included.

Physical playback with the user's DAC/DFF and ROM is not tested here. Do not claim universal Native DSD, DoP DSD256, guaranteed temperatures or battery behavior. Generic USB descriptors are not retail model proof; a saved display name is explicitly chosen by the user. No DSD-to-PCM fallback added.

Stock/NextUI binaries remain v1.9.1 and are historical payloads in this source package, not v1.9.2 releases.
