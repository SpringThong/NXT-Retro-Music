#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1];V='1.9.1'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def read(p):return json.loads((R/p).read_text())
common={'cache':read('logs/v191-cache-test.json'),'translation':read('logs/v191-translation-test.json'),'original_then_translation':read('logs/v191-original-translation-test.json'),'Groq_TXT_CPU':read('logs/v1891-cpu-key-test.json')}
for n,t in common.items():assert t['exit']==0,n
assert common['cache']['source_sha256']==sha(R/'upstream/src/nxt_translate.c')
for p,h in common['original_then_translation']['sources'].items():assert sha(R/'upstream/src'/p)==h
results={}
for rom,label,d in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 fp=read(f'{d}/BUILD_INPUTS_v{V}.json');binary=R/d/'bin/tg5040/musicplayer.elf';assert fp['binary_sha256']==sha(binary)
 for p,h in fp['sources'].items():assert sha(R/p)==h,p
 loader=read(f'logs/v191-loader-{rom}.json');ota=read(f'logs/v191-ota-qa-{rom}.json');dsd=read(f'logs/v191-dsd-{rom}.json');assert loader['exit']==ota['exit']==dsd['exit']==0
 for p,h in dsd['sources'].items():assert sha(R/'upstream/src'/p)==h,p
 artifacts={}
 for key,name in [('installer',f'NXT_Retro_Music_v{V}_{label}.zip'),('OTA_kit',f'NXT_OTA_Google_Drive_v{V}_{label}.zip')]:
  p=R/f'dist/{rom}-release'/name;artifacts[key]={'filename':name,'size':p.stat().st_size,'sha256':sha(p)}
 assert artifacts['installer']['sha256']==ota['installer_sha256']
 with zipfile.ZipFile(R/f'dist/{rom}-release'/artifacts['installer']['filename']) as z:
  app={'stock':'Apps/NXT_Retro_Music','nextui':'Tools/tg5040/NXT Retro Music.pak','nxredux':'Tools/NXT Retro Music.pak'}[rom];assert hashlib.sha256(z.read(app+'/bin/tg5040/musicplayer.elf')).hexdigest()==fp['binary_sha256'];assert json.loads(z.read(app+'/config.json'))['version']==V
  assert (app+'/integration/nextui-v2.1.2-nxt-icon.elf' in z.namelist())==(rom=='nxredux');assert all(Path(p).name not in ['groq-api-key.txt','groq-key.txt','translation-service.json','credentials.json'] for p in z.namelist())
 report={'version':V,'rom':rom,'baseline_version':'1.9' if rom=='nxredux' else '1.8.9','build':{'exit':0,'compiler':'GCC 8.3.0 AArch64','optimization':'O2'},'binary_sha256':fp['binary_sha256'],'artifacts':artifacts,'tests':{'common':common,'DSD':dsd,'loader':loader,'OTA':ota},'changes':['256 MiB translation cache on SD; no 512-song cap; bounded low-priority cleanup','Original local raw DSF/DFF DSD64-512, strict Native DSD and explicit DoP','Groq TXT import on all ROMs; MyMemory and Google retained','Original LRC/cache/LRCLIB lookup feeds translation; filename normalization and artist matching','ROM-specific integrations preserved'],'OTA':{'official_source':(R/d/f'ota-source-{rom}.url').read_text().strip(),'publisher_live_feed_uploaded':False},'hardware_tested':False,'limits':['Physical ROM/DAC compatibility, temperature, power and underruns require device testing','No DST/SACD ISO/remote DSD/DSD-to-PCM fallback','Original lyrics availability depends on metadata and LRCLIB; no audio speech transcription']}
 if rom=='nxredux':
  report['tests']['Home']=read('logs/v1891-home-test.json');report['tests']['LED']=read('logs/v1891-led-test.json');assert report['tests']['Home']['exit']==report['tests']['LED']['exit']==0
 (R/f'BUILD_REPORT_v{V}_{label}.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n');results[rom]={'binary_sha256':fp['binary_sha256'],'artifacts':artifacts}
(R/f'logs/v191-release-summary.json').write_text(json.dumps({'version':V,'exit':0,'roms':results},indent=2)+'\n')
(R/f'RELEASE_QA_v{V}.md').write_text('# v1.9.1 software validation\n\nAll three ARM64 loader builds and actual installer OTA integration tests passed (ROM/version/SHA/size, wrong-ROM rejection, cancellation, old-app backup and user-data preservation). Native/DoP byte equality, PCM policies, TXT import, cache retention/eviction and original-lyrics-to-translation cold offline reuse are checked with explicit hardware/network fixtures and ASan/UBSan.\n\nNo physical ROM boot, live service API, DAC Native/DoP compatibility, temperature or power measurements. Read each BUILD_REPORT for exact scope and checksums.\n')
print('PASS current sources, all three build fingerprints, tested installer hashes, ROM separation and private-key exclusion.')
