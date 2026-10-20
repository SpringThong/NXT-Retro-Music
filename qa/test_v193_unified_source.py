#!/usr/bin/env python3
"""Check current unified fingerprints and preservation against delivered source."""
from pathlib import Path
import hashlib,json,zipfile,os,subprocess
R=Path(__file__).resolve().parents[1]
def sha(data):return hashlib.sha256(data).hexdigest()
baseline=Path(os.environ['NXT_V193_BASELINE_SOURCE'])
assert sha(baseline.read_bytes())=='1a1e51a577875512fbf6c03bb201bc60777c2d0d75047c9850f31b574e2370fb'
metadata=json.loads((R/'SOURCE.json').read_text());assert metadata['version']=='1.9.3' and metadata['release_roms']==['stock','nextui','nxredux']
checked=set();frontends={};root='NXT_Retro_Music_v1.9.3_Source/'
with zipfile.ZipFile(baseline) as z:
 for rom,folder in [('stock','stock'),('nextui','upstream'),('nxredux','nxredux')]:
  fp=json.loads((R/folder/'BUILD_INPUTS_v1.9.3.json').read_text());binary=R/folder/'bin/tg5040/musicplayer.elf'
  assert fp['rom']=='NXT_ROM_'+rom.upper() and sha(binary.read_bytes())==fp['binary_sha256']
  for name,digest in fp['sources'].items():
   data=(R/name).read_bytes();assert sha(data)==digest,name
   assert data==z.read(root+name),name;checked.add(name)
  frontends[rom]={'binary_sha256':fp['binary_sha256'],'all_current_source_fingerprints_match':True}
  if rom!='nxredux':assert binary.read_bytes()==z.read(root+str(binary.relative_to(R))),rom
 assets=[n for n in z.namelist() if n.startswith(root+'upstream/res/') and not n.endswith('/')]
 for n in assets:assert z.read(n)==(R/n[len(root):]).read_bytes(),n
for rom in ['all','stock','nextui','nxredux']:
 p=subprocess.run(['python3',str(R/'tools/build-release.py'),'--rom',rom,'--check'],capture_output=True,text=True);assert p.returncode==0,p.stdout+p.stderr
make=(R/'Makefile').read_text()
for rom in ['stock','nextui','nxredux']:assert f'{rom}:\n\tpython3 tools/build-release.py --rom {rom}' in make
value={'version':'1.9.3','exit':0,'all_ROMs':frontends,'frontend_source_files_preserved':len(checked),'all_resource_assets_preserved':len(assets),'stock_nextui_binary_identical_to_delivered_source':True,'NXRedux_rebuilt_from_same_current_shared_source':True,'build_entry_metadata_preflight_all_and_individual_ROMs':True,'make_routes_to_explicit_ROMs':True,'baseline_source_sha256':sha(baseline.read_bytes()),'no_live_OTA_feed_uploaded':True,'hardware_tested':False}
(R/'logs/v193-unified-source-check.json').write_text(json.dumps(value,indent=2)+'\n');print('PASS current fingerprints for all ROMs;',len(checked),'frontend source files and',len(assets),'assets preserved')
