from pathlib import Path
import hashlib,json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
flags=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-DPLATFORM_DESKTOP','-fsanitize=address,undefined','-Werror=implicit-function-declaration','-Werror=int-conversion',*incs]
env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
results={}
with tempfile.TemporaryDirectory(prefix='nxt185-new-') as td:
 w=Path(td);cpu=w/'cpufreq';cpu.mkdir()
 for name,sources,extra,args in [
 ('cpu',[R/'qa/v187_cpu.c',S/'nxt_cpu.c'],['-DNXT_CPU_TEST',f'-DNXT_CPU_ROOT="{cpu}"'],[]),
 ('audio-policy',[R/'qa/v187_audio_policy.c',S/'nxt_audio_mode.c'],[],[]),
 ('writer',[R/'qa/v187_writer.c',S/'nxt_write.c'],['-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-lpthread'],[str(w)])]:
  binary=w/name
  with (R/f'logs/v187-{name}-build.log').open('w') as f:subprocess.run(flags+list(map(str,sources))+extra+['-o',str(binary)],stdout=f,stderr=f,check=True)
  p=subprocess.run([str(binary),*args],env=env,capture_output=True,text=True,timeout=40)
  (R/f'logs/v187-{name}-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode();results[name]={'exit':0,'ASan_UBSan':True,'production_source_sha256':{s.name:hashlib.sha256(s.read_bytes()).hexdigest() for s in sources if s.parent==S}}
results['hardware_tested']=False
(R/'logs/v187-new-features-test.json').write_text(json.dumps(results,indent=2)+'\n')
