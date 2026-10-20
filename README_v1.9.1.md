# NXT Retro Music v1.9.1 — Stock / NextUI / NX Redux

Shared source and complete native payloads. Build each frontend with tools/build-stock.py (NXT_BUILD_ROM=NXT_ROM_STOCK), tools/build-nextui.py and tools/build-nxredux.py. Pinned SDK/source archive checksums are in SOURCE.json. Native DSD requires actual ROM USB driver and DAC support; explicit DoP requires a DoP-compatible DAC. See docs/THEM_MOI_v1.9.1.md.

Each ROM has a separate installer and OTA publisher kit. Existing official feed IDs are retained; publisher must upload ZIPs and replace the existing Drive JSON content. Tests use simulated audio/network/ROM interfaces; no physical thermal/battery/DAC confirmation. NX CPU/LED/Home/Tools icon integration remains guarded to NX Redux.

Package with tools/package-stock.py, tools/package-nextui.py, tools/package-nxredux.py and NXT_PACKAGE_ROM=<rom> tools/package-ota-kit.py. Then tools/sync-overlay.py and tools/package-source.py. Source includes historical reports and current BUILD_REPORT_v1.9.1_*.json, compiler input fingerprints and QA fixtures. No user credentials/API keys are included.
