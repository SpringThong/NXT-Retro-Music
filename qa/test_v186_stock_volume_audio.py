from pathlib import Path
import subprocess,tempfile,os,json,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'.host-tools/usr/include',R/'.cache/stock-build/headers/samplerate',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SDK/'usr/include/dbus-1.0',SDK/'usr/lib/dbus-1.0/include']]
with tempfile.TemporaryDirectory(prefix='nxt186-stock-gain-') as td:
 b=Path(td)/'gain';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(R/'qa/v186_stock_volume_audio.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libsamplerate.so.0','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v186-stock-volume-audio-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b)],env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=20);(R/'logs/v186-stock-volume-audio-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v186-stock-volume-audio-test.json').write_text(json.dumps({'exit':0,'real_Stock_output_callback':True,'hardware_volume_not_double_attenuated':True,'existing_software_volume_retained':True,'ASan_UBSan':True,'hardware_tested':False,'source_sha256':hashlib.sha256((S/'nxt_player_core.c').read_bytes()).hexdigest()},indent=2)+'\n')
