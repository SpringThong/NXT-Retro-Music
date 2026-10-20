from pathlib import Path
import subprocess,tempfile,os,json
r=Path(__file__).resolve().parents[1];s=r/'upstream/src';q=r/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [s,s/'include',r/'upstream/NextUI/workspace/all/common',r/'upstream/NextUI/workspace/tg5040/platform',r/'upstream/NextUI/workspace/tg5040/libmsettings']]
results={}
with tempfile.TemporaryDirectory(prefix='nxt195-logic-') as td:
 for name,extra in [('lyrics',[str(s/'lyric_window.c'),str(s/'nxt_lyric_words.c'),str(q/'utf8proc.o')]),('motion',[str(s/'nxt_motion.c')])]:
  exe=Path(td)/name;cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(r/f'qa/v1975_{name}.c'),*extra,'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-lpthread','-lm','-o',str(exe)]
  with (r/f'logs/v1975-{name}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  p=subprocess.run([str(exe),td],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=60);(r/f'logs/v1975-{name}-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode();results[name]={'exit':0,'production_code':True,'ASan_UBSan':True,'physical_FPS_tested':False}
(r/'logs/v1975-motion-lyrics.json').write_text(json.dumps(results,indent=2)+'\n')
