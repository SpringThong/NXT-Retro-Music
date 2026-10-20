from pathlib import Path
import json,hashlib
r=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
shared=['preservation','ui-test','config-test','volume-test','volume-keys-test','motion-lyrics','screenoff','page-baseline-regression','dsd-test-nxredux']
for rom,label,folder in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=r/folder/'bin/tg5040/musicplayer.elf';installer=r/f'dist/{rom}-release/NXT_Retro_Music_v1.9.7.2_{label}.zip';ota=r/f'dist/{rom}-release/NXT_OTA_Google_Drive_v1.9.7.2_{label}.zip'
 inputs=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.2.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for name,h in inputs['sources'].items():assert sha(r/name)==h,name
 names=['v1972-'+n+'.json' for n in shared]+[f'v1972-{n}-{rom}.json' for n in ['loader','package-qa','ota-qa','ota-publisher-kit']]
 for name in names:
  d=json.loads((r/'logs'/name).read_text())
  if 'exit' in d:assert d['exit']==0,name
  if name.startswith('v1972-loader'):assert d['binary_sha256']==sha(binary)
  if name.startswith('v1972-package-qa'):assert d['installer_sha256']==sha(installer) and d['kit_sha256']==sha(ota)
  if name.startswith('v1972-ota-qa'):assert d['installer_sha256']==sha(installer) and d['baseline_installed_version_fixture']=='1.9.7.1'
 report={'version':'1.9.7.2','rom':rom,'baseline':'unified v1.9.7.1','build':{'exit':0,'compiler':'GCC 8.3.0 AArch64','optimization':'O2','ROM_define':'NXT_ROM_'+rom.upper(),'freshly_recompiled':True},'binary_sha256':sha(binary),'installer':installer.name,'installer_size':installer.stat().st_size,'installer_sha256':sha(installer),'OTA_kit':ota.name,'OTA_kit_sha256':sha(ota),'checks':{name:{'sha256':sha(r/'logs'/name)} for name in names},'root_cause':'Two page identities restarted Original Now Playing entrance on every repaint; reproduced with baseline and fixed by one identity per playback page.','screen_off_controls':'Persisted device-wide, default off; mapped keys/volume remain active while dark, Power wakes, hold SELECT lock/unlock, wake release guards, no Now Playing redraw while off.','preserved':'831 source inputs reviewed; 746 resources and online backend executables byte-identical; audio/network engines unchanged','native_renderer_sizes':[1024,640],'fonts_total':60,'themes_total':51,'physical_ROM_DAC_Bluetooth_tested':False,'actual_device_FPS_measured':False,'thermal_battery_measured':False,'live_OTA_feed_uploaded':False}
 (r/f'BUILD_REPORT_v1.9.7.2_{label}.json').write_text(json.dumps(report,indent=2)+'\n')
p=r/'SOURCE.json';m=json.loads(p.read_text());m.update(release_ready=True,stock_status='Three ARM64 builds, native renderer/power/input/config/volume/DSD, loader, installer and final OTA simulation passed. No physical test or live feed publication.',unified_source_status='Final v1.9.7.2 three-ROM fingerprints and current evidence verified',baseline_hardware_status='User supplied videos and reports of Original theme jumping on all three ROMs; software regression reproduced against v1.9.7.1.',v1972_verification='RELEASE_QA_v1.9.7.2.md');p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
print('PASS all current reports tied to actual build fingerprints, binary hashes, installers, OTA kits and test evidence.')
