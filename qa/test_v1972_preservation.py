from pathlib import Path
import hashlib,json,zipfile,os
r=Path(__file__).resolve().parents[1]
baseline=Path(os.environ.get('NXT_BASELINE_SOURCE',str(r.parents[1]/'source-input/NXT_Retro_Music_v1.9.7.1_Source.zip')))
def sha(b):return hashlib.sha256(b).hexdigest()
assert sha(baseline.read_bytes())=='f27b966365674d24f5f804cfdeaffc28ee22b48c92c1222e3661ae1a7e9e4e0c'
allowed={f'upstream/src/{n}' for n in ['nxt.h','nxt_ui.c','nxt_skin.c','nxt_pad.c','nxt_controls.c','nxt_config.c','nxt_screens.c','module_common.c','module_player.c']}
allowed|={'upstream/NextUI/workspace/all/common/api.c','upstream/NextUI/workspace/all/common/api.h'}
checked=set();changed=set();assets=0;current={}
with zipfile.ZipFile(baseline) as z:
 prefix='NXT_Retro_Music_v1.9.7.1_Source/'
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  new=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.2.json').read_text());old=json.loads(z.read(prefix+folder+'/BUILD_INPUTS_v1.9.7.1.json'))
  assert set(old['sources'])==set(new['sources'])
  for name,h in new['sources'].items():
   checked.add(name);assert sha((r/name).read_bytes())==h,name
   if h!=old['sources'][name]:assert name in allowed,name;changed.add(name)
  assert sha((r/folder/'bin/tg5040/musicplayer.elf').read_bytes())==new['binary_sha256'];current[rom]=new['binary_sha256']
 for name in z.namelist():
  rel=name.removeprefix(prefix)
  if rel.startswith('upstream/res/') and not name.endswith('/'):assert (r/rel).read_bytes()==z.read(name),rel;assets+=1
 for name in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  rel='upstream/bin/tg5040/'+name;assert (r/rel).read_bytes()==z.read(prefix+rel)
assert changed==allowed,(changed,allowed-changed)
report={'exit':0,'version':'1.9.7.2','baseline':'1.9.7.1','baseline_sha256':sha(baseline.read_bytes()),'source_inputs_checked':len(checked),'changed_inputs':sorted(changed),'unchanged_resource_files':assets,'audio_and_network_engines_unchanged':True,'online_backends_byte_identical':True,'current_binaries':current}
(r/'logs/v1972-preservation.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS preservation',len(checked),'source inputs;',assets,'resource files byte-identical; audio/network backend source unchanged.')
