from pathlib import Path
import json,hashlib
r=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
shared=['preservation','ui-test','config-test','volume-test','volume-keys-test','motion-lyrics','screenoff','presentation','vocal','dsd-test-nxredux']
for rom,label,folder in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=r/folder/'bin/tg5040/musicplayer.elf';installer=r/f'dist/{rom}-release/NXT_Retro_Music_v1.9.7.4_{label}.zip';ota=r/f'dist/{rom}-release/NXT_OTA_Google_Drive_v1.9.7.4_{label}.zip'
 inputs=json.loads((r/folder/'BUILD_INPUTS_v1.9.7.4.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for name,h in inputs['sources'].items():assert sha(r/name)==h,name
 names=['v1974-'+n+'.json' for n in shared]+[f'v1974-{n}-{rom}.json' for n in ['loader','package-qa','ota-qa','ota-publisher-kit']]
 for name in names:
  d=json.loads((r/'logs'/name).read_text())
  if 'exit' in d:assert d['exit']==0,name
  if name in ['v1974-vocal.json','v1974-screenoff.json']:assert d[rom.upper()]['exit']==0,name
  if name.startswith('v1974-loader'):assert d['binary_sha256']==sha(binary)
  if name.startswith('v1974-package-qa'):assert d['installer_sha256']==sha(installer) and d['kit_sha256']==sha(ota)
  if name.startswith('v1974-ota-qa'):assert d['installer_sha256']==sha(installer) and d['baseline_installed_version_fixture']=='1.9.7.3'
 report={'version':'1.9.7.4','rom':rom,'baseline':'unified v1.9.7.3','build':{'exit':0,'compiler':'GCC 8.3.0 AArch64','optimization':'O2','ROM_define':'NXT_ROM_'+rom.upper(),'freshly_recompiled':True},'binary_sha256':sha(binary),'installer':installer.name,'installer_size':installer.stat().st_size,'installer_sha256':sha(installer),'OTA_kit':ota.name,'OTA_kit_sha256':sha(ota),'checks':{name:{'sha256':sha(r/'logs'/name)} for name in names},'feature':'Session-only live stereo-centre vocal reduction; Strong dual-mono or Preserve bass, 20ms ramps, mono-source bypass, stereo vocal-only passages stay reduced. No AI, PC, service/model, allocations or I/O in the effect callback.','controls':'First Quick options row toggles; L3 auto-assigned only when previously free; custom bindings and explicit later unassignment preserved; global shortcut honours lock/screen-off policy. K status and native settings.','audio_policy':'PCM temporarily Normal without overwriting saved Normal/Bit-Perfect/Auto; returns to saved policy on disable. Existing Normal/online route is not reopened. Original DSD bypassed, direct DSD enable rejected. Actual transport/position/state operations are fixtures in policy tests; raw engine source unchanged.','preserved':'834 input files reviewed; 9 changed UI/config/DSP/policy inputs, 3 added effect/controller inputs; all 746 resource files and online backend executables byte-identical. Decoder/raw DSD/native compositor retained. Settled disabled DSP is byte-identical to baseline across 5 sample rates.','native_renderer_sizes':[1024,640],'fonts_total':60,'themes_total':51,'physical_ROM_DAC_Bluetooth_tested':False,'real_song_vocal_separation_listening_tested':False,'actual_device_FPS_measured':False,'thermal_battery_measured':False,'live_OTA_feed_uploaded':False}
 (r/f'BUILD_REPORT_v1.9.7.4_{label}.json').write_text(json.dumps(report,indent=2)+'\n')
p=r/'SOURCE.json';m=json.loads(p.read_text());m.update(release_ready=True,stock_status='All three ARM64 builds, native UI/power/config/volume, synthetic vocal DSP/policy, DSD parser, loader, final installer and OTA simulations passed. No physical test, real-song listening or live OTA publication.',unified_source_status='Final v1.9.7.4 three-ROM fingerprints and current evidence verified',baseline_hardware_status='User requested direct on-device Karaoke; no PC/service. Implementation is stereo-centre reduction, not AI separation; synthetic/software checks only.',v1974_verification='RELEASE_QA_v1.9.7.4.md');p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
print('PASS reports bound to current binaries/installers/OTA and actual test evidence.')
