from pathlib import Path
import hashlib,json,zipfile,re
r=Path(__file__).resolve().parents[1];baseline=r.parent.parent/'baseline/NXT_Retro_Music_v1.9.6_Source.zip'
def sha(data):return hashlib.sha256(data).hexdigest()
assert sha(baseline.read_bytes())=='0de030e258af8662be53b983e36602ad259694232458e69f5d85eaf61994482b'
allowed={f'upstream/src/{n}' for n in ['lyrics.c','lyrics.h','nxt.h','nxt_appearance.c','nxt_auralis.c','nxt_config.c','nxt_screens.c','nxt_skin.c','nxt_skin.h','nxt_ui.c','nxt_karaoke.c','module_player.c','nxt_spotify.c','nxt_soundcloud.c']}
new_allowed={'upstream/src/nxt_lyric_words.c','upstream/src/nxt_lyric_words.h'}
checked=set();changed=set();assets=0;current={};prefix='NXT_Retro_Music_v1.9.6_Source/'
with zipfile.ZipFile(baseline) as z:
 names=set(z.namelist())
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  new=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.json').read_text());old=json.loads(z.read(prefix+folder+'/BUILD_INPUTS_v1.9.6.json'))
  assert set(old['sources'])<=set(new['sources'])
  for name,digest in new['sources'].items():
   data=(r/name).read_bytes();assert sha(data)==digest,name
   if prefix+name in names:
    checked.add(name)
    if data!=z.read(prefix+name):changed.add(name);assert name in allowed,name
   else:assert name in new_allowed,name
  current[rom]=new['binary_sha256'];assert sha((r/folder/'bin/tg5040/musicplayer.elf').read_bytes())==new['binary_sha256']
 for name in names:
  rel=name.removeprefix(prefix)
  if rel.startswith('upstream/res/') and not name.endswith('/'):assert (r/rel).read_bytes()==z.read(name),rel;assets+=1
 for name in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  rel='upstream/bin/tg5040/'+name;assert (r/rel).read_bytes()==z.read(prefix+rel)
 old=z.read(prefix+'upstream/src/nxt_v18_palette.inc').decode();new=(r/'upstream/src/nxt_v18_palette.inc').read_text()
 assert re.findall(r'"([^"]+)"',old.split(';')[0])==re.findall(r'"([^"]+)"',new.split(';')[0])
 assert re.findall(r'0x[0-9a-f]+',old)[:350]==re.findall(r'0x[0-9a-f]+',new)[:350]
 old=z.read(prefix+'upstream/src/nxt_text.c').decode();new=(r/'upstream/src/nxt_text.c').read_text()
 for decl in ['names','paths']:
  pattern=r'static const char \*'+decl+r'\[\]=\{([^}]+)\}'
  before=re.findall(r'"([^"]+)"',re.search(pattern,old)[1]);after=re.findall(r'"([^"]+)"',re.search(pattern,new)[1]);assert before==after and len(after)==22
assert changed==allowed,(changed,allowed-changed)
result={'version':'1.9.7','exit':0,'baseline_version':'1.9.6','baseline_sha256':sha(baseline.read_bytes()),'baseline_frontend_inputs_checked':len(checked),'modified_frontend_inputs':sorted(changed),'new_frontend_inputs':sorted(new_allowed),'original_resources_preserved':assets,'first_50_palettes_and_all_51_names_preserved':True,'first_17_font_indices_preserved':True,'online_backends_byte_identical':True,'current_binaries':current,'physical_tested':False}
(r/'logs/v197-preservation.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS preservation:',len(checked),'source inputs,',assets,'old resources, 50 original palettes, 17 original font indices, backend bytes and current binaries')
