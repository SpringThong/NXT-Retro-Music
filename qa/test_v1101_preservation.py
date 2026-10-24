"""Compare release inputs to v1.10; only Back handling and Spotify auth may change."""
from pathlib import Path
import hashlib,json,zipfile,argparse
R=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--baseline',type=Path,required=True);args=a.parse_args()
allowed={'upstream/src/nxt.h','upstream/src/nxt_session.c','upstream/src/nxt_startup.c','upstream/src/nxt_spotify.c','upstream/src/nxt_soundcloud.c','upstream/src/module_player.c'}
sha=lambda b:hashlib.sha256(b).hexdigest()
resources=count=0;changed=[]
with zipfile.ZipFile(args.baseline) as z:
 prefix='NXT_Retro_Music_v1.10_Source/'
 for name in z.namelist():
  if name.endswith('/') or not name.startswith(prefix):continue
  relative=name[len(prefix):];p=R/relative
  if relative.startswith('upstream/res/'):
   assert p.is_file() and sha(p.read_bytes())==sha(z.read(name)),relative;resources+=1
  if relative.startswith(('upstream/src/','upstream/NextUI/workspace/','nxredux/src/')) and p.suffix in {'.c','.h','.inc'}:
   assert p.is_file(),relative;count+=1
   if sha(p.read_bytes())!=sha(z.read(name)):
    assert relative in allowed,relative;changed.append(relative)
 for name in ['nxt-soundcloud-backend','yt-dlp','yt-dlp-runtime/yt-dlp']:
  relative='upstream/bin/tg5040/'+name;assert sha((R/relative).read_bytes())==sha(z.read(prefix+relative)),relative
 assert json.loads(z.read(prefix+'SOURCE.json'))['official_OTA_sources']==json.loads((R/'SOURCE.json').read_text())['official_OTA_sources']
 for relative in ['spotify/backend/src/catalog.rs','spotify/backend/src/auth.rs','spotify/backend/src/pair.rs','upstream/src/nxt_player_core.c','upstream/src/nxt_online_pcm_queue.h','upstream/src/nxt_online_ramp.h','upstream/src/nxt_spotify_pcm.inc','upstream/src/nxt_soundcloud_pcm.inc']:
  assert sha((R/relative).read_bytes())==sha(z.read(prefix+relative)),relative
report={'version':'1.10.1','baseline':'1.10','exit':0,'source_files_compared':count,'resource_files_preserved':resources,'changed_C_H_INC_files':changed,'themes_fonts_artwork_unchanged':True,'online_audio_buffering_unchanged':True,'DSD_DSP_translation_cache_CPU_LED_library_OTA_code_unchanged':True,'soundcloud_ytdlp_runtime_unchanged':True,'original_three_OTA_feed_urls_unchanged':True,'hardware_tested':False}
(R/'logs/v1101-preservation.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS preservation against v1.10:',count,'source files,',resources,'assets, online buffering and all official OTA URLs')
