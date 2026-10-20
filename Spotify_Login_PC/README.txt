NXT Retro Music v1.3 Stock — Spotify PC login

Normal login: open Online music > Spotify > Log in by QR on the Brick. Scan the QR and approve at https://spotify.com/pair with a Spotify Premium account. The app then verifies playback authentication separately from Web API library access. The saved session is reused on the next launch.

For an existing authorized Web API login, run Dang_nhap_Spotify.bat on Windows with Python 3. Copy spotify-credentials.json into Apps/NXT_Retro_Music beside launch.sh. On the Brick choose Account > Advanced: import existing PC login. The app keeps the original file and makes a separate copy in its userdata directory, requesting Unix mode 0600 where supported. FAT/exFAT uses mount permissions; keep the SD card and original backup private. A QR login and imported PC login must belong to the same account; the app checks this before using the PC library login.

The PC tool accepts --client-id for your own registered, authorized OAuth application. This does not grant permissions or lift Spotify quotas; the account, application, callback and scopes must be allowed by Spotify. Callback: http://127.0.0.1:8080/callback

If the library API is restricted but playback authentication succeeds, use Play Spotify track link with https://open.spotify.com/track/… or spotify:track:… . The player obtains track metadata through its authenticated playback session. Playlists, search and liked-song lists still require Web API access. A rejected playback authentication cannot be bypassed with a link.

Credentials contain private tokens: keep the original as a private backup, never share it or include it when publishing the app. Logout removes the app's saved session but keeps the original PC file. For diagnosis share NXT_Spotify_Log.txt only, not the credentials JSON.
