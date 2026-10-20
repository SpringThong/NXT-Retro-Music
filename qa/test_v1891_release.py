from pathlib import Path
import os,subprocess,tempfile,shutil,json,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
includes=[S,S/'include',H,H/'zip/lib',H/'utf8proc',H/'samplerate/src',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SDK/'usr/include/SDL2']
with tempfile.TemporaryDirectory(prefix='nxt-1891-release-') as td:
 w=Path(td);(w/'data').mkdir();hdr=w/'headers';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2');b=w/'timer'
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NXREDUX','-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(w/'data')+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-I'+str(hdr),*['-I'+str(p) for p in includes],*map(str,[R/'qa/v1891_sleep_timer.c',S/'nxt_config.c',S/'nxt_write.c',S/'file_utils.c',S/'include/parson/parson.c']),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v1891-timer-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b)],capture_output=True,text=True,timeout=30,env={**os.environ,'NXT_SESSION_SOCKET':str(w/'session.sock'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'})
 (R/'logs/v1891-timer-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode()
 (R/'logs/v1891-timer-test.json').write_text(json.dumps({'exit':0,'full_cleanup_on_expiry':True,'deadline_cancel_replace_rollover_pause_online':True,'ASan_UBSan':True,'session_socket_mocked':True,'physical_deepsleep_tested':False},indent=2)+'\n')
 integration=w/'app/integration';shutil.copytree(R/'nxredux/integration',integration,ignore=shutil.ignore_patterns('backup'))
 system=w/'system';(system/'bin').mkdir(parents=True);original=Path('/workspace/scratch/d7cab07fb248/nx-reference-v1891/runtime/.system/bin/nextui.elf');target=system/'bin/nextui.elf';shutil.copy2(original,target)
 m=json.loads((integration/'icon-integration.json').read_text());assert sha(target)==m['original_sha256'] and sha(integration/'nextui-v2.1.2-nxt-icon.elf')==m['patched_sha256']
 env={**os.environ,'SYSTEM_PATH':str(system)};script=integration/'install-tools-icon.sh'
 subprocess.run(['sh',str(script)],env=env,check=True);assert sha(target)==m['patched_sha256'];assert sha(integration/'backup/nextui-v2.1.2-original.elf')==m['original_sha256'];assert (system/'res/menu/menu_icon_nxt.png').is_file()
 subprocess.run(['sh',str(script)],env=env,check=True);subprocess.run(['sh',str(script),'--restore'],env=env,check=True);assert sha(target)==m['original_sha256'];assert not (system/'res/menu/menu_icon_nxt.png').exists()
 target.write_bytes(b'OTHER CUSTOM ROM LAUNCHER');p=subprocess.run(['sh',str(script)],env=env);assert p.returncode and target.read_bytes()==b'OTHER CUSTOM ROM LAUNCHER'
 (R/'logs/v1891-icon-test.json').write_text(json.dumps({'exit':0,'exact_official_v212_guard':True,'atomic_install':True,'backup_idempotence_restore':True,'unknown_launcher_preserved':True,'patched_launcher_sha256':m['patched_sha256'],'hardware_tested':False},indent=2)+'\n')
print('PASS timer cleanup and exact-version Tools icon install/rollback.')
