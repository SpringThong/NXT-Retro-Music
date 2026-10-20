#!/usr/bin/env python3
from pathlib import Path
import os,subprocess,tempfile,json,hashlib,zipfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
baseline=Path('/workspace/scratch/d7cab07fb248/v19-restored/NXT_Retro_Music_v1.8.9.1_NXRedux_Source.zip');assert sha(baseline)=='9f399e957ded04288a20138d01fa80aa9fc6cd18517454eec27f165d742bfeb7';historical={};preserved={}
with zipfile.ZipFile(baseline) as z:
 prefix='NXT_Retro_Music_v1.8.9.1_NXRedux_Source/';paths=[n[len(prefix):] for n in z.namelist() if n.startswith(prefix+'upstream/res/') and not n.endswith('/')]
 paths+=['stock/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/nxt-spotify-backend','upstream/bin/tg5040/nxt-soundcloud-backend','upstream/src/nxt_dsp.c','upstream/src/nxt_theme_v18.inc','upstream/src/nxt_ota.c','upstream/src/nxt_ota_package.c','upstream/src/nxt_startup.c','upstream/src/nxt_cpu.c','upstream/src/nxt_led.c','upstream/src/nxt_nxredux.c','upstream/src/nxt_keyfile.c','nxredux/integration/nextui-v2.1.2-nxt-icon.elf','nxredux/integration/install-tools-icon.sh']
 for path in paths:
  assert sha(R/path)==hashlib.sha256(z.read(prefix+path)).hexdigest(),path;preserved[path]=sha(R/path)
  if path in ['stock/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/musicplayer.elf']:historical[path]=preserved[path]
(R/'nxredux/bin/tg5040/musicplayer.elf').chmod(0o755)
env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[R/'nxredux/bin/tg5040',R/'upstream/bin/tg5040',SDK/'lib',SDK/'usr/lib']))};out=[]
for arg in ['--build-info','--help']:
 p=subprocess.run([str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(SDK),str(R/'nxredux/bin/tg5040/musicplayer.elf'),arg],env=env,capture_output=True,text=True,timeout=20);p.check_returncode();out.append(p.stdout+p.stderr)
assert '1.9' in out[0] and 'NX Redux' in out[0];(R/'logs/v19-loader.log').write_text('\n'.join(out));print(out[0])
with tempfile.TemporaryDirectory(prefix='nxt19-timer-') as td:
 w=Path(td);(w/'data').mkdir();hdr=w/'headers';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2');b=w/'timer';includes=[S,S/'include',H,H/'zip/lib',H/'utf8proc',H/'samplerate/src',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SDK/'usr/include/SDL2']
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NXREDUX','-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(w/'data')+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-I'+str(hdr),*['-I'+str(p) for p in includes],*map(str,[R/'qa/v1891_sleep_timer.c',S/'nxt_config.c',S/'nxt_write.c',S/'file_utils.c',S/'include/parson/parson.c']),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-lpthread','-lm','-o',str(b)]
 p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr;p=subprocess.run([str(b)],capture_output=True,text=True,timeout=30,env={**os.environ,'NXT_SESSION_SOCKET':str(w/'session.sock'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});assert p.returncode==0,p.stdout+p.stderr;(R/'logs/v19-timer.log').write_text(p.stdout+p.stderr);print(p.stdout)
(R/'logs/v19-release-test.json').write_text(json.dumps({'exit':0,'version':'1.9','baseline_version':'1.8.9.1','historical_frontends_preserved':historical,'unchanged_files':preserved,'real_ARM64_loader':True,'timer_full_cleanup':True,'session_transport_mocked':True,'ASan_UBSan_timer':True,'hardware_tested':False},indent=2)+'\n')
