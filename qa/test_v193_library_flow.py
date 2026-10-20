from pathlib import Path
import os,tempfile,subprocess,json,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';H=R/'.cache/nxredux-build/headers';HI=R/'.cache/v193-host-include'
ROM=os.environ.get('QA_ROM','nxredux').lower();assert ROM in ['stock','nextui','nxredux']
with tempfile.TemporaryDirectory(prefix='nxt193-flow-') as td:
 w=Path(td);hdr=w/'include';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2');incs=[hdr,HI,H,H/'utf8proc',S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings'];b=w/'flow'
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+ROM.upper(),'-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*['-I'+str(i) for i in incs],str(R/'qa/v193_library_flow.c'),str(S/'nxt_library_index.c'),'-Wl,--gc-sections','-l:libsqlite3.so.0','-lpthread','-o',str(b)]
 p=subprocess.run(cmd,capture_output=True,text=True);(R/f'logs/v193-library-flow-build-{ROM}.log').write_text(p.stdout+p.stderr);assert not p.returncode,p.stderr
 music=w/'Music';(music/'Album').mkdir(parents=True);(music/'one.flac').write_text('one');(music/'Album/two.dff').write_text('two');data=w/'data';data.mkdir();output=''
 for warm in [0,1]:
  p=subprocess.run([str(b),str(data),str(music),str(warm)],capture_output=True,text=True,timeout=25,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});output+=p.stdout+p.stderr;assert not p.returncode,p.stdout+p.stderr
 (R/f'logs/v193-library-flow-{ROM}.log').write_text(output);(R/f'logs/v193-library-flow-{ROM}.json').write_text(json.dumps({'rom':ROM,'exit':0,'ASan_UBSan':True,'actual_production_scheduler_and_library_views':True,'cold_and_warm_processes':True,'cached_rows_available_before_scan':True,'no_scan_on_reentry':True,'one_query_per_unchanged_view':True,'folders_not_reloaded_each_idle_frame':True,'index_generation_refresh':True,'hidden_pause_and_upload_debounce':True,'favorites_and_recursive_scope':True,'hardware_tested':False,'source_sha256':hashlib.sha256((S/'nxt_library.c').read_bytes()).hexdigest()},indent=2)+'\n');print(output)
