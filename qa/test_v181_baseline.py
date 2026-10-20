"""Confirm the v1.8 audio/platform cores and account helpers are unchanged."""
from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1]
paths=['upstream/src/'+n for n in ['player.c','module_player.c','nxt_decoder.c','nxt_player_core.c','nxt_dsp.c','nxt_config.c','nxt_hardware.c','nxt_pcm.c','nxt_led.c','nxt_nextui.c','nxt_session.c','nxt_spotify.c','nxt_soundcloud.c','nxt_transfer.c','nxt_spotify_pcm.inc','nxt_soundcloud_pcm.inc','nxt_ime.c','keyboard.c']]
paths+=['upstream/bin/tg5040/'+n for n in ['nxt-spotify-backend','nxt-soundcloud-backend','yt-dlp','yt-dlp-runtime/yt-dlp']]
report={}
with zipfile.ZipFile(R.parent/'v181-input/NXT_Retro_Music_v1.8_Source.zip') as z:
 for name in paths:
  before=z.read('NXT_Retro_Music_v1.8_Source/'+name);after=(R/name).read_bytes();assert before==after,name
  report[name]=hashlib.sha256(after).hexdigest()
(R/'logs/v181-baseline-preserved.json').write_text(json.dumps({'exit':0,'baseline':'1.8','unchanged_files':report,'physical_device_tested':False},indent=2)+'\n')
print('PASS unchanged v1.8 decoder/audio/DSP, platform/DAC/LED, online-account backends, profile configuration, background session, Wi-Fi worker and multilingual keyboard cores.')
