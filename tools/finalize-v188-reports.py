from pathlib import Path
import hashlib,json,datetime,zipfile
R=Path(__file__).resolve().parents[1];V='1.8.8'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def passed(x):
 if isinstance(x,dict):
  for k,v in x.items():
   if k=='exit':assert v==0,(k,v)
   passed(v)
 elif isinstance(x,list):
  for v in x:passed(v)
checks=['v188-integer-decoder-test.json','v188-normal-audio-test.json','v188-bit-worker-test.json','v188-ui-test.json','v188-ui-nextui-test.json','v188-ui-nxredux-test.json','v188-config-test.json','v188-timer-test.json','v188-nextui-bluetooth-test.json','v188-ARM64-loader-test.json','v188-baseline-preserved.json','v188-new-features-test.json','v188-audio-core-test.json','v188-volume-test.json','v188-volume-keys-test.json','v188-stock-volume-audio-test.json','v188_spotify_ipc-test.json','v188_soundcloud_ipc-test.json','v188-drive-decode-test.json']
checks += ['v188-theme-catalog-test.json','v188-regression-runner.json','v188-translation-test.json','v188-lyric-revision-test.json']
for n in checks:passed(json.loads((R/'logs'/n).read_text()))
baseline=json.loads((R/'logs/v188-baseline-preserved.json').read_text())
for n,h in baseline['unchanged_files'].items():assert sha(R/n)==h,n
for n,h in baseline['intentional_native_changes'].items():assert sha(R/n)==h['after'],n
assert json.loads((R/'logs/v188-bit-worker-test.json').read_text())['source_sha256']==sha(R/'upstream/src/nxt_bitperfect.c')
for rom,label,directory in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=R/directory/'bin/tg5040/musicplayer.elf';inputs=json.loads((R/directory/f'BUILD_INPUTS_v{V}.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for n,h in inputs['sources'].items():assert sha(R/n)==h,n
 config=json.loads((R/directory/'config.json').read_text());assert config['version']==V and config['rom']==rom
 installer=R/f'dist/{rom}-release'/config['release_filename'];kit=installer.parent/f'NXT_OTA_Google_Drive_v{V}_{label}.zip'
 local=checks+[f'v188-ota-qa-{rom}.json',f'v188-ota-publisher-kit-{rom}.json',f'v188-arm64-{rom}-test.json']
 for n in local:passed(json.loads((R/'logs'/n).read_text()))
 ota=json.loads((R/f'logs/v188-ota-qa-{rom}.json').read_text());pub=json.loads((R/f'logs/v188-ota-publisher-kit-{rom}.json').read_text());assert sha(installer)==ota['installer_sha256']==pub['installer_sha256'];assert sha(kit)==pub['kit_sha256']
 for n,h in ota['OTA_source_sha256'].items():assert sha(R/'upstream/src'/n)==h,n
 settings='libnxt-nxredux-settings.so' if rom=='nxredux' else 'libmsettings.so'
 report={'project':'NXT Retro Music','version':V,'rom':rom,'device':'trimui-brick-pro','build':{'exit':0,'compiler':'pinned GCC 8.3.0 AArch64','all_source_inputs_verified':True,'log':f'logs/build-v188-{rom}.log'},'binary':binary.relative_to(R).as_posix(),'binary_sha256':sha(binary),'settings_library':settings,'settings_library_sha256':sha(binary.parent/settings),'installer':installer.name,'installer_sha256':sha(installer),'installer_bytes':installer.stat().st_size,'publisher_kit':kit.name,'publisher_kit_sha256':sha(kit),'checks':{n:{'sha256':sha(R/'logs'/n)} for n in local},'hardware_tested':False,'physical_battery_temperature_measured':False,'OTA_public_source_published':False,'verification_notes':f'RELEASE_QA_v{V}.md','baseline_source_sha256':baseline['baseline_source_sha256'],'created_UTC':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 (R/f'BUILD_REPORT_v{V}_{label}.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
meta=json.loads((R/'SOURCE.json').read_text());meta.update(stage='v1.8.8-native-packaged',baseline_version='1.8.7',stock_status='Fresh three-ROM v1.8.8 installers and OTA publisher kits; source inputs and exact package hashes verified',hardware_test_status='Translation/curl/cache/parser/UI/config and audio/CPUFreq/timer/OTA fixture checks passed. Successful live translation/Google key, physical thermal/battery/audio/sleep and public OTA remain untested.');(R/'SOURCE.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
out=R/'dist/release';out.mkdir(exist_ok=True)
for name in [f'HUONG_DAN_NXT_Retro_Music_v{V}.md',f'RELEASE_QA_v{V}.md']:
 source=R/('docs/'+name) if name.startswith('HUONG') else R/name
 import shutil
 shutil.copy2(source,out/name)
assert json.loads((R/'logs/v188-volume-test.json').read_text())['source_sha256']==sha(R/'upstream/src/nxt_volume.c')
with zipfile.ZipFile(out/f'NXT_Retro_Music_v{V}_QA.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted((R/'logs').glob('v188*')):
  if p.name not in {'v188-source-package.log','v188-finalize.log'}:z.write(p,'logs/'+p.name)
 for p in sorted((R/'qa/screens-v1.8.8').glob('*.png')):
  if p.name.startswith(('library-files-two-items','about-current','about-main-features','changes-current','changes-from-X','theme-gallery-last-page','new-theme-contact-sheet','translation-')) or any(p.name.startswith(f'theme-{i:02d}-playing') for i in range(36,50)):
   z.write(p,'native-screens/'+p.name)
 for rom,label,directory in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
  z.write(R/f'BUILD_REPORT_v{V}_{label}.json',f'BUILD_REPORT_v{V}_{label}.json');z.write(R/directory/f'BUILD_INPUTS_v{V}.json',directory+f'/BUILD_INPUTS_v{V}.json');z.write(R/f'logs/build-v188-{rom}.log',f'logs/build-v188-{rom}.log')
 z.write(R/f'RELEASE_QA_v{V}.md',f'RELEASE_QA_v{V}.md')
for name in ['cpu','audio-policy','writer']:
 evidence=json.loads((R/'logs/v188-new-features-test.json').read_text())[name]
 for n,h in evidence['production_source_sha256'].items():assert sha(R/'upstream/src'/n)==h,n
print('PASS final current source inputs, three new native binaries, installers, OTA kits and QA fingerprints')
