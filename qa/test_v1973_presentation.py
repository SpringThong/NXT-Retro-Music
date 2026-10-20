from pathlib import Path
import subprocess,tempfile,os,json,zipfile
r=Path(__file__).resolve().parents[1];q=r/'.cache/render-sp';s=r/'upstream/src';v=r/'upstream/NextUI/workspace/all/common/generic_video.c'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [s,s/'include',r/'upstream/NextUI/workspace/all/common',r/'upstream/NextUI/workspace/tg5040/platform',r/'upstream/NextUI/workspace/tg5040/libmsettings']]
text=v.read_text();a=text.index('static void presentNativeFrame');b=text.index('\nvoid PLAT_animateSurfaceOpacity',a);production=text[a:b]
with tempfile.TemporaryDirectory() as td:
 p=Path(td);c=p/'present.c';c.write_text((r/'qa/v1973_present_fixture.c').read_text()+production+(r/'qa/v1973_present_test.inc').read_text());exe=p/'present'
 cmd=['gcc','-no-pie','-O1','-g','-DUSE_SDL2','-fsanitize=address,undefined',*incs,str(c),'-l:libSDL2-2.0.so.0','-o',str(exe)]
 with (r/'logs/v1973-presentation-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 out=subprocess.run([str(exe)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'});print(out.stdout+out.stderr);out.check_returncode()
 # The old whole-page movement violates the same no-displacement contract.
 with zipfile.ZipFile(r.parents[1]/'source-input/NXT_Retro_Music_v1.9.7.2_Source.zip') as z:(p/'old_motion.c').write_bytes(z.read('NXT_Retro_Music_v1.9.7.2_Source/upstream/src/nxt_motion.c'))
 cmd=['gcc','-no-pie','-O1','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP',*incs,str(r/'qa/v1973_motion.c'),str(p/'old_motion.c'),'-l:libSDL2-2.0.so.0','-lm','-o',str(p/'old')];subprocess.run(cmd,check=True)
 old=subprocess.run([str(p/'old')],capture_output=True,text=True);assert old.returncode!=0 and '!NxtMotion_frame' in old.stderr
(r/'logs/v1973-presentation.json').write_text(json.dumps({'exit':0,'production_SDL_native_compositor':True,'software_renderer':True,'full_and_layer_only_frames':True,'old_frame_poison_and_clip_target_viewport':True,'old_whole_page_transition_reproduced':True,'ASan_UBSan':True,'physical_GPU_driver_tested':False},indent=2)+'\n')
