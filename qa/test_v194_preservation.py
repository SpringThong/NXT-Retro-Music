from pathlib import Path
import hashlib,json,os,re,zipfile
R=Path(__file__).resolve().parents[1]
B=Path(os.environ['NXT_V194_BASELINE_SOURCE'])
def sha(data):return hashlib.sha256(data).hexdigest()
assert sha(B.read_bytes())=='83fd7cb0b44c13ff88a1dbe9566e890972a69a3c9b6cf49f3e6677cf4ab30f68'
allowed={f'upstream/src/{n}' for n in ['module_common.c','musicplayer.c','nxt.h','nxt_appearance.c','nxt_config.c','nxt_controls.c','nxt_hardware.c','nxt_player_core.c','nxt_screens.c','nxt_skin.c','nxt_skin.h','nxt_text.c','nxt_text.h','nxt_theme.c','nxt_theme.h','nxt_theme_scene.inc','nxt_theme_v18.inc','nxt_ui.c','nxt_v18_palette.inc','nxt_volume.c','nxt_volume.h']}
checked=set();changed=set();resources=0;frontends={};prefix='NXT_Retro_Music_v1.9.3_Source/'
with zipfile.ZipFile(B) as z:
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  f=json.loads((R/folder/'BUILD_INPUTS_v1.9.4.json').read_text());binary=(R/folder/'bin/tg5040/musicplayer.elf').read_bytes();assert sha(binary)==f['binary_sha256']
  old=json.loads(z.read(prefix+folder+'/BUILD_INPUTS_v1.9.3.json'))
  assert set(old['sources'])<=set(f['sources'])
  for n,digest in f['sources'].items():
   data=(R/n).read_bytes();assert sha(data)==digest,n
   if prefix+n in z.namelist():
    checked.add(n)
    if data!=z.read(prefix+n):changed.add(n);assert n in allowed,n
   else:assert n in ['upstream/src/nxt_auralis.c','upstream/src/nxt_auralis.h'],n
  frontends[rom]=f['binary_sha256']
 for n in z.namelist():
  rel=n.removeprefix(prefix)
  if rel.startswith('upstream/res/') and not n.endswith('/'):assert (R/rel).read_bytes()==z.read(n),rel;resources+=1
 for n in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  rel='upstream/bin/tg5040/'+n;assert (R/rel).read_bytes()==z.read(prefix+rel)
 old=z.read(prefix+'upstream/src/nxt_v18_palette.inc').decode();new=(R/'upstream/src/nxt_v18_palette.inc').read_text()
 assert re.findall(r'"([^"]+)"',old.split(';')[0])==re.findall(r'"([^"]+)"',new.split(';')[0])[:50]
 assert re.findall(r'0x[0-9a-f]+',old)==re.findall(r'0x[0-9a-f]+',new)[:350]
assert changed==allowed,(changed,allowed-changed)
assert sha((R/'upstream/res/fonts/misans-semibold.ttf').read_bytes())=='77c23f31ae124867778344970155a0c8d34a89897dedaab81aeee82ff00a4ce6'
assert 'Xiaomi' in (R/'upstream/licenses/MiSans.txt').read_text()
result={'version':'1.9.4','exit':0,'baseline_version':'1.9.3','baseline_sha256':sha(B.read_bytes()),'baseline_frontend_inputs_checked':len(checked),'modified_frontend_inputs':sorted(changed),'new_Auralis_module_only':True,'original_resources_preserved':resources,'first_50_theme_names_and_palettes_preserved':True,'online_backends_byte_identical':True,'unmodified_MiSans_SHA256_verified':True,'current_binaries':frontends,'physical_tested':False}
(R/'logs/v194-preservation.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS current three-ROM fingerprints,',len(checked),'baseline frontend inputs,',resources,'unchanged resources, 50 stable themes and original online backends')
