# v1.9.1 software validation

All three ARM64 loader builds and actual installer OTA integration tests passed (ROM/version/SHA/size, wrong-ROM rejection, cancellation, old-app backup and user-data preservation). Native/DoP byte equality, PCM policies, TXT import, cache retention/eviction and original-lyrics-to-translation cold offline reuse are checked with explicit hardware/network fixtures and ASan/UBSan.

No physical ROM boot, live service API, DAC Native/DoP compatibility, temperature or power measurements. Read each BUILD_REPORT for exact scope and checksums.
