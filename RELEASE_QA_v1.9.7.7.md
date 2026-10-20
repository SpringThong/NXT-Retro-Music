# Release QA — v1.9.7.7

Baseline: unified v1.9.7.6. Version corrected by the user to **1.9.7.7**.

## Passed software checks

- Fresh O2 GCC 8.3.0 ARM64 frontends for Stock, NextUI and NX Redux; every C/H/INC build fingerprint matches the final tree. QEMU runs `--build-info` and `--help` for all three real binaries.
- Native production SDL/TTF renderer with ASan/UBSan: 51 themes, 60 multilingual fonts, 1024×768 and 640×480; 396 PNG frames. Original artwork aspect-fit, bilingual/italic lyrics, word highlights, preview Apply/Cancel, settings/controls/album preview flows remain exercised.
- Checkerboard downscaling averages detail, alpha edges retain white without dark fringes, rounded corners contain fractional coverage, diagonal drawing respects clipping. Cold/warm cached art matches filtered references.
- Repeated icon draws at two sizes create zero new surfaces in 100 warmed iterations. Auralis creates zero new surfaces in 30 warmed paints per state per resolution; status clock is fixed in this deterministic fixture.
- 200 native menu/player theme images checked; both-size active repeat/shuffle states increase mean brightness by over 120 levels in the circular control. Changed pixels stay within the affected controls. Previous/next mirrored mask mean error <0.01 RGB level in a flat-cover fixture; pixel rounding tolerance 1 px.
- Review found and fixed cramped Ocean Blue metadata/cover placement, widened Boombox metadata and kept song information above the bilingual reserved footer. Album fitting honours both width and height.
- 894 C/H/INC baseline files compared: only 10 intended UI/version files changed. 414 non-icon resource files and all online runtime executables remain byte-identical. 316 existing icons regenerated from pinned Lucide/original SVGs; coloured brands and emoji unchanged.
- All three full installer ZIPs: CRC, exact executable/resource bytes, target paths, current config/state/pak versions, official ROM feed and permission bits; no private data. OTA kit contains the exact checked installer and current offline publisher tools.
- Production OTA worker/installer under ASan/UBSan upgrades an installed 1.9.7.6 fixture for each ROM: Drive confirmation, size/SHA, wrong-ROM/HTML rejection, cancellation/reaping, old app backup and preserved settings/profiles/library/translation keys/cache/wallpaper.

## Performance boundaries

No full-app supersampling, extra background service, CPU clock increase or scheduler change. Icon cache 2 MiB, rounded mask cache 4 MiB, existing text cache 6 MiB; art is prepared on source/size/style change. Opaque panel interiors use fast fills and blend only AA corners. Source artwork and the physical screen limit real detail; upscaling a low-resolution cover cannot create original HD detail.

Host ASan timing is informational and is not a device performance guarantee:
- 640 px / 50 warm legacy theme samples: median 1.020 ms, maximum 2.111 ms on this host.
- 1024 px / 50 warm legacy theme samples: median 2.641 ms, maximum 4.646 ms on this host.

No physical TrimUI playback, scan-out, FPS, temperature or battery test was performed. No live OTA feed was uploaded. App functions involving audio/network/hardware in the visual harness use explicit fixtures. Current SoundCloud/Spotify backends are preserved binaries from v1.9.7.6; no new live-service playback claim.

## Package hashes

- NXT_Retro_Music_v1.9.7.7_Stock.zip: `66892730768bb583f604bee9cd5994170aa28fb8c57557f6531c1f0c4ebe2500`
- NXT_OTA_Google_Drive_v1.9.7.7_Stock.zip: `b7f555779b78f3c02e210f4a4edffc131d3db63c20679e1b27f51c93561edf41`
- NXT_Retro_Music_v1.9.7.7_NextUI.zip: `6934d41b4849d1239c709eadf2fd2912288028c667d380a651ba0826d4332313`
- NXT_OTA_Google_Drive_v1.9.7.7_NextUI.zip: `eef219dab071dac27bf1cec03296e236dd32c86368115debe1aa414319fab7f7`
- NXT_Retro_Music_v1.9.7.7_NXRedux.zip: `d660410b95ec76943d4fbbd595dffdd00140cfd245bd88dcfdc4c781c0b16bf1`
- NXT_OTA_Google_Drive_v1.9.7.7_NXRedux.zip: `c5c5ea60cf1748caa0fe16fdf426f14031e3abd50307c09d34da71a3171b7326`
