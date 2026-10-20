#!/usr/bin/env python3
"""Freeze current release evidence after real binary/package/OTA checks pass."""
from pathlib import Path
import datetime,hashlib,json
R=Path(__file__).resolve().parents[1];VERSION='1.8.2'
assert not json.loads((R/'SOURCE.json').read_text()).get('release_build_paused_by_user'), 'Release build paused by user for Bluetooth investigation; do not finalize old binaries.'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def read(name):return json.loads((R/'logs'/name).read_text())
def passed(v):
 if isinstance(v,dict):
  for k,x in v.items():
   if k=='exit':assert x==0,(k,x)
   passed(x)
 elif isinstance(v,list):
  for x in v:passed(x)
common=['v182-ui-test.json','v182-ui-nextui-test.json','v182-config-test.json','v182-performance-test.json','v182-stock-rootfs-test.json','v182-packages-test.json','v182-baseline-preserved.json','v182-publisher-generator-qa.json','v182-nextui-bluetooth-test.json','v182-led-driver-test.json','v182-bt-user-log-evidence.json']
for name in common:passed(read(name))
assert read('v182-ui-test.json')['rom']=='NXT_ROM_STOCK'
assert read('v182-ui-nextui-test.json')['rom']=='NXT_ROM_NEXTUI'
assert read('v182-baseline-preserved.json')['baseline']=='1.8.1'
assert read('v182-performance-test.json')['modes_reduce_actual_visual_work']
bt=read('v182-nextui-bluetooth-test.json')
assert not bt['release_build_paused_by_user']
for check in ['v181_original_selector_reproduces_user_log','fixed_source_recovers_requested_BT_with_stale_speaker_config','firmware_global_asoundrc_unchanged','Bluetooth_volume_explicit_unmute_both_ROMs','idle_no_audio_reopen']:
 assert bt[check],check
for name,value in bt['sources'].items():assert sha(R/'upstream/src'/name)==value,name
assert sha(R/'upstream/NextUI/workspace/tg5040/libmsettings/msettings.c')==bt['msettings_sha256']
packages=read('v182-packages-test.json')
for rom,label,binary in [('stock','Stock','stock/bin/tg5040/musicplayer.elf'),('nextui','NextUI','upstream/bin/tg5040/musicplayer.elf')]:
 names=[f'v182-arm64-{rom}-test.json',f'v182-ota-qa-{rom}.json',f'v182-ota-publisher-kit-{rom}.json']
 arm,ota,kit=[read(n) for n in names]
 for v in [arm,ota,kit]:passed(v);assert v['version']==VERSION and v['rom']==rom
 installer=R/f'dist/{rom}-release/NXT_Retro_Music_v{VERSION}_{label}.zip';publisher=R/f'dist/{rom}-release/NXT_OTA_Google_Drive_v{VERSION}_{label}.zip'
 assert arm['binary_sha256']==sha(R/binary)
 assert 'v'+VERSION in arm['checks']["['--build-info']"]['stdout']
 assert packages[rom]['sha256']==ota['installer_sha256']==kit['installer_sha256']==sha(installer)
 assert kit['kit_sha256']==sha(publisher)
 assert ota['baseline_installed_version_fixture']=='1.8.1'
 for name,value in ota['OTA_source_sha256'].items():assert sha(R/'upstream/src'/name)==value
 inputs=json.loads((R/('stock' if rom=='stock' else 'upstream')/f'BUILD_INPUTS_v{VERSION}.json').read_text())
 assert inputs['binary_sha256']==sha(R/binary)
 for name,value in inputs['sources'].items():assert sha(R/name)==value,name
 result={'version':VERSION,'rom':rom,'build':{'exit':0,'compiler':'pinned aarch64-nextui-linux-gnu-gcc 8.3.0','native_AArch64':True,'frontend_sources':'stock/frontend-sources.json'},'binary':binary,'binary_sha256':sha(R/binary),'installer':installer.name,'installer_sha256':sha(installer),'installer_bytes':installer.stat().st_size,'publisher_kit':publisher.name,'publisher_kit_sha256':sha(publisher),'checks':{n:{'sha256':sha(R/'logs'/n)} for n in common+names},'native_render_resolutions':[[1024,768],[640,480]],'native_layouts':36,'performance_modes':['Battery saver','Balanced','Performance'],'default_performance_mode':'Balanced','performance_scope':'Visual refresh, FFT and vinyl/preview animation budgets; audio/DSP, input polling and firmware CPU policy unchanged','hardware_tested':False,'physical_temperature_measured':False,'physical_battery_life_measured':False,'physical_USB_DAC_tested':False,'authenticated_account_playback_tested':False,'OTA_public_source_published':False,'verification_notes':'RELEASE_QA_v1.8.2.md','created_UTC':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 (R/f'BUILD_REPORT_v{VERSION}_{label}.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
meta=json.loads((R/'SOURCE.json').read_text())
meta.update(stage='v1.8.2-native-packaged',build_variant='Stock / NextUI',stock_status='Fresh Stock/NextUI binaries, installers and OTA publisher kits built with the tested Bluetooth fix',current_build_report='BUILD_REPORT_v1.8.2_Stock.json',build_report='BUILD_REPORT_v1.8.2_Stock.json',current_build_reports=['BUILD_REPORT_v1.8.2_Stock.json','BUILD_REPORT_v1.8.2_NextUI.json'],previous_build_report='BUILD_REPORT_v1.8.1_Stock.json',baseline_version='1.8.1',hardware_test_status='User confirms earlier versions played Bluetooth; supplied v1.8.1 NextUI logs confirm the route failure. Fresh v1.8.2 passed regression and binary/package checks; physical Bluetooth playback not tested.',v182_verification='RELEASE_QA_v1.8.2.md',current_binaries_contain_Bluetooth_fix=True)
meta['features_v182']['Bluetooth_source_fix']['new_native_release_built']=True
meta['bluetooth_device_log_evidence']['new_native_release_built']=True
meta.pop('pre_Bluetooth_build_reports',None)
meta['features_v182']['original_non_square_art']='Centered aspect-fit within the old album frame'
meta['features_v182']['performance_policy']={'playing_regular_ms':[125,100,50],'playing_mechanical_ms':[100,50,33],'FFT_ms':[67,33,33],'vinyl_preview_ms':[100,50,33],'paused_ms':1000,'EInk_ms':250,'hidden_off_visual_work':0}
(R/'SOURCE.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
print('PASS current Stock/NextUI build evidence, exact binaries/installers/OTA kits and full source input hashes. Wrote v1.8.2 reports.')
