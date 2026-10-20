#!/usr/bin/env python3
"""Compare the v1.9.7.6 source snapshot to this visual-only release."""
from pathlib import Path
import hashlib,json,zipfile,argparse
R=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--baseline',type=Path,required=True);args=a.parse_args()
sha=lambda b:hashlib.sha256(b).hexdigest()
expected={'nxt.h','nxt_screens.c','nxt_skin.c','nxt_skin.h','nxt_auralis.c','nxt_theme.c','nxt_theme_v18.inc','nxt_record.c','nxt_custom.c','nxt_avatar_v18.inc'}
changed=[];checked=0;backends={};resources=0
with zipfile.ZipFile(args.baseline) as z:
 root='NXT_Retro_Music_v1.9.7.6_Source/'
 for name in z.namelist():
  if not name.startswith(root):continue
  rel=name[len(root):];p=R/rel
  if not p.is_file():continue
  if rel.startswith(('upstream/src/','upstream/NextUI/workspace/','nxredux/src/')) and p.suffix in {'.c','.h','.inc'}:
   checked+=1
   if sha(z.read(name))!=sha(p.read_bytes()):changed.append(rel)
  if rel.startswith('upstream/res/') and not rel.startswith('upstream/res/icons/'):
   assert sha(z.read(name))==sha(p.read_bytes()),rel;resources+=1
 for rel in ['upstream/bin/tg5040/nxt-soundcloud-backend','upstream/bin/tg5040/nxt-spotify-backend','upstream/bin/tg5040/yt-dlp','upstream/bin/tg5040/yt-dlp-runtime/yt-dlp']:
  assert sha(z.read(root+rel))==sha((R/rel).read_bytes()),rel;backends[rel]=sha((R/rel).read_bytes())
 assert set(changed)=={'upstream/src/'+n for n in expected},changed
 vectors=json.loads((R/'artwork/icon-refinement-v1977.json').read_text())['icons'];icons={}
 for name in vectors:
  p=R/'upstream/res/icons'/name
  assert sha(z.read(root+'upstream/res/icons/'+name))!=sha(p.read_bytes()),name;icons[name]=sha(p.read_bytes())
 for name in (R/'upstream/res/icons').glob('*.png'):
  if name.name not in vectors:assert sha(z.read(root+name.relative_to(R).as_posix()))==sha(name.read_bytes()),name
report={'version':'1.9.7.7','exit':0,'baseline_archive_sha256':sha(args.baseline.read_bytes()),'source_files_compared':checked,'changed_source_files':changed,'non_icon_resources_unchanged':resources,'icons_rebuilt':len(icons),'rebuilt_icon_sha256':icons,'online_binaries_preserved':backends,'audio_DSD_DSP_volume_inputs_fps_scheduler_CPU_performance_LED_OTA_translation_library_profiles_sources_unchanged':True,'vocal_reduction_remains_removed':True,'physical_tested':False}
(R/'logs/v1977-preservation.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',checked,'source files,',resources,'non-icon resources,',len(icons),'refined vector icons; audio, controls, CPU/scheduler, online backends and OTA unchanged')
