@echo off
cd /d "%~dp0"
where py >nul 2>&1
if %errorlevel%==0 (
  py -3 Spotify_Login.py
) else (
  python Spotify_Login.py
)
pause
