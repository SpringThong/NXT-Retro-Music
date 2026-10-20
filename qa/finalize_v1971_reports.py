from pathlib import Path
import json,hashlib
r=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
shared=['v1971-preservation.json','v1971-ui-test.json','v1971-font-assets.json','v1971-config-test.json','v1971-volume-test.json','v1971-volume-keys-test.json','v1971-index.json','v1971-motion-lyrics.json','v1971-frame-budget.json','v1971-karaoke-test.json','v1971-library-flow-nxredux.json','v1971-shuffle-controls.json']
for rom,label,folder in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=r/folder/'bin/tg5040/musicplayer.elf';p=r/f'dist/{rom}-release/NXT_Retro_Music_v1.9.7.1_{label}.zip';ota=r/f'dist/{rom}-release/NXT_OTA_Google_Drive_v1.9.7.1_{label}.zip'
 inputs=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.1.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for path,h in inputs['sources'].items():assert sha(r/path)==h,path
 names=shared+[f'v1971-dsd-test-{rom}.json',f'v1971-loader-{rom}.json',f'v1971-package-qa-{rom}.json',f'v1971-ota-qa-{rom}.json']
 for name in names:
  assert (r/'logs'/name).is_file(),name
  data=json.loads((r/'logs'/name).read_text())
  if 'exit' in data:assert data['exit']==0,name
  if name.startswith('v1971-loader'):assert data['binary_sha256']==sha(binary)
  if name.startswith('v1971-package'):assert data['installer_sha256']==sha(p) and data['kit_sha256']==sha(ota)
  if name.startswith('v1971-ota-qa'):assert data['installer_sha256']==sha(p) and data['baseline_installed_version_fixture']=='1.9.7'
 report={'version':'1.9.7.1','rom':rom,'baseline':'unified v1.9.7','source_scope':'Shared source, separate ROM adapters and installers','build':{'exit':0,'entrypoint':'python3 tools/build-release.py --rom all --jobs 4','compiler':'GCC 8.3.0 AArch64','optimization':'O2','ROM_define':'NXT_ROM_'+rom.upper(),'freshly_recompiled':True},'binary_sha256':sha(binary),'installer':p.name,'installer_size':p.stat().st_size,'installer_sha256':sha(p),'OTA_kit':ota.name,'OTA_kit_sha256':sha(ota),'checks':{name:{'sha256':sha(r/'logs'/name)}for name in names},'native_renderer_sizes':[1024,640],'fonts_total':60,'themes_total':51,'animation_target_Hz':60,'actual_device_FPS_measured':False,'Groq_live_singing_tested':False,'karaoke_scope':'Optional consented local-file Whisper word analysis <=10 min; partial accent-insensitive alignment + labelled estimated fallback; no universal singing accuracy guarantee','new_scope':'38 licensed fonts and native transactional bilingual appearance/font preview; karaoke algorithm unchanged from v1.9.7','physical_ROM_DAC_Bluetooth_tested':False,'thermal_battery_measured':False,'live_OTA_feed_uploaded':False}
 (r/f'BUILD_REPORT_v1.9.7.1_{label}.json').write_text(json.dumps(report,indent=2)+'\n')
meta=json.loads((r/'SOURCE.json').read_text());meta.update(release_ready=True,stock_status='All three v1.9.7.1 ARM64 builds, native renderer/parser/motion/config/volume/library/DSD policy, loader and final installer/OTA checks passed. No physical test or live feed upload.',unified_source_status='Final three-ROM fingerprints and current v1.9.7.1 evidence verified',v1971_verification='RELEASE_QA_v1.9.7.1.md',baseline_hardware_status='User reported v1.9.7 issues; v1.9.7.1 has software/rendering checks only.')
(r/'SOURCE.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n');print('PASS final release reports tied to actual binaries, final installer hashes, OTA kit hashes and current test evidence.')
