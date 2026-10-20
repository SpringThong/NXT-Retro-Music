from pathlib import Path
import subprocess,json,hashlib,struct,random,os
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';W=R/'.cache/v1881/pcm';W.mkdir(parents=True,exist_ok=True)
SDK=R/'.nxt-sdk/tg5040/compiler';SYS=SDK/'aarch64-nextui-linux-gnu/libc';H=R/'.cache/stock-build/headers';L=R/'relink/lib'
I=[H,H/'ffmpeg',S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SYS/'usr/include/SDL2']
cc=[str(SDK/'bin/aarch64-nextui-linux-gnu-gcc'),'-O2','-DNXT_ROM_STOCK','-DUSE_SDL2','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in I],str(R/'qa/v1881_decoder.c'),str(S/'nxt_decoder.c'),'-Wl,--gc-sections',*[str(L/'ffmpeg'/f'{n}.a') for n in ['libavformat','libavcodec','libswresample','libavutil']],'-lm','-lpthread','-lz','-o',str(W/'decoder')]
subprocess.run(cc,check=True)
qemu=R/'.host-tools/usr/bin/qemu-aarch64';env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')};prefix=[str(qemu),'-L',str(SYS)]
results=[];rng=random.Random(184)
for rate,bits,ch in [(44100,16,2),(48000,24,2),(96000,24,1),(192000,32,2),(44100,16,1)]:
 name=f'{rate}-{bits}-{ch}';raw=W/(name+'.raw');width=bits//8;vals=[-(1<<(bits-1)),(1<<(bits-1))-1,-1,0,1,2,-2];samples=vals+ [rng.randrange(-(1<<(bits-1)),1<<(bits-1)) for _ in range(rate*ch-len(vals))]
 raw.write_bytes(b''.join((n & ((1<<bits)-1)).to_bytes(width,'little') for n in samples));wav=W/(name+'.wav')
 subprocess.run(['ffmpeg','-y','-v','error','-f',f's{bits}le','-ar',str(rate),'-ac',str(ch),'-i',str(raw),'-c:a',f'pcm_s{bits}le',str(wav)],check=True)
 files=[wav]
 if bits<32:
  for codec,suffix in [('flac','.flac'),('alac','.m4a')]:
   dest=W/(name+suffix);subprocess.run(['ffmpeg','-y','-v','error','-i',str(wav),'-c:a',codec,str(dest)],check=True);files.append(dest)
 for path in files:
  p=subprocess.run(prefix+[str(W/'decoder'),str(path),str(raw),'unused',str(rate),str(bits),str(ch),'accept'],env=env,capture_output=True,text=True,timeout=20);print(p.stdout+p.stderr,end='');p.check_returncode();results.append({'file':path.name,'exit':0,'reference_pcm_sha256':hashlib.sha256(raw.read_bytes()).hexdigest(),'exact_integer_decode':True,'exact_native_seek':True})
mp3=W/'lossy.mp3';subprocess.run(['ffmpeg','-y','-v','error','-i',str(W/'44100-16-2.wav'),str(mp3)],check=True)
p=subprocess.run(prefix+[str(W/'decoder'),str(mp3),'unused','unused','44100','16','2','reject'],env=env,capture_output=True,text=True,timeout=20);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v1881-integer-decoder-test.json').write_text(json.dumps({'exit':0,'actual_ARM64_decoder':True,'cases':results,'lossy_source_rejected':True,'physical_DAC_tested':False},indent=2)+'\n')
