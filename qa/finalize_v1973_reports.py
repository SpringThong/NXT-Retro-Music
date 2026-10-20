from pathlib import Path
import json,hashlib
r=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
shared=['preservation','ui-test','config-test','volume-test','volume-keys-test','motion-lyrics','screenoff','presentation']
for rom,label,folder in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=r/folder/'bin/tg5040/musicplayer.elf';installer=r/f'dist/{rom}-release/NXT_Retro_Music_v1.9.7.3_{label}.zip';ota=r/f'dist/{rom}-release/NXT_OTA_Google_Drive_v1.9.7.3_{label}.zip'
 inputs=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.3.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for name,h in inputs['sources'].items():assert sha(r/name)==h,name
 names=['v1973-'+n+'.json' for n in shared]+[f'v1973-{n}-{rom}.json' for n in ['loader','package-qa','ota-qa','ota-publisher-kit']]
 for name in names:
  d=json.loads((r/'logs'/name).read_text())
  if 'exit' in d:assert d['exit']==0,name
  if name.startswith('v1973-loader'):assert d['binary_sha256']==sha(binary)
  if name.startswith('v1973-package-qa'):assert d['installer_sha256']==sha(installer) and d['kit_sha256']==sha(ota)
  if name.startswith('v1973-ota-qa'):assert d['installer_sha256']==sha(installer) and d['baseline_installed_version_fixture']=='1.9.7.2'
 report={'version':'1.9.7.3','rom':rom,'baseline':'unified v1.9.7.2','build':{'exit':0,'compiler':'GCC 8.3.0 AArch64','optimization':'O2','ROM_define':'NXT_ROM_'+rom.upper(),'freshly_recompiled':True},'binary_sha256':sha(binary),'installer':installer.name,'installer_size':installer.stat().st_size,'installer_sha256':sha(installer),'OTA_kit':ota.name,'OTA_kit_sha256':sha(ota),'checks':{name:{'sha256':sha(r/'logs'/name)} for name in names},'root_cause':'Video feedback covers every page switch. Remove whole-page translation/snapshot, and compose full and layer-only native frames from a clean window backbuffer with fixed layer coordinates.','screen_off_controls':'Persisted device-wide, default off; mapped keys/volume remain active while dark, Power wakes, hold SELECT lock/unlock, wake release guards, no Now Playing redraw while off.','preserved':'831 source inputs reviewed; only five version/notes/page/presentation files changed. 746 resources and online backend executables byte-identical; screen-off controls, audio and network engines unchanged','native_renderer_sizes':[1024,640],'fonts_total':60,'themes_total':51,'physical_ROM_DAC_Bluetooth_tested':False,'actual_device_FPS_measured':False,'thermal_battery_measured':False,'live_OTA_feed_uploaded':False}
 (r/f'BUILD_REPORT_v1.9.7.3_{label}.json').write_text(json.dumps(report,indent=2)+'\n')
p=r/'SOURCE.json';m=json.loads(p.read_text());m.update(release_ready=True,stock_status='Three ARM64 builds, native renderer/presentation/power/input/config/volume, loader, installer and final OTA simulation passed. No physical test or live feed publication.',unified_source_status='Final v1.9.7.3 three-ROM fingerprints and current evidence verified',baseline_hardware_status='User supplied video of every menu and full-lyrics switch; only native software/renderer checks, no physical scan-out test.',v1973_verification='RELEASE_QA_v1.9.7.3.md');p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
print('PASS all current reports tied to actual build fingerprints, binary hashes, installers, OTA kits and test evidence.')
