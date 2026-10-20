#!/usr/bin/env python3
from pathlib import Path
import json,os,subprocess,tempfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
results={}
for rom in ['nxredux','stock','nextui']:
 with tempfile.TemporaryDirectory(prefix='nxt-v186-timer-') as td:
  w=Path(td);data=w/'data';data.mkdir();binary=w/'timer'
  cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom.upper(),'-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,*map(str,[R/'qa/v186_sleep_timer.c',S/'nxt_config.c',S/'nxt_write.c',S/'file_utils.c',S/'include/parson/parson.c']),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-lpthread','-lm','-o',str(binary)]
  with (R/f'logs/v186-timer-build-{rom}.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  p=subprocess.run([str(binary)],capture_output=True,text=True,timeout=20,env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'NXT_SESSION_SOCKET':td+'/timer.sock','ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
  (R/f'logs/v186-timer-test-{rom}.log').write_text(p.stdout+p.stderr);print(rom,p.stdout+p.stderr);p.check_returncode();results[rom]={'exit':0,'full_exit_on_expiry':True,'ASan_UBSan':True}
(R/'logs/v186-timer-test.json').write_text(json.dumps({'results':results,'hardware_sleep_tested':False,'sleep_policy':'Firmware configured idle/suspend policy after full NXT cleanup; Stock owned awake flags released','sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_config.c','nxt_session.c']}},indent=2)+'\n')
