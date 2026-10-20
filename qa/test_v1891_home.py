from pathlib import Path
import json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';CC=SDK.parents[1]/'bin/aarch64-nextui-linux-gnu-gcc'
incs=[H,S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',SDK/'usr/include',SDK/'usr/include/SDL2']
with tempfile.TemporaryDirectory(prefix='nxt-1891-home-') as td:
 b=Path(td)/'home';cmd=[str(CC),'-O1','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NXREDUX','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in incs],str(R/'qa/v1891_home_bridge.c'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v1891-home-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 env={**os.environ,'NX_MUSIC_SOCKET':td+'/control.sock','LD_LIBRARY_PATH':str(SDK/'lib')+':'+str(SDK/'usr/lib')}
 p=subprocess.run([str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(SDK),str(b)],env=env,capture_output=True,text=True,timeout=30);(R/'logs/v1891-home-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode()
(R/'logs/v1891-home-test.json').write_text(json.dumps({'exit':0,'production_ARM64_bridge':True,'protocol_version':10,'snapshot_and_frames':True,'waveform':True,'local_Spotify_SoundCloud_queue_controls':True,'music_gain_not_master_volume':True,'malformed_client_rejected':True,'owner_lock_and_cleanup':True,'Unix_socket_transport_mocked':True,'physical_device_tested':False},indent=2)+'\n')
