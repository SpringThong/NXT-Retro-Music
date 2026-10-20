# NXT Retro Music

[English](#english) · [Tiếng Việt](#tieng-viet)

<a id="english"></a>

## English

**A music player for TrimUI Brick, Brick Hammer, and Brick Pro, running on NX Redux and NextUI, with a separate Stock ROM package for Brick Pro.**

**Built through vibe coding.**

NXT Retro Music brings local music, online sources, USB DAC playback, bilingual lyrics and extensive personalization together on a retro handheld. Choose a listening profile, pick a theme and make the player feel like your own.

**Idea and implementation:** Nguyễn Xuân Thông — **Spring Thông**. This is a personal, AI-assisted **vibe coding** project, developed through repeated implementation, device testing and feedback. It builds on existing open-source player and firmware projects; credits and license notices are retained below.

This README describes **v1.9.7.7**, with separate packages for **Stock, NextUI and NX Redux**.

## Info

- **Author:** Nguyễn Xuân Thông — **Spring Thông**
- **Hardware family:** TrimUI Brick · Brick Hammer · Brick Pro
- **Firmware packages:** Stock · NextUI · NX Redux
- **Version:** **1.9.7.7**
- **Validation:** Tested by the author on Brick Pro. Brick and Brick Hammer require device/firmware confirmation; controls, LED and DAC behavior can differ.

[Download releases](https://github.com/SpringThong/NXT-Retro-Music/releases) · [Report an issue](https://github.com/SpringThong/NXT-Retro-Music/issues/new)

## Download v1.9.7.7

Choose the installer matching your ROM. Extract it to the SD-card root, then launch NXT from Apps/Tools. **You only need one installer to use the app.**

| ROM | Installer |
| --- | --- |
| **Stock** | [Download Stock](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Stock.zip) |
| **NextUI** | [Download NextUI](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NextUI.zip) |
| **NX Redux** | [Download NX Redux](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NXRedux.zip) |

For development: [complete corresponding source ZIP](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Source.zip). [SHA-256 checksums](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/SHA256SUMS.txt).

![Auralis Now Playing with bilingual lyrics](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/auralis-now-playing.png)

*Native UI render using sample artwork and lyrics. Images show the Vietnamese interface; these are rendered previews, not photographs of a device.*

## Contents

- [At a glance](#at-a-glance)
- [Compatibility and installation](#compatibility-and-installation)
- [Playback, library and playlists](#playback-library-and-playlists)
- [Audio, DAC and DSD](#audio-dac-and-dsd)
- [Lyrics, translation and word highlighting](#lyrics-translation-and-word-highlighting)
- [Make it yours](#make-it-yours)
- [Profiles, recommendations and statistics](#profiles-recommendations-and-statistics)
- [Default controls](#default-controls)
- [Online source setup](#online-source-setup)
- [Power, performance and screen-off controls](#power-performance-and-screen-off-controls)
- [Wi-Fi file transfer](#wi-fi-file-transfer)
- [OTA updates](#ota-updates)
- [Build from source](#build-from-source)
- [Troubleshooting and feedback](#troubleshooting-and-feedback)
- [Credits and licenses](#credits-and-licenses)
- [Support the project](#support-the-project)

## At a glance

| Area | What you can do |
| --- | --- |
| Music | Play files from the SD card; browse tracks, folders, artists and albums; manage queues, favorites and playlists. |
| Online | Use Spotify Premium, public SoundCloud tracks/playlists, public Google Drive audio/folders, internet radio and podcasts. |
| Sound | Switch between speakers, 3.5 mm, Bluetooth and USB DAC; use Normal, Bit-Perfect or Automatic sound modes. |
| DSD | Play local DSF/DFF through a compatible USB DAC using Native DSD, DoP or automatic Native/DoP selection. |
| EQ/DSP | 12 quick presets, custom graphic EQ, advanced 10/15/31-band options, six parametric filters and saved presets. |
| Lyrics | Compact and full-screen lyrics, bilingual display, italic translations, word highlighting and optional Groq voice-timing analysis. |
| Personalization | **51 themes, 60 fonts, 300 selectable icons, 180 3D emoji**, wallpapers, RGB/HEX colors and live appearance previews. |
| Lighting | Music spectrum palettes/styles and reactive LED colors/effects with immediate setting changes. |
| Profiles | Up to 16 local listeners, separate appearance/history, listening statistics and preference-based suggestions. |
| Convenience | Restore the previous song/position, background playback, sleep-and-exit timer, screen-off controls, Wi-Fi file transfer and ROM-specific OTA. |
| Languages | 14 app languages, language-aware on-screen keyboards and offline multilingual font fallback. |

### More of the interface

| Cassette C60 | Studio Meter |
| --- | --- |
| ![Cassette player](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/cassette-player.png) | ![Studio Meter player](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/studio-meter-player.png) |

| Forest Dial menu | Auralis full-screen lyrics |
| --- | --- |
| ![Forest Dial menu](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/forest-dial-menu.png) | ![Auralis bilingual full lyrics](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/auralis-full-lyrics.png) |

## Compatibility and installation

The current target is **TrimUI Brick Pro**. Select the package matching the installed ROM; the three packages share the project source but use different launch paths and ROM adapters.

| ROM | Installer | App folder on the SD card |
| --- | --- | --- |
| Stock | `NXT_Retro_Music_v1.9.7.7_Stock.zip` | `Apps/NXT_Retro_Music/` |
| NextUI | `NXT_Retro_Music_v1.9.7.7_NextUI.zip` | `Tools/tg5040/NXT Retro Music.pak/` |
| NX Redux | `NXT_Retro_Music_v1.9.7.7_NXRedux.zip` | `Tools/NXT Retro Music.pak/` |

1. Download the matching installer from the project's release downloads. The `_Source.zip` is for development; `NXT_OTA_Google_Drive_…` packages are publisher kits.
2. Close NXT completely before replacing an existing installation. Back up your app data before a manual update.
3. Extract the installer **to the root of the SD card**, merging the included `Apps` or `Tools` directory. Avoid an extra nested app folder.
4. Launch NXT from your ROM's Apps/Tools menu.
5. Select or create a listener. NXT opens Now Playing with that listener's saved song and position. Playback is paused by default unless auto-play has been enabled.
6. Choose the music directory under **Settings → Library & files → Music folder**, then open Library.

Connect Wi-Fi and pair Bluetooth devices in the ROM first. USB DAC support depends on the device, ROM USB/audio drivers and the DAC itself. Other TrimUI models are not confirmed targets for these builds.

## Playback, library and playlists

- Local formats recognized by the current native player include **FLAC, WAV, MP3, OGG, Opus, M4A, AAC, ALAC and audio in MP4**, plus the separate DSF/DFF USB DSD path. Decoding also depends on the actual codec inside a file.
- Browse **All tracks / Folders / Artists / Albums**, search metadata and change sorting. Natural sorting keeps names such as `1`, `2`, `10` in a sensible order.
- A saved library index opens quickly. A background incremental check detects changes, skips unchanged metadata and batches database updates. A manual new-music check and full rebuild remain available in Library options.
- Manage the queue, play next, favorite tracks, shuffle and repeat Off/One/All. Previous/next follows the selected shuffle policy.
- Create, rename, reorder and delete playlists; import M3U/M3U8 and export M3U8.
- Resume saved progress, enable optional auto-play, or keep audio playing while returning to menus/background sessions.
- Normal PCM playback offers **Normal, Gapless and Crossfade** transitions, with adjustable crossfade duration. Applicability depends on the source and audio mode.
- Now Playing and song/output information show track counts, title/artist/album, codec, source sample rate, bit depth, bitrate, channels and actual output/DAC information when available.

## Audio, DAC and DSD

### Output and sound modes

Open **Settings → Playback & audio → Sound & output**.

| Mode | Behavior |
| --- | --- |
| Normal | The standard PCM path, with applicable EQ/DSP and playback processing. |
| Bit-Perfect | Requests the exact local integer PCM USB path on a compatible DAC. EQ/DSP, limiter, ReplayGain and resampling are bypassed while that path is active. |
| Automatic | Uses eligible local Bit-Perfect USB playback when possible and falls back to Normal when exact output is unavailable. Online sources use Normal playback. |

The player handles output changes and offers auto-switch preferences, separate saved route volumes, DAC-unplug behavior and output diagnostics. You can adjust volume on the handheld or on a DAC where supported. If software volume is required for PCM, the player can use Normal processing; a saved Bit-Perfect preference alone does not prove that every sample is unchanged. Check the actual active output information.

### EQ and DSP

Tap **SELECT** to open the EQ preset grid. Tap SELECT again to close it without choosing a preset; B also goes back.

- **12 presets:** Flat, Pop, Rock, Jazz, Classical, Vocal, EDM, Acoustic, Bass Boost, Treble, Lo-Fi and Loudness.
- Custom five-band controls and advanced graphic EQ with 10, 15 or 31 bands.
- Six parametric filters with adjustable frequency, gain, Q and enable state.
- Preamp, bass, treble, left/right balance, stereo width, compressor, crossfeed, limiter and a response-curve view.
- Track/album ReplayGain in supported Normal playback.
- Save, load, rename, import, export and delete custom `.nxt-eq` presets; output-specific sound profiles are also supported.

Stored DSP preferences remain available when an exact USB mode bypasses processing.

### DSF / DFF

Select **DSD USB output → Native DSD / DoP / Automatic Native / DoP** in Sound & output. The original DSD path is intended for a compatible USB DAC; it does not silently convert DSD to PCM to play on the internal speaker or Bluetooth.

Both the **DAC and ROM/kernel driver** must support the requested transport. DoP also needs a sufficient PCM carrier rate: DSD64 uses 176.4 kHz; DSD128 uses 352.8 kHz; DSD256 uses 705.6 kHz. A DAC advertising Native DSD support does not establish that every DoP rate is available.

A DAC may report an OEM/controller USB name. Use **DAC display name** to save a friendly label rather than assuming the USB descriptor identifies the retail model. Detailed source, transport and hardware information is available in the output screen and exported device log.

## Lyrics, translation and word highlighting

### Reading lyrics

NXT can use a matching `.lrc` beside a local song, embedded lyrics, cached lyrics and its online lyric lookup sources. Missing original lyrics can be looked up before translation; a matching result is not guaranteed for every recording.

- Read lyrics in Now Playing or press **Y** for full-screen lyrics.
- Choose **Off / Translation / Bilingual** translation display.
- Bilingual display keeps the original and translation together in both views. Translated text uses an italic face/style to distinguish it from the original.
- Follow the current line, scroll manually, change text size and adjust lyric offset.
- Tune current/unsung/translation colors, bold weight, outline and shadow. Auralis also exposes title/artist appearance and independent cover brightness for Now Playing and full lyrics.
- Choose among ten lyric-transition settings, including Off. The full-screen lyric view keeps the spectrum out of the lyric area.

### Translation services and offline cache

| Service | App setup |
| --- | --- |
| MyMemory | No API key entered in NXT; service limits still apply. |
| Groq | Your own API key and a model selected in the app; key entry or TXT import. |
| Google Cloud | Your own Cloud Translation key/account; separate service access and billing. |

The translation target follows the **app language**. Source-language detection can be overridden. Translation runs in a background worker, prioritizes useful cues and reuses saved results. The SD-card translation cache is bounded to **256 MiB** and reuses translated lines offline; this is a storage budget, not a 256 MiB RAM allocation or a guarantee that all songs are already translated. Older saved results can be removed when the cache budget is reached.

Text translation sends lyric text when enabled. **Optional voice-timing analysis is different: it uploads audio only after consent.** Service quotas, model access, translation quality and any charges depend on the selected provider/account. HTTP 429 indicates a service rate/quota limit; retries wait rather than bypassing it. Pending translation requests pause while the app is hidden or the screen is off to reduce background work.

### Word-by-word highlighting

NXT distinguishes three timing sources:

1. **Real word timestamps:** Enhanced LRC word markers can work fully offline without a translation API key.
2. **Optional Groq voice timing:** Analyze a supported local audio file, up to **10 minutes**, using your own Groq key and Whisper Large v3 after agreeing to the audio upload. Automatic analysis is off by default. Results are cached separately for offline reuse, within **16 MiB / 256 timing files**.
3. **Offline estimated timing:** An optional approximation based on line timing and text length, labelled as estimated. It does not detect the singer's voice.

Turn estimated timing off if you want only existing or successfully matched generated word timings. Groq recognition can miss singing, backing vocals, accents or a different recording; a global lyric offset cannot repair incorrect word boundaries. Protected online streams and the raw DSD path are not recorded for this analysis.

## Make it yours

Open **Personal → Profile → Personal appearance**. Appearance, LEDs and spectrum settings live here, while device/audio controls remain in Settings.

### Themes and Auralis

- **Theme collection:** 51 complete layouts, including Original NXT, cassette/radio/record-player designs, clean modern palettes and Auralis.
- **Original theme options:** system/dark/light appearance, accent palette, custom RGB/HEX, fonts, interface/lyric text sizes, wallpaper gallery, wallpaper veil, menu opacity, album-art/vinyl selection, animations, menu icons, lyric offset and lyric styling.
- **Auralis:** full-screen album artwork behind readable lyrics, glass controls, bilingual line pairs and a separate full-lyrics view. Its dedicated options appear **only when Auralis is selected**.
- Separate Auralis cover-brightness controls for Now Playing and full-screen lyrics; use its reference font or a selected font family.
- **v1.9.7.7 refinements:** 316 existing icons rebuilt from vector assets, smoother rounded edges/diagonals, slightly larger Auralis controls, filled active states, filtered artwork scaling and corrected Ocean Blue/Boombox information layout.

Album artwork keeps its source detail and aspect ratio. A small source image cannot become genuine HD detail just by enlarging it.

<details>
<summary>All 51 themes</summary>

|  |  |  |
| --- | --- | --- |
| 1. Original NXT | 2. Cassette C60 | 3. Vintage Radio |
| 4. Walkman | 5. Reel to Reel | 6. Hi-Fi Rack |
| 7. CD Deck | 8. Jukebox | 9. CRT Terminal |
| 10. Turntable | 11. Modern Focus | 12. Modern Ambient |
| 13. Boombox | 14. MiniDisc | 15. Pocket Wheel |
| 16. Classic 90s | 17. Tube Amplifier | 18. Gramophone |
| 19. Car Audio | 20. Synth Studio | 21. Oscilloscope |
| 22. Bento | 23. E-Ink | 24. Hologram |
| 25. Nordic Studio | 26. Night Drive | 27. Jazz Lounge |
| 28. Soft Paper | 29. Aurora | 30. Ocean Blue |
| 31. Lo-fi Desk | 32. Film Gallery | 33. Studio Meter |
| 34. Mono Slate | 35. Rose Quartz | 36. Gallery Noir |
| 37. Forest Dial | 38. Coral Tape | 39. Midnight Indigo |
| 40. Copper Sunset | 41. Sakura Glass | 42. Cobalt Grid |
| 43. Desert Radio | 44. Emerald Studio | 45. Lavender Paper |
| 46. Ice Blue | 47. Amber Console | 48. Ruby Velvet |
| 49. Graphite Split | 50. Olive Notebook | 51. Auralis Themes |

</details>

### Fonts, wallpaper, avatars and previews

- **60 font choices**, including MiSans, HarmonyOS Sans, OPPO Sans, Inter, Roboto, Be Vietnam Pro, Geist, Public Sans and several serif/monospace families.
- Offline font fallback covers Vietnamese and supported CJK/Thai scripts. Third-party font names and licenses are retained; Apple SF Pro is not bundled.
- Built-in wallpapers, SD-card JPG/PNG import and an online wallpaper gallery with saved copies; adjust the veil and menu opacity for readability. Original-theme backgrounds and fixed theme layouts have different applicable options.
- **300 selectable symbols and 180 3D emoji**, plus photo or text avatars and customizable menu icons.
- In the live appearance editor, preview typography/colors/effects over the current artwork, switch between Now Playing/full lyrics, then **Apply or Cancel**. Draft changes are not continuously saved while adjusting.

| Preview control | Action |
| --- | --- |
| D-pad | Select a property and adjust its value. |
| Y on a color property | Enter/leave RGB adjustment. |
| L1 / R1 in RGB adjustment | Larger component steps. |
| X | Switch Now Playing / full-lyrics preview. |
| SELECT | Replay the sample transition. |
| A / B | Apply / cancel. |

Sample lyrics in the editor are labelled as a preview and do not replace the real song's lyrics or create translation requests.

<details>
<summary>All 60 font choices</summary>

|  |  |  |
| --- | --- | --- |
| 1. Noto Sans | 2. Inter | 3. Roboto |
| 4. Source Sans 3 | 5. Nunito Sans | 6. Manrope |
| 7. Rubik | 8. Ubuntu | 9. Lato |
| 10. Montserrat | 11. Noto Serif | 12. Source Serif 4 |
| 13. Lora | 14. Merriweather | 15. JetBrains Mono |
| 16. Fira Code | 17. MiSans · Xiaomi / NX Redux | 18. HarmonyOS Sans · Huawei |
| 19. HarmonyOS Sans Condensed · Huawei | 20. OPPO Sans 4.0 · OPPO | 21. Roboto Condensed · Google |
| 22. Roboto Slab · Google | 23. Be Vietnam Pro | 24. Plus Jakarta Sans |
| 25. Work Sans | 26. Mulish | 27. Public Sans |
| 28. IBM Plex Sans | 29. Lexend | 30. Onest |
| 31. Geist | 32. Wix Madefor Text | 33. Readex Pro |
| 34. Barlow | 35. Asap | 36. Archivo |
| 37. Exo 2 | 38. Chakra Petch | 39. Prompt |
| 40. Kanit | 41. Sarabun | 42. Quicksand |
| 43. Comfortaa | 44. Space Grotesk | 45. Cabin |
| 46. Open Sans | 47. Arimo | 48. Josefin Sans |
| 49. Bitter | 50. Playfair Display | 51. Literata |
| 52. Vollkorn | 53. Cormorant Garamond | 54. Alegreya |
| 55. Crimson Pro | 56. Fraunces | 57. Spectral |
| 58. Gelasio | 59. Epilogue | 60. Newsreader |

</details>

### Spectrum and LEDs

**Spectrum:** 21 palette choices, custom RGB endpoints and five styles: Solid, Gradient, 7-color Rainbow, VU and Multicolor Mix. VU colors reflect signal magnitude; it is a level visualization. Toggle/cycle the spectrum from the player or configure it from Personal appearance.

**LEDs:** on/off, brightness, constant color or music-reactive operation, preset/custom RGB colors and six effects: Solid, Two-color gradient, Rainbow wave, Split colors, Aurora and Breathing. Changes apply without an extra LED power cycle where the ROM/hardware interface supports them.

### Languages and keyboard

The first-launch app language is **English**. Available languages are English, Vietnamese, French, Russian, Korean, Japanese, Thai, Simplified Chinese, Traditional Chinese, German, Spanish, Brazilian Portuguese, Italian and Indonesian.

Choose **Settings → Language**. The in-app keyboard follows the selected language/layout, with Vietnamese Telex, Korean composition, Japanese input and offline Japanese/Chinese character candidates where applicable. This configures NXT's keyboard, not the ROM's global keyboard.

## Profiles, recommendations and statistics

- Create, rename, switch and delete up to **16 local listener profiles**, with confirmation before deletion. Deleting a listener removes that profile's data, not the music files on the card.
- Personalize nickname, avatar, birthday and appearance; choose genre, mood and country preferences from Personal instead of answering a mandatory check-in at every launch.
- Keep profile-specific listening history, favorites, appearance and saved listening progress. History recording can be disabled.
- Explore **Continue listening**, **Familiar mix**, **Not heard yet**, **Long unheard** and preference-based Spotify/SoundCloud suggestions/playlists, subject to source access.
- Review **Today / 7 days / This month** listening time, plays, artists, top tracks/artists and daily charts. History can be paged and tracks reopened.
- Current listening statistics cover supported Local/Spotify/SoundCloud/Drive sessions; radio and podcasts are not included in those profile history totals.

## Default controls

These are the default **Now Playing** bindings; you can remap 16 controls under **Settings → Key settings**. Menu and full-lyrics screens have their own context hints.

| Button | Default action |
| --- | --- |
| A | Play / pause. |
| B | Return to the menu. |
| X | Queue / track list. |
| Y | Full-screen lyrics. |
| L1 / R1 | Previous / next track. |
| L2 | Cycle spectrum palette/show state. |
| R2 | Favorite current track. |
| SELECT, short press | Open/close EQ. |
| START | Quick options: output, mode, timer, shuffle, repeat, track/output info and more. |
| Left / right | Seek backward / forward. |
| SELECT + up/down | Device volume. |
| SELECT + left/right | Display brightness. |
| Hold SELECT | Lock / unlock controls. |
| L3 / R3 | Unassigned by default; these are stick clicks. |

The front logo button is the app exit shortcut. The side Home/task-manager behavior belongs to the ROM; the NX Redux build includes its music-service integration for supported firmware. Use **Play in background** when you want to leave the foreground while retaining playback.

In full-screen lyrics: up/down scroll, Y switches following, X changes lyric size, L1/R1 adjust offset, START opens translation/timing options, A pauses/resumes and B returns.

## Online source setup

### Spotify Premium — PC credentials and QR authorization

Open **Online music → Spotify**. Spotify Premium and valid playback authorization are required. Search, account playlists, liked songs and saved albums additionally depend on Web API access; the app checks library and playback permissions separately.

The complete setup below covers the **PC import + QR** flow used with this project, including cases where scanning QR alone leaves the account unverified:

1. On a PC, install Python 3 and open the included `Spotify_Login_PC` folder. On Windows run `Dang_nhap_Spotify.bat`, or run this command from that folder:

   ```bash
   python Spotify_Login.py
   ```

2. Authorize in the browser. The tool writes **`spotify-credentials.json`** into its working/output directory. For your own authorized Spotify application, it accepts `--client-id`; its default callback is `http://127.0.0.1:8080/callback`.
3. Copy that JSON **beside `launch.sh` in the app folder for your ROM**, using the installation table above.
4. On the Brick, open **Spotify → Account / Account & tools → Advanced · import existing PC login**.
5. Choose **Log in / switch account by QR**, scan the QR or enter its displayed code at **https://spotify.com/pair**, and approve using **the same Spotify Premium account** as the PC authorization.
6. Wait for playback authentication to complete before selecting music. NXT saves the authorized session for later launches.

Some authorized setups can complete QR login directly. If only library access is restricted, **Play Spotify track link** can use an authorized playback session; it does not bypass rejected playback authentication or Premium requirements. Check `NXT_Spotify_Log.txt` when login fails. OAuth app access, scopes and Spotify service policies still apply.

### Groq — translation key without typing it on the handheld

1. Create an API key in your own account at **https://console.groq.com/keys**.
2. On your PC, create **`groq-api-key.txt`** containing one complete key on one line:

   ```text
   gsk_REPLACE_WITH_YOUR_COMPLETE_KEY
   ```

3. Put the file in **`Music/groq-api-key.txt` at the SD-card root**, or beside the app's `launch.sh`.
4. Open **Settings → Playback & audio → Playback → Lyric translation**, or open translation options from full lyrics with START.
5. Select **Groq** as the translation service, then **Groq API key → Import from TXT**. Choose the model and Bilingual/Translation display.

A `GROQ_API_KEY=...` line is also accepted. The bundled v1.9.7.7 model selector contains `openai/gpt-oss-20b`, `openai/gpt-oss-120b` and `qwen/qwen3.8-27b`; the provider determines which IDs your account can actually use. Pick another offered model or service if access fails.

The same key can be used for the separate optional voice-timing feature, which asks for consent before audio upload and uses the provider's audio quota. Keep keys and credential JSONs private; do not include them in a public repository.

### Google Drive — public folders and audio files

1. Share the intended audio file/folder so it is publicly readable.
2. Open **Online music → Google Drive → Add Google Drive link** and enter the shared link.
3. Alternatively, import a `.txt` or `.url` file containing one Drive link per line.
4. Open a saved link, browse subfolders and select a track, or choose **Listen to the whole folder** for its audio queue.

Playback streams supported files rather than requiring a complete offline download first. Network buffering still occurs. Private/login-only links and provider download limits may prevent access. Saved Drive links can be refreshed or removed in the app.

### SoundCloud, radio, podcasts and download helper

- **SoundCloud:** search public tracks/playlists and use playable suggestions without entering a SoundCloud API key. Access restrictions, previews, private or subscription-only tracks remain restricted. The v1.9.7.6 public-track playback fix is retained.
- **Radio:** browse/listen to internet stations through the radio module.
- **Podcasts:** browse subscriptions/episodes through the podcast module.
- **YouTube audio download helper:** the source snapshot also includes a separate download/search/queue module using yt-dlp. It is not an authenticated YouTube Music library integration; availability depends on the helper and source site. Use permitted content.

## Power, performance and screen-off controls

Open **Settings → Device performance** for **Battery saver / Balanced / Performance** and the device monitor.

The modes adjust visual work. NextUI and NX Redux also attempt reversible CPUFreq policy changes when the ROM permits them; Stock retains ROM-managed CPU policy. A mode name alone is not a guarantee that a ROM accepted a CPU cap. The status line reports what was applied, and NXT restores CPU settings it still owns when leaving the session.

The monitor reads available CPU usage/clock, temperature, RAM and battery telemetry. Battery history represents **whole-device percentage drop during NXT sessions**, excluding charging/gaps; it is not a laboratory measurement of this app's power alone.

- Set a sleep timer in quick options or playback settings. At expiry NXT saves progress/settings, stops playback and closes the app, including its background session. The ROM then decides when the handheld suspends; NXT does not guarantee immediate deep sleep.
- Configure automatic screen-off and background playback.
- Enable **Settings → Playback & audio → Playback → Screen-off controls** to keep mapped controls working while the display stays dark. Power wakes it; holding SELECT still locks/unlocks input. The option is off by default.
- Cached artwork/text/icons, bounded caches, incremental library indexing and less work while hidden aim to reduce unnecessary load. Motion scheduling can target roughly 60 Hz for active animations, rather than continuously redrawing every static page.

Actual smoothness, temperature and battery life depend on the ROM, output, network work, selected effects and content. The v1.9.7.7 release has software-rendering/build/OTA checks; **no measured device FPS or thermal guarantee** is claimed.

## Wi-Fi file transfer

Open **Settings → Library & files → Wi-Fi file transfer**. Join the same trusted Wi-Fi network on your phone/PC, open the **address shown by NXT**, enter its six-digit PIN and select files to upload into the configured music folder.

The server runs on demand. The screen shows progress; X cancels a file and B stops transfer. Completed uploads trigger a library refresh. This release provides Wi-Fi file copy; USB-C is used for supported audio hardware rather than an app USB file-copy mode.

## OTA updates

Each ROM package includes its own official update source. Open:

**Information, guide & updates → Software update → Check for updates**

Users normally do not need to type a feed URL. If a custom source was previously selected, use **Restore publisher source** to return to the bundled feed.

The updater checks package identity, ROM/device, version, size, SHA-256 and ZIP content before installation; it stages the new app, retains an old-app backup and keeps user data outside the replaced app directory. Leave enough SD space for the package, staging and backup.

**For publishers:** each `NXT_OTA_Google_Drive_v1.9.7.7_<ROM>.zip` contains the exact installer and `CHUAN_BI_OTA.html` to prepare `update.json`. Upload the installer first, generate the JSON for that exact ZIP, then replace the content of the **existing ROM-specific Drive manifest file** while keeping its file ID/link. The ZIP kits do not publish themselves. Keep Stock, NextUI and NX Redux feeds separate.

## Build from source

The default branch is a compact release page. To build, download the complete `_Source.zip` from [Releases](https://github.com/SpringThong/NXT-Retro-Music/releases/tag/v1.9.7.7), or use the preserved [source-v1.9.7.7 branch](https://github.com/SpringThong/NXT-Retro-Music/tree/source-v1.9.7.7). Run the commands below from the extracted source root.

The implementation is a native **C/SDL** application with Rust Spotify/SoundCloud helpers and bundled runtime/dependency assets. One source tree produces three ROM-specific frontends and packages.

Start from the **complete source snapshot**, preserving its libraries, assets, vendor sources and helper binaries. A lightweight web upload of just `upstream/src` is not a complete build environment. The frontend cross-build expects an **x86_64 Linux host, Python 3.11+**, standard host utilities and network access for the pinned SDK on first use. The tools download/check the pinned compiler and sysroot; a system ARM compiler is not substituted automatically.

```bash
# Validate the bundled source inputs and ROM metadata.
python3 tools/build-release.py --rom all --check

# Build installers and OTA publisher kits for every ROM.
python3 tools/build-release.py --rom all --jobs 4

# Or select one target.
python3 tools/build-release.py --rom nxredux --jobs 4

# Compile frontends without packaging.
python3 tools/build-release.py --rom all --jobs 4 --build-only
```

Results are written to `dist/stock-release/`, `dist/nextui-release/` and `dist/nxredux-release/`. Existing helper executables are reused by this frontend build; changing those helpers requires their separate Rust build/toolchain workflow.

| Source area | Purpose |
| --- | --- |
| `upstream/src/` | Shared native application, player, audio, lyrics, profiles and UI. |
| `upstream/res/`, `upstream/licenses/` | Runtime assets, fonts, catalogs and third-party notices. |
| `stock/`, `nxredux/`, `upstream/` | Target launch/configuration and ROM-specific payloads. |
| `spotify/`, `soundcloud/` | Online helper source and vendored components. |
| `relink/`, `sources/` | Bundled build libraries, source archives and pinned inputs. |
| `tools/`, `qa/` | Build/package/update tools and software checks. |
| `Spotify_Login_PC/` | Optional PC OAuth authorization tool. |
| `SOURCE.json`, `RELEASE_QA_v1.9.7.7.md` | Snapshot provenance and release verification/limitations. |

For GitHub, keep development sources readable and distribute installer/OTA/full-source ZIPs through **Releases** rather than committing release archives. Preserve required assets or document how contributors restore the full build snapshot. See the accompanying [publishing notes](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/docs/GITHUB_PUBLISH.vi.md).

## Troubleshooting and feedback

Useful details for an issue: app version, ROM/version, source type, steps to reproduce, file format and connected output/DAC. Include relevant logs after removing personal details; never attach API keys or login credentials.

| Symptom | Check |
| --- | --- |
| Spotify account appears unverified | Complete PC import and same-account QR playback authorization; inspect the Spotify log. |
| Translation returns HTTP 429 | Wait for provider limits to recover or choose another configured service. |
| New song has no translation offline | It needs original lyrics and an online translation first; only saved results are reusable offline. |
| Highlighting misses the singer | Verify the exact recording, lyric offset and whether timings are real, generated or estimated. |
| Native DSD unavailable | Check USB DAC transport/rate support and the ROM driver; inspect Native/DoP diagnostics. |
| Bluetooth connects but audio is elsewhere | Pair/connect in the ROM and verify the active output in NXT. |
| No new OTA version | Check the installed version and the published manifest; the manifest must describe the exact newer package for that ROM. |

In **Application information**, START exports `NXT_App_Log.txt` and Y exports `NXT_Device_Log.txt`. Service logs such as `NXT_Spotify_Log.txt` and `NXT_SoundCloud_Log.txt` are available for source-specific diagnosis. Exported app/device logs are placed beside the app launcher; available launcher/input logs depend on the ROM.

## Credits and licenses

- **Nguyễn Xuân Thông — Spring Thông:** NXT concept, implementation direction, personalization and project development through vibe coding.
- **Mohammad Syuhada** and **nborodikhin / NextUI Music Player:** upstream player work. Upstream repository recorded in this snapshot: https://github.com/nborodikhin/nextui-music-player.
- **NextUI, NX Redux and TrimUI community projects:** firmware/platform work and integrations.
- **SDL, FFmpeg, librespot, yt-dlp, libsamplerate, SQLite, FreeType, HarfBuzz, FriBidi, utf8proc**, icon/font authors and other bundled dependencies.

NXT extension files carry **`GPL-3.0-only`** notices; see [NXT GPL text](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/licenses/NXT-GPL-3.0.txt). The original player retains its [MIT license](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/LICENSE), and NextUI and third-party components retain their own notices. This is not a claim that every bundled file uses one license. Preserve the [third-party license directory](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/licenses/) when distributing the project, including font and icon licenses.

This is an independent community project, not an official TrimUI, Spotify, SoundCloud, Google or Groq application. Those names identify their respective products/services.

## Support the project

If you enjoy NXT Retro Music, sharing a demo, reporting reproducible bugs, helping with translations or starring the repository all help.

Optional support via **PayPal: `thongkhung2@gmail.com`**. Support is voluntary and does not unlock cloud-service quotas or paid account access.

Built for a small handheld, shaped around the way I like to listen. — **Spring Thông**

---

<a id="tieng-viet"></a>

## Tiếng Việt

**Trình nghe nhạc cho dòng TrimUI Brick, Brick Hammer và Brick Pro trên NX Redux và NextUI, kèm bộ cài Stock riêng cho Brick Pro.**

**Một dự án vibe coding, tập trung vào nghe nhạc và cá nhân hóa.**

NXT Retro Music kết hợp nhạc trong thẻ nhớ, các nguồn nghe online, USB DAC, lời song ngữ và nhiều tùy chọn cá nhân hóa trên một chiếc máy chơi game cầm tay. Chọn hồ sơ, chọn giao diện và tạo không gian nghe nhạc riêng của bạn.

**Ý tưởng và thực hiện:** Nguyễn Xuân Thông — **Spring Thông**. Đây là dự án cá nhân được phát triển theo cách **vibe coding**, dùng công cụ AI hỗ trợ triển khai, sau đó chỉnh sửa qua nhiều vòng thử máy và phản hồi. Dự án kế thừa những thành phần mã nguồn mở đã có; phần ghi công và giấy phép được giữ lại ở cuối trang.

README này giới thiệu **v1.9.7.7**, với bộ cài riêng cho **Stock, NextUI và NX Redux**.

## Thông tin

- **Tác giả:** Nguyễn Xuân Thông — **Spring Thông**
- **Dòng thiết bị:** TrimUI Brick · Brick Hammer · Brick Pro
- **Các bộ cài:** Stock · NextUI · NX Redux
- **Phiên bản:** **1.9.7.7**
- **Kiểm chứng:** Tác giả thử sử dụng trên Brick Pro. Brick và Brick Hammer cần kiểm tra trên máy/ROM cụ thể; nút bấm, LED và DAC có thể khác nhau.

[Tải các bản phát hành](https://github.com/SpringThong/NXT-Retro-Music/releases) · [Báo lỗi](https://github.com/SpringThong/NXT-Retro-Music/issues/new)

## Tải v1.9.7.7

Chọn đúng bộ cài cho ROM đang dùng, giải nén vào gốc thẻ nhớ rồi mở NXT trong Apps/Tools. **Dùng app chỉ cần tải một bản cài.**

| ROM | Bản cài |
| --- | --- |
| **Stock** | [Tải bản cài Stock](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Stock.zip) |
| **NextUI** | [Tải bản cài NextUI](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NextUI.zip) |
| **NX Redux** | [Tải bản cài NX Redux](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NXRedux.zip) |

Dành cho phát triển: [source ZIP đầy đủ tương ứng](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Source.zip). [Mã kiểm tra SHA-256](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/SHA256SUMS.txt).

![Auralis Now Playing với lời song ngữ](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/auralis-now-playing.png)

*Ảnh render từ giao diện native, sử dụng bìa và lời mẫu. Đây là ảnh dựng kiểm tra giao diện tiếng Việt, không phải ảnh chụp máy thật.*

## Mục lục

- [Nhìn nhanh các tính năng](#nhìn-nhanh-các-tính-năng)
- [Tương thích và cài đặt](#tương-thích-và-cài-đặt)
- [Phát nhạc, thư viện và playlist](#phát-nhạc-thư-viện-và-playlist)
- [Âm thanh, DAC và DSD](#âm-thanh-dac-và-dsd)
- [Lời bài hát, dịch lời và tô từng chữ](#lời-bài-hát-dịch-lời-và-tô-từng-chữ)
- [Cá nhân hóa theo ý bạn](#cá-nhân-hóa-theo-ý-bạn)
- [Hồ sơ, gợi ý và thống kê](#hồ-sơ-gợi-ý-và-thống-kê)
- [Các nút điều khiển mặc định](#các-nút-điều-khiển-mặc-định)
- [Thiết lập các nguồn online](#thiết-lập-các-nguồn-online)
- [Hiệu năng, pin và thao tác khi tắt màn hình](#hiệu-năng-pin-và-thao-tác-khi-tắt-màn-hình)
- [Chuyển file qua Wi-Fi](#chuyển-file-qua-wi-fi)
- [Cập nhật OTA](#cập-nhật-ota)
- [Build từ mã nguồn](#build-từ-mã-nguồn)
- [Xử lý lỗi và góp ý](#xử-lý-lỗi-và-góp-ý)
- [Ghi công và giấy phép](#ghi-công-và-giấy-phép)
- [Ủng hộ dự án](#ủng-hộ-dự-án)

## Nhìn nhanh các tính năng

| Nhóm | Bạn có thể làm gì? |
| --- | --- |
| Nghe nhạc | Phát nhạc trong thẻ; duyệt theo bài, thư mục, nghệ sĩ, album; quản lý hàng đợi, yêu thích và playlist. |
| Online | Spotify Premium, bài/playlist SoundCloud công khai, nhạc và thư mục Google Drive công khai, radio internet và podcast. |
| Âm thanh | Loa máy, tai nghe 3,5 mm, Bluetooth, USB DAC; chọn Thường, Bit-Perfect hoặc Tự động. |
| DSD | Phát DSF/DFF trên thẻ qua USB DAC tương thích bằng Native DSD, DoP hoặc tự động chọn Native/DoP. |
| EQ/DSP | 12 preset nhanh, EQ đồ họa tùy chỉnh, EQ nâng cao 10/15/31 dải, 6 bộ lọc PEQ và preset riêng. |
| Lời nhạc | Lời trên Now Playing và full màn hình, song ngữ, bản dịch nghiêng, tô từng chữ và tùy chọn phân tích mốc hát bằng Groq. |
| Cá nhân hóa | **51 theme, 60 font, 300 icon để chọn, 180 emoji 3D**, hình nền, màu RGB/HEX và chỉnh giao diện có xem trước. |
| Hiệu ứng | Phổ nhạc nhiều bảng màu/kiểu hiển thị; LED theo nhạc, màu và hiệu ứng áp dụng ngay. |
| Hồ sơ | Tối đa 16 người nghe cục bộ, giao diện/lịch sử riêng, thống kê và gợi ý theo sở thích. |
| Tiện ích | Nhớ bài và vị trí nghe, phát nền, hẹn giờ dừng và đóng app, thao tác khi tắt màn hình, chép nhạc Wi-Fi và OTA riêng từng ROM. |
| Ngôn ngữ | 14 ngôn ngữ app, bàn phím trong app theo ngôn ngữ và font dự phòng đa ngôn ngữ dùng offline. |

### Một vài giao diện khác

| Cassette C60 | Studio Meter |
| --- | --- |
| ![Trình phát Cassette](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/cassette-player.png) | ![Trình phát Studio Meter](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/studio-meter-player.png) |

| Menu Forest Dial | Full lời Auralis |
| --- | --- |
| ![Menu Forest Dial](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/forest-dial-menu.png) | ![Full lời song ngữ Auralis](https://raw.githubusercontent.com/SpringThong/NXT-Retro-Music/v1.9.7.7/docs/screenshots/auralis-full-lyrics.png) |

## Tương thích và cài đặt

Mục tiêu của bản hiện tại là **TrimUI Brick Pro**. Chọn đúng bộ cài cho ROM đang sử dụng. Ba bản dùng chung mã nguồn dự án nhưng có đường dẫn khởi chạy và phần tích hợp ROM riêng.

| ROM | Bộ cài | Thư mục app trên thẻ nhớ |
| --- | --- | --- |
| Stock | `NXT_Retro_Music_v1.9.7.7_Stock.zip` | `Apps/NXT_Retro_Music/` |
| NextUI | `NXT_Retro_Music_v1.9.7.7_NextUI.zip` | `Tools/tg5040/NXT Retro Music.pak/` |
| NX Redux | `NXT_Retro_Music_v1.9.7.7_NXRedux.zip` | `Tools/NXT Retro Music.pak/` |

1. Tải ZIP cài đặt đúng ROM từ nơi phát hành của dự án. File `_Source.zip` dành cho phát triển; file `NXT_OTA_Google_Drive_…` là bộ chuẩn bị OTA cho người phát hành.
2. Thoát hẳn NXT trước khi thay bản cũ. Sao lưu dữ liệu cá nhân trước khi cập nhật thủ công.
3. Giải nén bộ cài **vào gốc thẻ nhớ**, gộp thư mục `Apps` hoặc `Tools` có sẵn trong ZIP. Tránh tạo thêm một lớp thư mục app bị lồng bên ngoài.
4. Mở NXT trong mục Apps/Tools của ROM.
5. Chọn hoặc tạo hồ sơ người nghe. App mở Now Playing với bài và vị trí đã lưu của hồ sơ đó. Mặc định chưa tự phát, trừ khi bạn đã bật tùy chọn tự phát.
6. Vào **Cài đặt → Thư viện & tệp → Thư mục nhạc** để chọn nguồn nhạc, sau đó mở Thư viện.

Bật Wi-Fi, ghép và kết nối Bluetooth trong ROM trước. USB DAC còn phụ thuộc phần cứng, driver USB/âm thanh của ROM và DAC. Các bộ cài này chưa xác nhận khả năng chạy trên những mẫu TrimUI khác.

## Phát nhạc, thư viện và playlist

- Trình phát native hiện nhận các định dạng **FLAC, WAV, MP3, OGG, Opus, M4A, AAC, ALAC và âm thanh trong MP4**, cùng đường phát DSD USB riêng cho DSF/DFF. Khả năng giải mã còn phụ thuộc codec thực bên trong file.
- Duyệt theo **Tất cả bài / Thư mục / Nghệ sĩ / Album**, tìm trong thông tin bài và đổi cách sắp xếp. Sắp xếp tự nhiên giữ tên như `1`, `2`, `10` theo thứ tự hợp lý.
- Thư viện có chỉ mục lưu sẵn để mở nhanh. App kiểm tra thay đổi tăng dần ở nền, bỏ đọc lại metadata của file không đổi và gom cập nhật cơ sở dữ liệu. Trong tùy chọn Thư viện vẫn có kiểm tra nhạc mới và quét lại toàn bộ.
- Quản lý hàng đợi, phát tiếp theo, yêu thích, ngẫu nhiên và lặp Tắt/Một bài/Tất cả. Chuyển bài trước/sau tuân theo chế độ ngẫu nhiên đã chọn.
- Tạo, đổi tên, đổi thứ tự và xóa playlist; nhập M3U/M3U8, xuất M3U8.
- Nhớ vị trí nghe, tùy chọn tự phát khi mở app và tiếp tục phát khi quay về menu hoặc đưa app xuống nền.
- Luồng PCM Thường có các lựa chọn **Thông thường, Liền mạch và Chồng âm**, kèm thời gian crossfade tùy chỉnh. Việc áp dụng phụ thuộc nguồn nhạc và chế độ âm thanh.
- Now Playing và mục thông tin bài hát/đầu ra hiển thị số thứ tự/tổng bài, tên bài/nghệ sĩ/album, codec, sample rate nguồn, độ sâu bit, bitrate, số kênh và thông tin DAC/đầu ra thực tế khi có dữ liệu.

## Âm thanh, DAC và DSD

### Đầu ra và chế độ âm thanh

Mở **Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra**.

| Chế độ | Cách hoạt động |
| --- | --- |
| Thường | Luồng PCM thông thường; có thể áp dụng EQ/DSP và các xử lý phát nhạc tương ứng. |
| Bit-Perfect | Yêu cầu đường USB PCM số nguyên nguyên bản cho nhạc local phù hợp và DAC tương thích. Khi đường này hoạt động, bỏ qua EQ/DSP, limiter, ReplayGain và đổi tần số lấy mẫu. |
| Tự động | Ưu tiên Bit-Perfect USB cho nguồn local đủ điều kiện; dùng Thường nếu đầu ra nguyên bản không khả dụng. Nguồn online dùng luồng Thường. |

App xử lý thay đổi đầu ra, có tùy chọn tự chuyển, lưu âm lượng theo từng đường phát, chọn cách xử lý khi rút DAC và xem chẩn đoán đầu ra. Bạn có thể chỉnh âm lượng trên máy hoặc DAC tùy hỗ trợ. Nếu PCM cần điều chỉnh âm lượng bằng phần mềm, app có thể chuyển sang luồng Thường; chỉ chọn Bit-Perfect trong cài đặt chưa chứng minh mọi mẫu âm thanh đều giữ nguyên. Hãy xem trạng thái đầu ra đang hoạt động.

### EQ và DSP

Nhấn ngắn **SELECT** để mở bảng preset EQ. Nhấn SELECT thêm lần nữa để đóng mà không phải chọn preset; B vẫn quay lại được.

- **12 preset:** Flat, Pop, Rock, Jazz, Classical, Vocal, EDM, Acoustic, Bass Boost, Treble, Lo-Fi và Loudness.
- Tùy chỉnh nhanh 5 dải; EQ đồ họa nâng cao 10, 15 hoặc 31 dải.
- 6 bộ lọc EQ tham số, chỉnh tần số, gain, Q và bật/tắt từng bộ lọc.
- Preamp, bass, treble, cân bằng trái/phải, độ rộng stereo, compressor, crossfeed, limiter và xem đường đáp ứng.
- ReplayGain theo bài/album trong luồng Thường được hỗ trợ.
- Lưu, nạp, đổi tên, nhập, xuất và xóa preset `.nxt-eq`; có cấu hình âm thanh theo đầu ra.

Các thiết lập DSP đã lưu vẫn được giữ khi đường USB nguyên bản bỏ qua xử lý.

### DSF / DFF

Trong Âm thanh & đầu ra, chọn **Đầu ra DSD USB → Native DSD / DoP / Tự động Native / DoP**. Đường DSD nguyên bản dành cho USB DAC tương thích, không tự chuyển ngầm DSD thành PCM để phát qua loa máy hoặc Bluetooth.

Cả **DAC lẫn driver ROM/kernel** phải hỗ trợ cách truyền đang yêu cầu. DoP còn cần đúng tần số PCM làm lớp mang: DSD64 dùng 176,4 kHz; DSD128 dùng 352,8 kHz; DSD256 dùng 705,6 kHz. DAC ghi hỗ trợ Native DSD chưa đồng nghĩa mọi mức DoP đều dùng được.

DAC có thể khai báo tên USB của OEM/bộ điều khiển. Mục **Tên hiển thị DAC** cho phép lưu tên dễ nhận biết thay vì coi USB descriptor là tên thương mại chính xác. Xem thông tin nguồn, cách truyền và phần cứng trong trang đầu ra hoặc log thiết bị.

## Lời bài hát, dịch lời và tô từng chữ

### Đọc lời

NXT có thể dùng `.lrc` cùng tên đặt cạnh bài trên thẻ, lời nhúng trong file, lời đã lưu và các nguồn tìm lời online. Khi thiếu lời gốc, app có thể tìm lời trước rồi mới dịch; không bảo đảm mọi bản thu đều có kết quả khớp.

- Xem lời ngay trên Now Playing hoặc nhấn **Y** để mở full màn hình.
- Chọn hiển thị dịch **Tắt / Bản dịch / Song ngữ**.
- Song ngữ giữ lời gốc và bản dịch cạnh nhau ở cả hai màn hình. Bản dịch dùng chữ nghiêng để phân biệt với lời gốc.
- Bám câu đang hát, cuộn tay, đổi cỡ chữ và điều chỉnh độ trễ lời.
- Chỉnh riêng màu lời đang hát/chưa hát/bản dịch, chữ đậm, viền và bóng. Auralis còn có màu/cỡ thông tin bài và độ sáng bìa riêng cho Now Playing/full lời.
- Chọn 10 thiết lập chuyển lời, gồm Tắt. Phổ nhạc không vẽ chồng vào vùng full lời.

### Dịch vụ dịch và cache offline

| Dịch vụ | Thiết lập trong app |
| --- | --- |
| MyMemory | Không nhập API key trong NXT; vẫn có giới hạn dịch vụ. |
| Groq | Khóa API riêng và model chọn trong app; nhập trên máy hoặc qua TXT. |
| Google Cloud | Khóa/tài khoản Cloud Translation riêng, quyền truy cập và chi phí riêng. |

Ngôn ngữ đích đi theo **ngôn ngữ app**. Có thể chọn lại ngôn ngữ lời gốc khi tự nhận diện chưa đúng. Dịch chạy ở worker nền, ưu tiên các câu cần dùng và tái sử dụng kết quả đã lưu. Cache lời dịch trên thẻ giới hạn **256 MiB**, dùng lại các câu đã dịch khi offline; đây là hạn mức lưu trữ, không phải nạp 256 MiB vào RAM hay bảo đảm mọi bài đều đã được dịch. Các bản lưu cũ có thể được dọn khi đạt hạn mức cache.

Dịch chữ gửi nội dung lời khi tính năng được bật. **Phân tích mốc hát là tính năng riêng, chỉ gửi âm thanh sau khi bạn đồng ý.** Hạn mức, quyền dùng model, chất lượng và chi phí nếu có phụ thuộc dịch vụ/tài khoản. HTTP 429 là giới hạn tốc độ/hạn mức; app chờ thử lại, không vượt giới hạn. Khi ẩn app hoặc tắt màn hình, yêu cầu dịch còn chờ được tạm dừng để giảm việc chạy nền.

### Tô từng chữ theo nhạc

App phân biệt ba nguồn mốc thời gian:

1. **Mốc chữ thật:** Enhanced LRC có thời điểm từng chữ, dùng offline mà không cần khóa dịch vụ dịch.
2. **Phân tích giọng hát Groq tùy chọn:** Dùng khóa Groq và Whisper Large v3 để phân tích file local được hỗ trợ, tối đa **10 phút**, sau khi xác nhận gửi âm thanh. Tự phân tích mặc định tắt. Kết quả có cache riêng dùng offline, trong giới hạn **16 MiB / 256 file mốc chữ**.
3. **Ước lượng offline:** Tùy chọn ước lượng theo mốc câu và độ dài chữ, có nhãn ước lượng. Cách này không nhận diện giọng ca sĩ.

Tắt ước lượng nếu chỉ muốn tô chữ khi có mốc gốc hoặc mốc tạo ra đã ghép khớp. Groq có thể nhận sai tiếng hát, bè, phát âm hoặc bản thu khác; tăng/giảm độ trễ chung không sửa được từng mốc chữ sai. App không ghi lại luồng online được bảo vệ hoặc đường DSD raw để phân tích.

## Cá nhân hóa theo ý bạn

Mở **Cá nhân → Hồ sơ → Giao diện cá nhân**. Theme, LED và phổ nhạc nằm ở đây; các thiết lập thiết bị/âm thanh nằm trong Cài đặt.

### Theme và Auralis

- **Bộ sưu tập giao diện:** 51 bố cục, gồm Original NXT, cassette/radio/đĩa nhạc, các bảng màu hiện đại và Auralis.
- **Tùy chọn giao diện gốc:** theo hệ thống/sáng/tối, bảng màu, RGB/HEX riêng, font, cỡ chữ giao diện/lời, kho hình nền, lớp phủ, độ đậm ô menu, ảnh bìa/đĩa than, chuyển động, icon menu, độ trễ và kiểu lời.
- **Auralis:** bìa album phủ toàn màn hình phía sau lời, nút glass, các cặp lời song ngữ và màn hình full lời riêng. Mục tùy chỉnh riêng **chỉ hiện khi đang chọn Auralis**.
- Chỉnh độc lập độ sáng bìa Now Playing/full lời của Auralis; dùng font tham chiếu hoặc bộ font đã chọn.
- **Tinh chỉnh v1.9.7.7:** dựng lại 316 icon có sẵn từ vector, làm mịn góc bo/đường chéo, tăng nhẹ nút Auralis, nền nút bật rõ hơn, lọc ảnh khi đổi cỡ và sửa vùng thông tin của Ocean Blue/Boombox.

Ảnh bìa giữ chi tiết nguồn và tỉ lệ. Phóng lớn ảnh nhỏ không tạo thêm chi tiết HD thật.

<details>
<summary>Danh sách đủ 51 theme</summary>

|  |  |  |
| --- | --- | --- |
| 1. Original NXT | 2. Cassette C60 | 3. Vintage Radio |
| 4. Walkman | 5. Reel to Reel | 6. Hi-Fi Rack |
| 7. CD Deck | 8. Jukebox | 9. CRT Terminal |
| 10. Turntable | 11. Modern Focus | 12. Modern Ambient |
| 13. Boombox | 14. MiniDisc | 15. Pocket Wheel |
| 16. Classic 90s | 17. Tube Amplifier | 18. Gramophone |
| 19. Car Audio | 20. Synth Studio | 21. Oscilloscope |
| 22. Bento | 23. E-Ink | 24. Hologram |
| 25. Nordic Studio | 26. Night Drive | 27. Jazz Lounge |
| 28. Soft Paper | 29. Aurora | 30. Ocean Blue |
| 31. Lo-fi Desk | 32. Film Gallery | 33. Studio Meter |
| 34. Mono Slate | 35. Rose Quartz | 36. Gallery Noir |
| 37. Forest Dial | 38. Coral Tape | 39. Midnight Indigo |
| 40. Copper Sunset | 41. Sakura Glass | 42. Cobalt Grid |
| 43. Desert Radio | 44. Emerald Studio | 45. Lavender Paper |
| 46. Ice Blue | 47. Amber Console | 48. Ruby Velvet |
| 49. Graphite Split | 50. Olive Notebook | 51. Auralis Themes |

</details>

### Font, hình nền, avatar và xem trước

- **60 lựa chọn font**, gồm MiSans, HarmonyOS Sans, OPPO Sans, Inter, Roboto, Be Vietnam Pro, Geist, Public Sans và nhiều font serif/monospace.
- Font dự phòng offline hỗ trợ tiếng Việt và các hệ chữ CJK/Thái tương ứng. Giữ tên và giấy phép của font bên thứ ba; không đóng gói Apple SF Pro.
- Hình nền dựng sẵn, nhập JPG/PNG từ thẻ và kho hình nền online có bản lưu; chỉnh lớp phủ/độ đậm menu cho dễ đọc. Các tùy chọn áp dụng khác nhau giữa nền Original và bố cục theme cố định.
- **300 biểu tượng để chọn, 180 emoji 3D**, avatar từ ảnh/chữ và đổi icon từng mục menu.
- Trình chỉnh có xem trước trực tiếp màu/chữ/hiệu ứng trên bìa đang dùng; đổi qua lại Now Playing/full lời, sau đó **Áp dụng hoặc Hủy**. Không liên tục lưu cấu hình khi đang kéo/chỉnh bản nháp.

| Nút trong xem trước | Thao tác |
| --- | --- |
| D-pad | Chọn thuộc tính và đổi giá trị. |
| Y ở thuộc tính màu | Vào/ra chỉnh RGB. |
| L1 / R1 khi chỉnh RGB | Đổi thành phần màu bước lớn hơn. |
| X | Đổi xem trước Now Playing / full lời. |
| SELECT | Chạy lại hiệu ứng mẫu. |
| A / B | Áp dụng / hủy. |

Lời mẫu được ghi rõ là xem trước; không thay lời thật của bài đang nghe hoặc phát sinh yêu cầu dịch.

<details>
<summary>Danh sách đủ 60 font</summary>

|  |  |  |
| --- | --- | --- |
| 1. Noto Sans | 2. Inter | 3. Roboto |
| 4. Source Sans 3 | 5. Nunito Sans | 6. Manrope |
| 7. Rubik | 8. Ubuntu | 9. Lato |
| 10. Montserrat | 11. Noto Serif | 12. Source Serif 4 |
| 13. Lora | 14. Merriweather | 15. JetBrains Mono |
| 16. Fira Code | 17. MiSans · Xiaomi / NX Redux | 18. HarmonyOS Sans · Huawei |
| 19. HarmonyOS Sans Condensed · Huawei | 20. OPPO Sans 4.0 · OPPO | 21. Roboto Condensed · Google |
| 22. Roboto Slab · Google | 23. Be Vietnam Pro | 24. Plus Jakarta Sans |
| 25. Work Sans | 26. Mulish | 27. Public Sans |
| 28. IBM Plex Sans | 29. Lexend | 30. Onest |
| 31. Geist | 32. Wix Madefor Text | 33. Readex Pro |
| 34. Barlow | 35. Asap | 36. Archivo |
| 37. Exo 2 | 38. Chakra Petch | 39. Prompt |
| 40. Kanit | 41. Sarabun | 42. Quicksand |
| 43. Comfortaa | 44. Space Grotesk | 45. Cabin |
| 46. Open Sans | 47. Arimo | 48. Josefin Sans |
| 49. Bitter | 50. Playfair Display | 51. Literata |
| 52. Vollkorn | 53. Cormorant Garamond | 54. Alegreya |
| 55. Crimson Pro | 56. Fraunces | 57. Spectral |
| 58. Gelasio | 59. Epilogue | 60. Newsreader |

</details>

### Phổ nhạc và LED

**Phổ nhạc:** 21 bảng màu, hai màu RGB tùy chỉnh và 5 kiểu: Solid, Gradient, 7-color Rainbow, VU, Multicolor Mix. VU là kiểu hiển thị mức tín hiệu, đổi màu theo độ lớn. Bật/đổi bảng màu từ trình phát hoặc chỉnh trong Giao diện cá nhân.

**LED:** bật/tắt, độ sáng, màu cố định hoặc theo nhạc, màu sẵn/RGB tự chọn và 6 hiệu ứng: Một màu, Gradient hai màu, Sóng cầu vồng, Chia hai màu, Cực quang, Nhịp thở. Thay đổi áp dụng mà không cần tắt/bật LED thêm lần nữa khi giao tiếp ROM/phần cứng hỗ trợ.

### Ngôn ngữ và bàn phím

Ngôn ngữ mở lần đầu là **tiếng Anh**. Có tiếng Anh, Việt, Pháp, Nga, Hàn, Nhật, Thái, Trung giản thể, Trung phồn thể, Đức, Tây Ban Nha, Bồ Đào Nha Brazil, Ý và Indonesia.

Chọn tại **Cài đặt → Ngôn ngữ**. Bàn phím trong app đi theo ngôn ngữ/layout đã chọn, có Telex tiếng Việt, ghép chữ Hàn, nhập tiếng Nhật và gợi ý chữ Nhật/Trung offline ở các layout tương ứng. Thiết lập này dành cho bàn phím NXT, không đổi bàn phím toàn ROM.

## Hồ sơ, gợi ý và thống kê

- Tạo, đổi tên, chuyển và xóa tối đa **16 hồ sơ người nghe cục bộ**, có xác nhận trước khi xóa. Xóa hồ sơ sẽ xóa dữ liệu của hồ sơ đó, không xóa file nhạc trên thẻ.
- Chỉnh biệt danh, avatar, sinh nhật và giao diện; chọn thể loại, tâm trạng, quốc gia ở Cá nhân, không bắt trả lời check-in mỗi lần vào app.
- Giữ lịch sử, yêu thích, giao diện và tiến trình nghe theo hồ sơ. Có thể tắt ghi lịch sử.
- Khám phá **Nghe tiếp**, **Những bài quen thuộc**, **Chưa nghe**, **Lâu chưa nghe** và nhạc/playlist Spotify, SoundCloud gợi ý theo sở thích, tùy quyền truy cập nguồn.
- Xem thống kê **Hôm nay / 7 ngày / Tháng này**: thời gian nghe, lượt phát, nghệ sĩ, bài/nghệ sĩ nghe nhiều và biểu đồ theo ngày. Lịch sử có phân trang và mở lại bài đã nghe.
- Thống kê hiện áp dụng cho các phiên Local/Spotify/SoundCloud/Drive được hỗ trợ; radio và podcast không tính vào tổng lịch sử của hồ sơ này.

## Các nút điều khiển mặc định

Đây là gán phím mặc định trong **Now Playing**; có thể đổi 16 nút ở **Cài đặt → Cài đặt phím**. Menu và màn hình full lời có hướng dẫn phím theo ngữ cảnh riêng.

| Nút | Chức năng mặc định |
| --- | --- |
| A | Phát / tạm dừng. |
| B | Về menu. |
| X | Hàng đợi / danh sách bài. |
| Y | Full lời. |
| L1 / R1 | Bài trước / bài sau. |
| L2 | Đổi bảng màu/trạng thái hiển thị phổ nhạc. |
| R2 | Yêu thích bài hiện tại. |
| Nhấn ngắn SELECT | Mở/đóng EQ. |
| START | Tùy chọn nhanh: đầu ra, chế độ âm thanh, hẹn giờ, ngẫu nhiên, lặp, thông tin bài/đầu ra… |
| Trái / phải | Tua lùi / tới. |
| SELECT + lên/xuống | Âm lượng máy. |
| SELECT + trái/phải | Độ sáng màn hình. |
| Giữ SELECT | Khóa / mở khóa điều khiển. |
| L3 / R3 | Mặc định không gán; là thao tác nhấn xuống cần analog. |

Nút logo phía trước là phím thoát app. Home/task manager bên hông thuộc ROM; bản NX Redux có tích hợp dịch vụ nhạc của ROM trên firmware được hỗ trợ. Chọn **Phát nền** nếu muốn rời màn hình app mà nhạc tiếp tục.

Trong full lời: lên/xuống cuộn, Y đổi bám lời, X đổi cỡ, L1/R1 chỉnh lệch lời, START mở dịch/mốc chữ, A phát/dừng, B quay lại.

## Thiết lập các nguồn online

### Spotify Premium — file đăng nhập PC và xác thực QR

Mở **Nhạc online → Spotify**. Cần Spotify Premium và xác thực quyền phát hợp lệ. Tìm kiếm, playlist tài khoản, bài đã thích và album đã lưu còn phụ thuộc quyền Web API; app kiểm tra quyền thư viện và quyền phát riêng.

Quy trình đầy đủ dưới đây dùng **nhập file PC + QR**, gồm trường hợp chỉ quét QR thì tài khoản vẫn hiện chưa xác thực:

1. Trên PC, cài Python 3 và mở thư mục `Spotify_Login_PC` đi kèm. Windows chạy `Dang_nhap_Spotify.bat`, hoặc dùng lệnh ngay trong thư mục đó:

   ```bash
   python Spotify_Login.py
   ```

2. Đăng nhập và cấp quyền trong trình duyệt. Công cụ xuất **`spotify-credentials.json`** vào thư mục chạy/nơi xuất đã chọn. Nếu dùng ứng dụng Spotify OAuth riêng đã được cấp quyền, công cụ nhận `--client-id`; callback mặc định là `http://127.0.0.1:8080/callback`.
3. Chép JSON **cạnh `launch.sh` trong thư mục app đúng ROM**, theo bảng cài đặt ở trên.
4. Trên Brick, vào **Spotify → Tài khoản / Tài khoản & công cụ → Nâng cao · nhập đăng nhập PC cũ**.
5. Chọn **Đăng nhập / đổi tài khoản bằng QR**, quét mã hoặc nhập mã đang hiện tại **https://spotify.com/pair**; xác nhận bằng **đúng tài khoản Spotify Premium đã đăng nhập trên PC**.
6. Chờ app hoàn tất xác thực quyền phát rồi chọn nhạc. Phiên đăng nhập hợp lệ được lưu cho lần sử dụng sau.

Một số thiết lập được cấp quyền có thể hoàn tất trực tiếp bằng QR. Nếu chỉ thư viện bị giới hạn, **Phát link bài hát Spotify** có thể dùng phiên đã có quyền phát; cách này không vượt qua việc từ chối xác thực phát nhạc hoặc yêu cầu Premium. Khi lỗi, xem `NXT_Spotify_Log.txt`. Quyền ứng dụng OAuth, scope và chính sách Spotify vẫn được áp dụng.

### Groq — nhập khóa dịch mà không gõ tay trên máy

1. Tạo API key trong tài khoản của bạn tại **https://console.groq.com/keys**.
2. Trên PC, tạo **`groq-api-key.txt`**, ghi một khóa đầy đủ trên một dòng:

   ```text
   gsk_REPLACE_WITH_YOUR_COMPLETE_KEY
   ```

3. Đặt file ở **`Music/groq-api-key.txt` tại gốc thẻ**, hoặc cạnh `launch.sh` của app.
4. Mở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát**, hoặc mở tùy chọn dịch bằng START từ full lời.
5. Chọn dịch vụ **Groq**, vào **Khóa API Groq → Nhập từ TXT**, chọn model và chế độ Song ngữ/Bản dịch.

File dạng `GROQ_API_KEY=...` cũng được chấp nhận. Menu model đi kèm v1.9.7.7 có `openai/gpt-oss-20b`, `openai/gpt-oss-120b` và `qwen/qwen3.8-27b`; việc tài khoản dùng được ID nào do phía dịch vụ quyết định. Khi không có quyền/model không khả dụng, thử model khác trong app hoặc đổi dịch vụ.

Khóa này cũng dùng được cho tính năng phân tích mốc hát riêng, nhưng app phải hỏi đồng ý gửi âm thanh và dùng hạn mức âm thanh của dịch vụ. Giữ riêng các khóa và JSON đăng nhập, không đưa vào repository công khai.

### Google Drive — thư mục và tệp nhạc công khai

1. Chia sẻ tệp/thư mục muốn nghe để người có link đọc được công khai.
2. Vào **Nhạc online → Google Drive → Thêm link Google Drive**, nhập link chia sẻ.
3. Hoặc nhập file `.txt`/`.url`, mỗi dòng một link Drive.
4. Mở link đã lưu, duyệt thư mục con, chọn bài hoặc **Nghe cả thư mục** để tạo hàng đợi nhạc trong thư mục.

App stream file được hỗ trợ, không bắt tải xong toàn bộ để nghe; vẫn có bộ đệm mạng. Link riêng tư/bắt đăng nhập hoặc giới hạn tải của Drive có thể không truy cập được. Có thể làm mới hoặc xóa link đã lưu trong app.

### SoundCloud, radio, podcast và công cụ tải

- **SoundCloud:** tìm bài/playlist công khai và nhạc gợi ý có thể phát, không cần nhập API key SoundCloud. Các hạn chế với bài riêng tư, preview hoặc thuê bao vẫn còn hiệu lực. Giữ bản sửa nhạc công khai từ v1.9.7.6.
- **Radio:** duyệt và nghe đài internet qua module radio.
- **Podcast:** duyệt đăng ký và các tập qua module podcast.
- **Công cụ tải âm thanh YouTube:** source này còn có module tìm/tải/hàng đợi riêng dùng yt-dlp. Đây không phải tích hợp thư viện tài khoản YouTube Music; khả năng hoạt động phụ thuộc helper và nguồn. Sử dụng nội dung được phép tải.

## Hiệu năng, pin và thao tác khi tắt màn hình

Mở **Cài đặt → Hiệu năng thiết bị** để chọn **Tiết kiệm pin / Cân bằng / Hiệu năng** và xem thông số máy.

Các chế độ điều chỉnh khối lượng hiệu ứng. NextUI và NX Redux còn thử áp dụng chính sách CPUFreq có thể khôi phục khi ROM cho phép; Stock dùng chính sách CPU của ROM. Tên chế độ không bảo đảm ROM đã nhận mức giới hạn CPU. Dòng trạng thái cho biết phần đã áp dụng; khi rời phiên, app khôi phục những thiết lập CPU mà nó vẫn quản lý.

Màn hình theo dõi đọc tải/xung CPU, nhiệt độ, RAM và pin từ thông tin phần cứng sẵn có. Lịch sử pin là **mức giảm phần trăm của cả máy trong phiên NXT**, bỏ qua lúc sạc/khoảng ngắt; không phải phép đo riêng điện năng app.

- Đặt hẹn giờ trong tùy chọn nhanh hoặc Cách phát. Hết giờ, NXT lưu tiến trình/cài đặt, dừng nhạc và đóng app, kể cả phiên phát nền. ROM quyết định khi nào máy ngủ; app không bảo đảm deep sleep ngay lập tức.
- Có tự tắt màn hình và phát nhạc dưới nền.
- Bật **Cài đặt → Phát nhạc & âm thanh → Cách phát → Điều khiển khi tắt màn hình** để bấm các nút đã gán mà màn hình vẫn tối. Nút nguồn bật sáng lại; giữ SELECT vẫn khóa/mở khóa được. Mặc định tùy chọn này tắt.
- Cache ảnh/chữ/icon có giới hạn, chỉ mục nhạc tăng dần và giảm việc chạy khi ẩn app giúp hạn chế xử lý không cần thiết. Chuyển động đang hoạt động có thể nhắm nhịp khoảng 60 Hz, thay vì vẽ lại liên tục mọi trang đứng yên.

Độ mượt, nhiệt độ và pin thực tế phụ thuộc ROM, đầu ra, công việc mạng, hiệu ứng và nội dung. v1.9.7.7 có kiểm tra phần mềm/render/build/OTA; **không công bố cam kết FPS hoặc nhiệt độ đã đo trên máy thật**.

## Chuyển file qua Wi-Fi

Mở **Cài đặt → Thư viện & tệp → Chuyển file qua Wi-Fi**. Điện thoại/PC cùng mạng Wi-Fi tin cậy, mở **đúng địa chỉ app hiển thị**, nhập PIN 6 chữ số rồi chọn các file tải lên thư mục nhạc đã cấu hình.

Máy chủ chỉ chạy khi cần. Màn hình có tiến trình; X hủy file, B dừng chuyển. File chép xong được đưa vào lượt cập nhật thư viện. Bản này có chép file Wi-Fi; USB-C dùng cho phần cứng âm thanh được hỗ trợ, không có chế độ chép file USB riêng của app.

## Cập nhật OTA

Mỗi bộ cài có sẵn nguồn cập nhật chính thức đúng ROM. Vào:

**Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm → Kiểm tra cập nhật**

Người dùng thông thường không phải nhập URL nguồn. Nếu trước đây đã chọn nguồn tùy chỉnh, dùng **Khôi phục nguồn nhà phát hành** để về nguồn đi kèm.

App kiểm tra định danh, ROM/máy, phiên bản, dung lượng, SHA-256 và nội dung ZIP trước khi cài; chuẩn bị bản mới, giữ bản app cũ dự phòng và dữ liệu cá nhân ở ngoài thư mục bị thay. Cần đủ dung lượng thẻ cho ZIP, phần giải nén và bản dự phòng.

**Dành cho người phát hành:** mỗi `NXT_OTA_Google_Drive_v1.9.7.7_<ROM>.zip` chứa đúng bộ cài và trang `CHUAN_BI_OTA.html` tạo `update.json`. Đăng ZIP trước, tạo JSON từ đúng ZIP đó rồi cập nhật nội dung **chính file manifest Drive đã dùng cho ROM đó**, giữ nguyên ID/link. Bộ ZIP không tự đăng OTA. Stock, NextUI và NX Redux phải có nguồn riêng.

## Build từ mã nguồn

Nhánh mặc định là trang giới thiệu gọn. Để build, tải `_Source.zip` đầy đủ trong [Releases](https://github.com/SpringThong/NXT-Retro-Music/releases/tag/v1.9.7.7), hoặc dùng [nhánh source-v1.9.7.7](https://github.com/SpringThong/NXT-Retro-Music/tree/source-v1.9.7.7) được giữ riêng. Chạy các lệnh dưới đây từ thư mục gốc của source đã giải nén.

Ứng dụng chạy native bằng **C/SDL**, có helper Spotify/SoundCloud viết bằng Rust và tài nguyên/thư viện đi kèm. Một cây source tạo ra ba frontend/bộ cài riêng theo ROM.

Bắt đầu từ **source đầy đủ**, giữ thư viện, tài nguyên, vendor source và helper binary cần thiết. Chỉ tải `upstream/src` lên web chưa tạo thành môi trường build hoàn chỉnh. Cross-build frontend cần **Linux x86_64, Python 3.11+**, các tiện ích hệ thống thông thường và mạng để lấy SDK ghim phiên bản ở lần đầu. Công cụ tải/kiểm tra compiler và sysroot tương ứng; không tự thay bằng ARM compiler bất kỳ trong máy.

```bash
# Kiểm tra đầu vào và metadata ba ROM.
python3 tools/build-release.py --rom all --check

# Build bộ cài và bộ chuẩn bị OTA cho cả ba ROM.
python3 tools/build-release.py --rom all --jobs 4

# Hoặc chỉ chọn một ROM.
python3 tools/build-release.py --rom nxredux --jobs 4

# Chỉ biên dịch frontend, chưa đóng gói.
python3 tools/build-release.py --rom all --jobs 4 --build-only
```

Kết quả ở `dist/stock-release/`, `dist/nextui-release/`, `dist/nxredux-release/`. Lệnh build frontend dùng lại helper executable có sẵn; nếu sửa helper cần dùng quy trình/toolchain Rust riêng cho helper đó.

| Vị trí source | Nội dung |
| --- | --- |
| `upstream/src/` | App native dùng chung, trình phát, âm thanh, lời, hồ sơ và UI. |
| `upstream/res/`, `upstream/licenses/` | Ảnh/font/catalog và giấy phép bên thứ ba. |
| `stock/`, `nxredux/`, `upstream/` | Khởi chạy, cấu hình và payload theo ROM. |
| `spotify/`, `soundcloud/` | Source helper online và các thành phần vendor. |
| `relink/`, `sources/` | Thư viện build, source archive và đầu vào ghim phiên bản. |
| `tools/`, `qa/` | Công cụ build/đóng gói/OTA và kiểm tra phần mềm. |
| `Spotify_Login_PC/` | Công cụ OAuth trên PC nếu cần. |
| `SOURCE.json`, `RELEASE_QA_v1.9.7.7.md` | Nguồn gốc snapshot, kết quả kiểm tra và giới hạn xác nhận. |

Khi đưa lên GitHub, giữ source phát triển dễ đọc, còn ZIP cài/OTA/source đầy đủ nên đưa vào **Releases** thay vì commit các gói phát hành. Giữ đủ tài nguyên hoặc ghi rõ cách khôi phục snapshot đầy đủ để người khác build. Xem thêm [ghi chú đăng GitHub](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/docs/GITHUB_PUBLISH.vi.md).

## Xử lý lỗi và góp ý

Khi báo lỗi, ghi phiên bản app, ROM/phiên bản ROM, nguồn nhạc, bước tái hiện, định dạng file và đầu ra/DAC đang cắm. Gửi log liên quan sau khi xem bỏ thông tin cá nhân; không đính kèm API key hoặc dữ liệu đăng nhập.

| Hiện tượng | Nên kiểm tra |
| --- | --- |
| Spotify vẫn chưa xác thực | Hoàn tất nhập PC và xác thực phát QR cùng tài khoản; xem log Spotify. |
| Dịch báo HTTP 429 | Chờ hạn mức dịch vụ phục hồi hoặc chọn nguồn khác đã cấu hình. |
| Bài mới không có bản dịch khi offline | Cần có lời gốc và dịch online trước; offline chỉ dùng lại kết quả đã lưu. |
| Tô chữ lệch giọng hát | Kiểm tra đúng bản thu, độ trễ và mốc chữ là thật, tạo bằng AI hay ước lượng. |
| Native DSD không chạy | Kiểm tra cách truyền/tần số DAC và driver ROM; xem log Native/DoP. |
| Bluetooth kết nối nhưng phát ở nơi khác | Kết nối trong ROM và kiểm tra đầu ra đang hoạt động trong NXT. |
| OTA chưa có bản mới | So phiên bản đã cài với manifest đã đăng; manifest phải mô tả đúng ZIP mới hơn của ROM đó. |

Trong **Thông tin ứng dụng**, START xuất `NXT_App_Log.txt`, Y xuất `NXT_Device_Log.txt`. Có log nguồn như `NXT_Spotify_Log.txt`, `NXT_SoundCloud_Log.txt` để chẩn đoán riêng. Log app/thiết bị xuất cạnh launcher; log launcher/phím có sẵn tùy ROM.

## Ghi công và giấy phép

- **Nguyễn Xuân Thông — Spring Thông:** ý tưởng, định hướng thực hiện, cá nhân hóa và phát triển dự án theo cách vibe coding.
- **Mohammad Syuhada** và **nborodikhin / NextUI Music Player:** phần trình phát gốc. Repository upstream được ghi trong snapshot: https://github.com/nborodikhin/nextui-music-player.
- **NextUI, NX Redux và các dự án cộng đồng TrimUI:** nền tảng firmware và tích hợp.
- **SDL, FFmpeg, librespot, yt-dlp, libsamplerate, SQLite, FreeType, HarfBuzz, FriBidi, utf8proc**, tác giả icon/font và các thành phần phụ thuộc khác.

Các file mở rộng NXT có thông báo **`GPL-3.0-only`**; xem [văn bản GPL của NXT](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/licenses/NXT-GPL-3.0.txt). Trình phát gốc giữ [giấy phép MIT](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/LICENSE); NextUI và các thành phần bên thứ ba giữ thông báo riêng. Không coi mọi file trong gói là cùng một giấy phép. Khi phân phối, giữ [thư mục giấy phép bên thứ ba](https://github.com/SpringThong/NXT-Retro-Music/blob/v1.9.7.7/upstream/licenses/), gồm cả giấy phép font và icon.

Đây là dự án cộng đồng độc lập, không phải ứng dụng chính thức của TrimUI, Spotify, SoundCloud, Google hay Groq. Các tên trên chỉ các sản phẩm/dịch vụ tương ứng.

## Ủng hộ dự án

Nếu thích NXT Retro Music, bạn có thể chia sẻ video trải nghiệm, góp lỗi có bước tái hiện, hỗ trợ bản dịch hoặc bấm Star cho repository.

Ủng hộ tùy tâm qua **PayPal: `thongkhung2@gmail.com`**. Việc ủng hộ là tự nguyện, không mở hạn mức dịch vụ đám mây hay quyền tài khoản trả phí.

Làm cho một chiếc máy nhỏ, theo cách mình thích nghe nhạc. — **Spring Thông**
