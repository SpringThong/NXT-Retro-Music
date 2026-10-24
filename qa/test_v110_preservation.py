from pathlib import Path
import hashlib,json,zipfile,argparse
R=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--baseline',type=Path,required=True);args=a.parse_args()
allowed={'nxt.h','nxt_screens.c','nxt_startup.c','nxt_spotify.c','nxt_spotify_pcm.inc','nxt_soundcloud.c','nxt_soundcloud_pcm.inc','module_menu.c','module_player.c','nxt_player_core.c','nxt_session.c'}
sha=lambda b:hashlib.sha256(b).hexdigest()
checked=resources=0;changed=[]
with zipfile.ZipFile(args.baseline) as z:
 prefix='NXT_Retro_Music_v1.9.7.7_Source/'
 for n in z.namelist():
  if n.endswith('/') or not n.startswith(prefix):continue
  rel=n[len(prefix):];p=R/rel
  if rel.startswith('upstream/res/'):
   assert p.is_file() and sha(p.read_bytes())==sha(z.read(n)),rel;resources+=1
  if rel.startswith(('upstream/src/','upstream/NextUI/workspace/','nxredux/src/')) and p.suffix in {'.c','.h','.inc'}:
   assert p.is_file(),rel;checked+=1
   if sha(p.read_bytes())!=sha(z.read(n)):
    assert rel.startswith('upstream/src/') and p.name in allowed,rel;changed.append(rel)
 for name in ['nxt-soundcloud-backend','yt-dlp','yt-dlp-runtime/yt-dlp']:
  rel='upstream/bin/tg5040/'+name;assert sha((R/rel).read_bytes())==sha(z.read(prefix+rel)),rel
 old_meta=json.loads(z.read(prefix+'SOURCE.json'))
 assert old_meta['official_OTA_sources']==json.loads((R/'SOURCE.json').read_text())['official_OTA_sources']
 for n in ['nxt_ota.c','nxt_ota_package.c','nxt_dsd.c','nxt_bitperfect.c','nxt_translate.c','nxt_library_index.c','nxt_theme.c','nxt_skin.c','nxt_cpu.c','nxt_led.c']:
  rel='upstream/src/'+n;assert sha((R/rel).read_bytes())==sha(z.read(prefix+rel)),rel
report={'version':'1.10','exit':0,'baseline':'1.9.7.7','source_files_compared':checked,'resource_files_preserved':resources,'intended_source_changes':changed,'themes_fonts_artwork_unchanged':True,'soundcloud_ytdlp_runtime_unchanged':True,'original_three_OTA_feed_urls_unchanged':True,'DSD_bitperfect_translation_library_CPU_LED_OTA_code_unchanged':True,'physical_hardware_tested':False}
(R/'logs/v110-preservation.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS v1.9.7.7 preservation:',checked,'source files,',resources,'resource files, all official feeds; only intended player/login/exit changes')
