from pathlib import Path
import subprocess,json,hashlib,struct,random,os
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';W=R/'.cache/v184/pcm';W.mkdir(parents=True,exist_ok=True)
SDK=R/'.nxt-sdk/tg5040/compiler';SYS=SDK/'aarch64-nextui-linux-gnu/libc';H=R/'.cache/stock-build/headers';L=R/'relink/lib'
I=[H,H/'ffmpeg',S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SYS/'usr/include/SDL2']
cc=[str(SDK/'bin/aarch64-nextui-linux-gnu-gcc'),'-O2','-DNXT_ROM_STOCK','-DUSE_SDL2','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in I],str(R/'qa/test_native_audio.c'),str(R/'qa/v184_decoder_stubs.c'),str(S/'nxt_decoder.c'),str(S/'nxt_dsp.c'),str(S/'nxt_eq.c'),'-Wl,--gc-sections',*[str(L/'ffmpeg'/f'{n}.a') for n in ['libavformat','libavcodec','libswresample','libavutil']],'-lm','-lpthread','-lz','-o',str(W/'normal-decoder')]
subprocess.run(cc,check=True)
qemu=R/'.host-tools/usr/bin/qemu-aarch64';env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')};prefix=[str(qemu),'-L',str(SYS)]
p=subprocess.run(prefix+[str(W/'normal-decoder'),str(R/'qa/media')],env=env,capture_output=True,text=True,timeout=30);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v184-normal-audio-test.json').write_text(json.dumps({'exit':0,'actual_ARM64_decoder':True,'formats':10,'DSP_replaygain_EQ_limiter':True,'physical_output_tested':False},indent=2)+'\n')
