#!/usr/bin/env python3
"""Finalize only the freshly built and checked Stock/NextUI v1.8.3 releases."""
from pathlib import Path
import datetime,hashlib,json

R=Path(__file__).resolve().parents[1]
VERSION='1.8.3'

def sha(path):
    with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()

def read(name):return json.loads((R/'logs'/name).read_text())

def passed(value):
    if isinstance(value,dict):
        for key,item in value.items():
            if key=='exit':assert item==0,(key,item)
            passed(item)
    elif isinstance(value,list):
        for item in value:passed(item)

def main():
    common=['v183-timer-test.json','v183-config-test.json','v183-stock-runtime-test.json',
            'v183-nextui-bluetooth-test.json','v183-stock-rootfs-test.json',
            'v183-nextui-binary-test.json','v183-baseline-preserved.json',
            'v183-publisher-generator-qa.json','v183-packages-test.json']
    for name in common:passed(read(name))
    timer=read('v183-timer-test.json')
    assert all(timer['results'][rom]['full_exit_on_expiry'] for rom in ['stock','nextui'])
    stock=read('v183-stock-runtime-test.json')
    assert stock['owned_sleep_inhibitors_released'] and stock['other_app_inhibitors_preserved']
    assert stock['official_stock_sleep_flag_protocol_matches']
    baseline=read('v183-baseline-preserved.json')
    for relative,digest in baseline['unchanged_files'].items():assert sha(R/relative)==digest,relative
    for relative,digests in baseline['intentional_native_changes'].items():assert sha(R/relative)==digests['after'],relative
    for relative,digest in timer['sources'].items():assert sha(R/'upstream/src'/relative)==digest,relative
    bt=read('v183-nextui-bluetooth-test.json')
    for relative,digest in bt['sources'].items():assert sha(R/'upstream/src'/relative)==digest,relative
    assert sha(R/'upstream/NextUI/workspace/tg5040/libmsettings/msettings.c')==bt['msettings_sha256']
    for rom,label,binary in [('stock','Stock','stock/bin/tg5040/musicplayer.elf'),
                              ('nextui','NextUI','upstream/bin/tg5040/musicplayer.elf')]:
        names=[f'v183-ota-qa-{rom}.json',f'v183-ota-publisher-kit-{rom}.json']
        ota,kit=[read(name) for name in names]
        for value in [ota,kit]:passed(value);assert value['rom']==rom and value['version']==VERSION
        assert ota['baseline_installed_version_fixture']=='1.8.2'
        installer=R/f'dist/{rom}-release/NXT_Retro_Music_v{VERSION}_{label}.zip'
        publisher=R/f'dist/{rom}-release/NXT_OTA_Google_Drive_v{VERSION}_{label}.zip'
        assert sha(installer)==read('v183-packages-test.json')[rom]['sha256']==ota['installer_sha256']==kit['installer_sha256']
        assert sha(publisher)==kit['kit_sha256']
        for relative,digest in ota['OTA_source_sha256'].items():assert sha(R/'upstream/src'/relative)==digest,relative
        inputs=json.loads((R/('stock' if rom=='stock' else 'upstream')/f'BUILD_INPUTS_v{VERSION}.json').read_text())
        assert inputs['binary_sha256']==sha(R/binary)
        for relative,digest in inputs['sources'].items():assert sha(R/relative)==digest,relative
        result={
            'project':'NXT Retro Music','version':VERSION,'rom':rom,'device':'trimui-brick-pro',
            'build':{'exit':0,'compiler':'pinned GCC 8.3.0 AArch64','all_source_inputs_verified':True,
                     'frontend_sources':'stock/frontend-sources.json','log':f'logs/v183-build-{rom}.log'},
            'binary':binary,'binary_sha256':sha(R/binary),
            'settings_library_sha256':sha(R/Path(binary).parent/'libmsettings.so'),
            'installer':installer.name,'installer_sha256':sha(installer),'installer_bytes':installer.stat().st_size,
            'publisher_kit':publisher.name,'publisher_kit_sha256':sha(publisher),
            'checks':{name:{'sha256':sha(R/'logs'/name)} for name in common+names},
            'sleep_timer':{'saves_progress_and_settings':True,'full_app_exit':True,
                          'background_exit':True,'pause_screen_off_counted':True,
                          'new_daemon_added':False,'SIGKILL_used':False,'forced_suspend':False,
                          'sleep_policy':'Firmware configured idle/suspend settings after NXT cleanup'},
            'features_retained_from_v182':True,'native_layouts':36,'system_languages':14,
            'hardware_tested':False,'physical_Bluetooth_USB_DAC_tested':False,
            'physical_battery_temperature_measured':False,'physical_sleep_wakeup_tested':False,
            'authenticated_online_playback_in_this_build':False,'OTA_public_source_published':False,
            'native_loader_reference':'Official Brick Pro Stock v1.1.1-20260717 rootfs; NextUI payload tested on same dependencies, not a running NextUI OS',
            'firmware_reference':json.loads((R/'STOCK_SOURCE.json').read_text())['official_reference']['firmware'],
            'baseline_source_sha256':baseline['baseline_source_sha256'],
            'NXRedux_v182_binary_unchanged':True,'verification_notes':f'RELEASE_QA_v{VERSION}.md',
            'created_UTC':datetime.datetime.now(datetime.timezone.utc).isoformat(),
        }
        (R/f'BUILD_REPORT_v{VERSION}_{label}.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    p=R/'SOURCE.json';meta=json.loads(p.read_text())
    meta['stage']='v1.8.3-native-packaged'
    meta['stock_status']='Fresh v1.8.3 Stock/NextUI installers and OTA publisher kits; full timer exit tested'
    meta['hardware_test_status']='Software regression and ARM64 loader tests passed; physical playback, battery/temperature and suspend/wakeup remain to test on device'
    p.write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
    print('PASS current Stock/NextUI builds, exact installers/OTA kits, source fingerprints and current-release QA evidence.')

if __name__=='__main__':main()
