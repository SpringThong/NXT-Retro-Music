#!/usr/bin/env python3
from pathlib import Path
import json,hashlib
R=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def read(p):return json.loads((R/p).read_text())
tests={'DSD':read('logs/v19-dsd-test.json'),'release':read('logs/v19-release-test.json'),'OTA':read('logs/v19-ota-qa-nxredux.json'),'CPU_Groq_TXT':read('logs/v1891-cpu-key-test.json'),'LED':read('logs/v1891-led-test.json'),'Home':read('logs/v1891-home-test.json')}
for n,t in tests.items():assert t['exit']==0,n
fp=read('nxredux/BUILD_INPUTS_v1.9.json');assert fp['binary_sha256']==sha(R/'nxredux/bin/tg5040/musicplayer.elf')
for p,h in fp['sources'].items():assert sha(R/p)==h,p
for p,h in tests['DSD']['sources'].items():assert sha(R/'upstream/src'/p)==h,p
artifacts={}
for k,n in [('installer','NXT_Retro_Music_v1.9_NXRedux.zip'),('OTA_kit','NXT_OTA_Google_Drive_v1.9_NXRedux.zip')]:
 p=R/'dist/nxredux-release'/n;artifacts[k]={'filename':n,'size':p.stat().st_size,'sha256':sha(p)}
assert artifacts['installer']['sha256']==tests['OTA']['installer_sha256']
v={'version':'1.9','rom':'nxredux','device':'TrimUI Brick Pro','firmware_reference':'NX Redux v2.1.2','baseline_version':'1.8.9.1','baseline_source_sha256':'9f399e957ded04288a20138d01fa80aa9fc6cd18517454eec27f165d742bfeb7','build':{'exit':0,'compiler':'aarch64-nextui-linux-gnu-gcc 8.3.0','optimization':'O2','fresh_frontend_and_settings_adapter':True,'launcher_preserved_v1891':True},'binary_sha256':fp['binary_sha256'],'adapter_sha256':sha(R/'nxredux/bin/tg5040/libnxt-nxredux-settings.so'),'package':{'historical_frontends_preserved':tests['release']['historical_frontends_preserved'],'UI_assets_preserved':sum(p.startswith('upstream/res/') for p in tests['release']['unchanged_files']),'private_keys_excluded':True,'firmware_settings_library_not_replaced':True},'artifacts':artifacts,'changes':['Direct raw DSF/DFF mono/stereo DSD64-512','Native DSD default with actual hw DSD format validation','Explicit compatible-DAC DoP opt-in only','No PCM decode/resampling/DSP/software gain/FFT in DSD','Bounded file/audio blocks and pause on DAC removal','DSD metadata/artwork/resume and Wi-Fi copy','Retained NX v1891 integrations; Stock/NextUI remain v189'],'tests':tests,'OTA':{'official_source':(R/'nxredux/ota-source-nxredux.url').read_text().strip(),'live_source_uploaded':False,'publisher_action':'Upload ZIP and replace existing Drive JSON content, preserving file ID'},'limits':['Physical DAC/ROM Native DSD and DoP compatibility not tested','No measured device temperature, battery or hardware underrun data','No DST, SACD ISO, remote DSD streaming or PCM fallback','Device DSD volume requires DAC hardware mixer; no software sample gain']}
(R/'BUILD_REPORT_v1.9_NXRedux.json').write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n');print('PASS current build fingerprints, QA reports, installer and OTA kit SHA')
