from pathlib import Path
import hashlib,json,zipfile,os
r=Path(__file__).resolve().parents[1]
baseline=Path(os.environ.get('NXT_BASELINE_SOURCE',str(r.parents[1]/'source-input/NXT_Retro_Music_v1.9.7.2_Source.zip')))
def sha(b):return hashlib.sha256(b).hexdigest()
assert sha(baseline.read_bytes())=='3884ae8be59f2e008e64664f178b4fdf7ece49ad3beb2fe26244754b7c715ad6'
allowed={f'upstream/src/{n}' for n in ['nxt.h','nxt_motion.c','nxt_motion.h','nxt_screens.c']}
allowed.add('upstream/NextUI/workspace/all/common/generic_video.c')
checked=set();changed=set();assets=0;current={}
with zipfile.ZipFile(baseline) as z:
 prefix='NXT_Retro_Music_v1.9.7.2_Source/'
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  new=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.3.json').read_text());old=json.loads(z.read(prefix+folder+'/BUILD_INPUTS_v1.9.7.2.json'))
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
report={'exit':0,'version':'1.9.7.3','baseline':'1.9.7.2','baseline_sha256':sha(baseline.read_bytes()),'source_inputs_checked':len(checked),'changed_inputs':sorted(changed),'unchanged_resource_files':assets,'audio_and_network_engines_unchanged':True,'online_backends_byte_identical':True,'current_binaries':current}
(r/'logs/v1973-preservation.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS preservation',len(checked),'source inputs;',assets,'resource files byte-identical; audio/network backend source unchanged.')
