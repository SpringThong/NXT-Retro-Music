from pathlib import Path
import hashlib,json,zipfile,os
r=Path(__file__).resolve().parents[1]
baseline=Path(os.environ.get('NXT_BASELINE_SOURCE',str(r.parents[2]/'releases_v1.9.7.3/NXT_Retro_Music_v1.9.7.3_Source.zip')))
def sha(b):return hashlib.sha256(b).hexdigest()
assert sha(baseline.read_bytes())=='df9e0adb02ee1bd8ee98bd9e06129c2f7ee54acece48af5ba93c9225457cc0d5'
allowed={f'upstream/src/{n}' for n in ['nxt.h','nxt_screens.c','nxt_controls.c','nxt_controls.h','nxt_config.c','nxt_dsp.c','nxt_audio_mode.c','nxt_skin.c','module_common.c']}
new_allowed={f'upstream/src/{n}' for n in ['nxt_vocal.c','nxt_vocal.h','nxt_vocal_ui.c']}
checked=set();changed=set();assets=0;current={}
with zipfile.ZipFile(baseline) as z:
 prefix='NXT_Retro_Music_v1.9.7.3_Source/'
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  new=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.4.json').read_text());old=json.loads(z.read(prefix+folder+'/BUILD_INPUTS_v1.9.7.3.json'))
  assert set(new['sources'])-set(old['sources'])==new_allowed
  for name,h in new['sources'].items():
   checked.add(name);assert sha((r/name).read_bytes())==h,name
   if name in old['sources'] and h!=old['sources'][name]:assert name in allowed,name;changed.add(name)
  assert sha((r/folder/'bin/tg5040/musicplayer.elf').read_bytes())==new['binary_sha256'];current[rom]=new['binary_sha256']
 for name in z.namelist():
  rel=name.removeprefix(prefix)
  if rel.startswith('upstream/res/') and not name.endswith('/'):assert (r/rel).read_bytes()==z.read(name),rel;assets+=1
 for name in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  rel='upstream/bin/tg5040/'+name;assert (r/rel).read_bytes()==z.read(prefix+rel)
assert changed==allowed,(changed,allowed-changed)
report={'exit':0,'version':'1.9.7.4','baseline':'1.9.7.3','baseline_sha256':sha(baseline.read_bytes()),'source_inputs_checked':len(checked),'changed_inputs':sorted(changed),'unchanged_resource_files':assets,'decoder_raw_DSD_online_backends_and_native_compositor_unchanged':True,'new_DSP_and_audio_policy_files':sorted(new_allowed),'online_backends_byte_identical':True,'current_binaries':current}
(r/'logs/v1974-preservation.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS preservation',len(checked),'source inputs;',assets,'resource files byte-identical; audio/network backend source unchanged.')
