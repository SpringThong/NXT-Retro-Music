#!/usr/bin/env python3
"""Write release reports only after the real v1.8 checks have passed."""
from pathlib import Path
import hashlib,json,datetime
R=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def report(n):return json.loads((R/'logs'/n).read_text())
common=['v18-ui-test.json','v18-ui-nextui-test.json','v18-ime-test.json','v18-keyboard-test.json','v18-transfer-test.json','v18-usb-test.json','v18-led-driver-test.json','nextui-frame-budget-test.json','v18-stock-rootfs-test.json','v18-packages-test.json']
def passed(value):
 if isinstance(value,dict):
  for k,v in value.items():
   if k=='exit':assert v==0,(k,v)
   passed(v)
 elif isinstance(value,list):
  for v in value:passed(v)
for n in common:passed(report(n))
source=json.loads((R/'SOURCE.json').read_text());source.update(stage='v1.8-big-update-packaged',build_variant='Stock / NextUI · Big Update',current_build_report='BUILD_REPORT_v1.8_Stock.json',build_report='BUILD_REPORT_v1.8_Stock.json',current_build_reports=['BUILD_REPORT_v1.8_Stock.json','BUILD_REPORT_v1.8_NextUI.json'],previous_build_report='BUILD_REPORT_v1.7.1_Stock.json',hardware_test_status='Not tested on physical device',baseline_version='1.7.1')
source['features_v18']={'interfaces':36,'avatar_icons':300,'avatar_3D_emoji':180,'pixel_avatar_removed':True,'profile_saved_avatar_startup':True,'mood_genre_country_checkin':True,'queue_shuffle_repeat_timer_controls':True,'full_screen_4_3_native_themes':True,'system_languages':14,'first_launch_language':'en','matching_offline_keyboards':True,'offline_IME_rows':1136777,'settings_groups':6,'searchable_settings':True,'help_sections':8,'WiFi_transfer_on_demand':True,'USB_firmware_handoff':True,'PayPal':'thongkhung2@gmail.com','live_LED_without_toggle':True,'render_budget':'20/10/4 Hz playing, 1 Hz paused; spectrum capped; input unchanged','hidden_visual_work':False,'author':'Nguyễn Xuân Thông','nickname':'Spring Thông','Vibe_Code_retained':True}
source['v18_verification']='RELEASE_QA_v1.8.md';(R/'SOURCE.json').write_text(json.dumps(source,ensure_ascii=False,indent=2)+'\n')
packages=report('v18-packages-test.json')
for rom,label,binary in [('stock','Stock','stock/bin/tg5040/musicplayer.elf'),('nextui','NextUI','upstream/bin/tg5040/musicplayer.elf')]:
 arm_name=f'v18-arm64-{rom}-test.json';ota_name=f'v18-ota-qa-{rom}.json'
 arm=report(arm_name);ota=report(ota_name);passed(arm);passed(ota)
 assert arm['binary_sha256']==sha(R/binary)
 assert arm['version']=='1.8' and arm['rom']==rom and ota['version']=='1.8' and ota['rom']==rom
 p=R/f'dist/{rom}-release/NXT_Retro_Music_v1.8_{label}.zip'
 assert packages[rom]['sha256']==sha(p)
 assert ota['installer_sha256']==sha(p)
 for n,h in ota['OTA_source_sha256'].items():assert sha(R/'upstream/src'/n)==h
 inputs=json.loads((R/('stock' if rom=='stock' else 'upstream')/'BUILD_INPUTS_v1.8.json').read_text())
 assert inputs['binary_sha256']==sha(R/binary)
 for n,h in inputs['sources'].items():assert sha(R/n)==h,n
 value={'version':'1.8','rom':rom,'build':{'exit':0,'compiler':'pinned aarch64-nextui-linux-gnu-gcc 8.3.0','native_AArch64':True,'frontend_sources':'stock/frontend-sources.json'},'binary':binary,'binary_sha256':sha(R/binary),'installer':p.name,'installer_sha256':sha(p),'installer_bytes':p.stat().st_size,'checks':{n:{'sha256':sha(R/'logs'/n)} for n in common+[arm_name,ota_name]},'native_render_resolutions':[[1024,768],[640,480]],'full_native_layouts':36,'hardware_tested':False,'physical_temperature_measured':False,'physical_USB_cable_tested':False,'authenticated_account_playback_tested':False,'OTA_published':False,'verification_notes':'RELEASE_QA_v1.8.md','created_UTC':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 (R/f'BUILD_REPORT_v1.8_{label}.json').write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n')
print('Verified release reports written for Stock and NextUI.')
