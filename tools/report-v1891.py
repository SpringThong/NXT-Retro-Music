#!/usr/bin/env python3
"""Record release inputs, test evidence and exact package checksums."""
from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1];OUT=R/'dist/nxredux-release';VERSION='1.8.9.1'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
base=json.loads((R/'logs/v1891-baseline-preserved.json').read_text());checks={}
for name in ['cpu-key','led','home','timer','icon','loader']:
 q=json.loads((R/f'logs/v1891-{name}-test.json').read_text());assert q['exit']==0;checks[name]=q
checks['OTA']=json.loads((R/'logs/v1891-ota-qa-nxredux.json').read_text());assert checks['OTA']['exit']==0
arts={}
for kind,name in [('installer',f'NXT_Retro_Music_v{VERSION}_NXRedux.zip'),('OTA_kit',f'NXT_OTA_Google_Drive_v{VERSION}_NXRedux.zip')]:
 p=OUT/name;arts[kind]={'filename':name,'size':p.stat().st_size,'sha256':sha(p)}
with zipfile.ZipFile(OUT/arts['installer']['filename']) as z:
 root='Tools/NXT Retro Music.pak/'
 assert z.read(root+'ota-source-nxredux.url').decode().strip()=='https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing'
 assert hashlib.sha256(z.read(root+'integration/nextui-v2.1.2-nxt-icon.elf')).hexdigest()==checks['icon']['patched_launcher_sha256']
 assert json.loads(z.read(root+'config.json'))['version']==VERSION
 assert not any(n.endswith('groq-api-key.txt') or n.endswith('translation-service.json') for n in z.namelist())
 assert not any(n.startswith('.system/') for n in z.namelist())
report={'version':VERSION,'rom':'nxredux','device':'TrimUI Brick Pro','firmware_reference':'NX Redux v2.1.2','restored_from_verified_v189_source':True,'build':{'exit':0,'compiler':'aarch64-nextui-linux-gnu-gcc 8.3.0','optimization':'O2','fresh_frontend_and_settings_adapter':True,'native_launcher_from_official_v212':True},'binary_sha256':sha(R/'nxredux/bin/tg5040/musicplayer.elf'),'adapter_sha256':sha(R/'nxredux/bin/tg5040/libnxt-nxredux-settings.so'),'package':{'historical_frontends_preserved':base['historical_frontends_preserved'],'UI_assets_preserved':base['unchanged_UI_assets'],'private_keys_excluded':True,'firmware_settings_library_not_replaced':True},'artifacts':arts,'changes':['Tools icon with exact-version launcher backup','Effective CPU modes','Native Home protocol bridge','Immediate LED colors','Groq TXT import; existing providers retained'],'validation':checks,'baseline_preservation':base,'official_OTA_source':'https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing','public_source_published':False,'physical_device_tested':False,'notes':['Home frame transport mocked because executor denies AF_UNIX sockets; actual ARM64 bridge compiled/run.','CPU/LED use driver file fixtures.','OTA production worker installs exact ZIP using local curl fixtures; publisher still must update Drive JSON.','Real hardware audio, temperature, battery, Home, LED, CPU and deep sleep remain untested.']}
(R/f'BUILD_REPORT_v{VERSION}_NXRedux.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
print('Release report complete; package and OTA verified.')
