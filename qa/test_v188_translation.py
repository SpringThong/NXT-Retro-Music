from pathlib import Path
import hashlib,json,os,subprocess,tempfile,shutil
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',S/'include/mbedtls_lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-v188-translation-') as td:
 w=Path(td);data=w/'data';data.mkdir();bin_=w/'bin';bin_.mkdir();shutil.copy2(R/'qa/fake_translation_curl.py',bin_/'curl');(bin_/'curl').chmod(0o755)
 control=w/'control';control.write_text('ok');requests=w/'requests.jsonl'
 b=w/'test';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-Werror=implicit-function-declaration','-Werror=int-conversion','-fsanitize=address,undefined',*incs,str(R/'qa/v188_translation.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),str(S/'include/mbedtls_lib/platform_util.c'),str(Q/'utf8proc.o'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v188-translation-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 result=subprocess.run([str(b)],cwd=R/'upstream',env={**os.environ,'QA_DATA':str(data),'QA_CONTROL':str(control),'QA_REQUESTS':str(requests),'PATH':str(bin_)+':'+os.environ['PATH'],'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=90)
 (R/'logs/v188-translation-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');(R/'logs/v188-translation-fixture-requests.jsonl').write_text(requests.read_text() if requests.exists() else '');result.check_returncode()
 logged=[json.loads(x) for x in requests.read_text().splitlines()]
 assert all(len(x['text'].encode())<=480 for x in logged)
 assert 'FAKE_TEST_KEY_188' not in requests.read_text()
 report={'exit':0,'production_worker_curl_and_cache':True,'explicit_server_and_lyrics_fixtures':True,'default_off_zero_requests':True,'target_languages':14,'requested_English_Chinese_Korean_Japanese':True,'UTF8_bytes_per_chunk':480,'dedup_cache_offline':True,'immutable_original_timestamps':True,'quota_HTTP_JSON_oversize_auth_errors':True,'cooldown_across_tracks':True,'manual_retry':True,'hidden_screenoff_pause':True,'generation_cancel':True,'bounded_quit_reaps_child':True,'Google_key_private_stdin':True,'ASan_UBSan':True,'actual_service_success_tested':False,'physical_device_tested':False,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_translate.c','nxt_translate.h','lyrics.c','lyrics.h']}}
 (R/'logs/v188-translation-test.json').write_text(json.dumps(report,indent=2)+'\n')

