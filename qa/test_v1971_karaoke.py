from pathlib import Path
import os,subprocess,json,tempfile
r=Path(__file__).resolve().parents[1];s=r/'upstream/src';q=r/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [s,s/'include',r/'upstream/NextUI/workspace/all/common',r/'upstream/NextUI/workspace/tg5040/platform',r/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt196-karaoke-') as td:
 w=Path(td);fake=w/'bin';fake.mkdir();curl=fake/'curl';curl.write_text('''#!/usr/bin/env python3
import sys,json,os,time
cfg=sys.stdin.read()
assert 'audio/transcriptions' in cfg and 'timestamp_granularities[]=word' in cfg and 'whisper-large-v3' in cfg
assert 'TEST_ONLY_KEY' not in ' '.join(sys.argv)
mode=os.environ.get('QA_MODE','ok')
if mode=='quota':print('{}'+'\\nNXT_STATUS:429',end='');sys.exit(0)
if mode=='oversize':print('x'*600000,end='');sys.exit(0)
if mode=='slow':time.sleep(5)
print(json.dumps({'words':[{'word':'Hello','start':1,'end':1.4},{'word':'world','start':1.5,'end':2}]}))
print('NXT_STATUS:200',end='')
''');curl.chmod(0o755);exe=w/'test'
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(r/'qa/v1971_karaoke.c'),str(s/'nxt_lyric_words.c'),str(s/'include/parson/parson.c'),str(s/'include/mbedtls_lib/sha256.c'),str(s/'include/mbedtls_lib/platform_util.c'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(exe)]
 with (r/'logs/v1971-karaoke-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(exe),td],capture_output=True,text=True,env={**os.environ,'QA_DATA':td,'PATH':str(fake)+':'+os.environ['PATH'],'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=60);(r/'logs/v1971-karaoke-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode()
(r/'logs/v1971-karaoke-test.json').write_text(json.dumps({'exit':0,'production_worker_mapping_cache_transport':True,'partial_normalized_alignment':True,'off_no_audio_upload':True,'explicit_consent_default_cancel':True,'quota_oversize_and_slow_cancel':True,'online_path_does_not_analyse_old_local_file':True,'offline_cache_no_key':True,'generation_cancellation_reaping':True,'ASan_UBSan':True,'real_Groq_singing_tested':False,'PCM_and_HTTP_fixtures':True},indent=2)+'\n')
