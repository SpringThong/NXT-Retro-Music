from pathlib import Path
import subprocess,tempfile,os,json,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'.host-tools/usr/include',R/'.cache/nextui18-build/headers/samplerate',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SDK/'usr/include/dbus-1.0',SDK/'usr/lib/dbus-1.0/include']]
with tempfile.TemporaryDirectory(prefix='nxt185-core-') as td:
 b=Path(td)/'core';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(R/'qa/v188_audio_core.c'),str(S/'nxt_route.c'),str(S/'nxt_pcm.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libasound.so.2','-l:libdbus-1.so.3','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v188-audio-core-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b)],env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=20);(R/'logs/v188-audio-core-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v188-audio-core-test.json').write_text(json.dumps({'exit':0,'actual_player_and_policy_handoff':True,'ASan_UBSan':True,'hardware_tested':False,'decoder_and_raw_PCM_fixtures':True,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_audio_mode.c','nxt_player_core.c']}},indent=2)+'\n')
