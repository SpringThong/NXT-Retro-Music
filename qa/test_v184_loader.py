from pathlib import Path
import subprocess,os,json,hashlib,struct,tempfile,socket
R=Path(__file__).resolve().parents[1];SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';helpers=R/'upstream/bin/tg5040';env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')}
results={}
for rom,label,dir_ in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NX Redux','nxredux')]:
 b=R/dir_/'bin/tg5040';binary=b/'musicplayer.elf';header=binary.read_bytes()[:64];assert header[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',header,18)[0]==183
 base=[str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(SDK),'-E','LD_LIBRARY_PATH='+':'.join(map(str,[b,helpers,SDK/'lib',SDK/'usr/lib']))];checks=[]
 for args in [['--build-info'],['--help']]:
  p=subprocess.run(base+[str(binary),*args],env=env,capture_output=True,text=True,timeout=15);p.check_returncode();assert '1.8.4' in p.stdout;assert label in p.stdout if args==['--build-info'] else '--build-info' in p.stdout;checks.append({'args':args,'exit':0,'stdout':p.stdout[:500]});print(rom,args,'PASS')
 result={'rom':rom,'version':'1.8.4','exit':0,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'actual_release_ELF':True,'runtime':'QEMU user-mode with pinned SDK ARM64 libc','checks':checks,'physical_ROM_boot_tested':False};results[rom]=result
 (R/f'logs/v184-arm64-{rom}-test.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
# Backend payloads are unmodified; verify their startup/quit with a real IPC pair.
backend_results={}
with tempfile.TemporaryDirectory() as td:
 for name in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  a,b=socket.socketpair(socket.AF_UNIX,socket.SOCK_SEQPACKET)
  def fd3():
   if b.fileno()!=3:os.dup2(b.fileno(),3)
  be={**env,'NXT_SPOTIFY_CREDENTIALS':td+'/missing.json','NXT_SPOTIFY_PLAYBACK_CREDENTIALS':td+'/missing2.json','NXT_SOUNDCLOUD_CREDENTIALS':td+'/missing3.json','NXT_SOUNDCLOUD_PCM_FD':'3','NXT_SPOTIFY_PCM_FD':'3'}
  p=subprocess.run(base+[str(helpers/name)],input='{"cmd":"quit"}\n',env=be,cwd=R/'upstream',capture_output=True,text=True,timeout=20,pass_fds=(a.fileno(),b.fileno()),preexec_fn=fd3);a.close();b.close();p.check_returncode();backend_results[name]={'exit':0,'sha256':hashlib.sha256((helpers/name).read_bytes()).hexdigest(),'stdout':p.stdout[:500]};print(name,'PASS')
(R/'logs/v184-ARM64-loader-test.json').write_text(json.dumps({'exit':0,'roms':results,'backend_quit_checks':backend_results,'authenticated_streaming_tested':False},ensure_ascii=False,indent=2)+'\n')
