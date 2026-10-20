#!/usr/bin/env python3
"""Load release ELFs under the pinned SDK libc. No physical-device emulation."""
from pathlib import Path
import hashlib,json,os,socket,subprocess,tempfile
R=Path(__file__).resolve().parents[1];meta=json.loads((R/'SOURCE.json').read_text());ROM=os.environ.get('NXT_TEST_ROM','stock');LABEL='Stock' if ROM=='stock' else 'NextUI';B=R/('stock/bin/tg5040' if ROM=='stock' else 'upstream/bin/tg5040');helpers=R/'upstream/bin/tg5040';libc=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
base=[str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(libc),'-E','LD_LIBRARY_PATH='+':'.join(map(str,[B,helpers,libc/'lib',libc/'usr/lib']))];env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')};result={}
for executable in [B/"musicplayer.elf",helpers/"nxt-spotify-backend",helpers/"nxt-soundcloud-backend"]:executable.chmod(0o755)
for args in [['--build-info'],['--help']]:
 p=subprocess.run([*base,str(B/'musicplayer.elf'),*args],capture_output=True,text=True,timeout=15,env=env);print(p.stderr,flush=True);p.check_returncode();assert LABEL in p.stdout if args==['--build-info'] else '--build-info' in p.stdout;result[str(args)]={'exit':p.returncode,'stdout':p.stdout[:1600]}
relinked=R/('dist/stock-relinked/musicplayer.elf' if ROM=='stock' else 'dist/relinked/musicplayer.elf')
if relinked.is_file():
 p=subprocess.run([*base,str(relinked),'--build-info'],capture_output=True,text=True,timeout=15,env=env);print(p.stderr,flush=True);p.check_returncode();assert LABEL in p.stdout;result['relinked_frontend']={'exit':0,'stdout':p.stdout[:1600]}
with tempfile.TemporaryDirectory() as td:
 for name in ['nxt-spotify-backend','nxt-soundcloud-backend']:
  receiver_socket,sender_socket=socket.socketpair(socket.AF_UNIX,socket.SOCK_SEQPACKET);receiver=receiver_socket.fileno();sender=sender_socket.fileno()
  def fd3():
   if sender!=3:os.dup2(sender,3)
  be={**env,'NXT_SPOTIFY_CREDENTIALS':td+'/missing-spotify.json','NXT_SPOTIFY_PLAYBACK_CREDENTIALS':td+'/missing-playback.json','NXT_SOUNDCLOUD_CREDENTIALS':td+'/missing-cloud.json','NXT_SOUNDCLOUD_PCM_FD':'3','NXT_SPOTIFY_PCM_FD':'3'}
  p=subprocess.run([*base,str(helpers/name)],capture_output=True,text=True,input='{"cmd":"quit"}\n',cwd=R/'upstream',timeout=15,env=be,pass_fds=tuple({receiver,sender}),preexec_fn=fd3);receiver_socket.close();sender_socket.close();print(name,p.returncode,p.stdout[:1000],p.stderr[:1000],flush=True);p.check_returncode();result[name]={'exit':0,'stdout':p.stdout[:1000],'stderr':p.stderr[:600]}
report={'version':meta['version'],'rom':ROM,'exit':0,'actual_release_ELFs':True,'binary_sha256':hashlib.sha256((B/'musicplayer.elf').read_bytes()).hexdigest(),'runtime':'Pinned SDK ARM64 libc; QEMU user-mode loader and quit checks','checks':result,'physical_hardware_tested':False,'authenticated_playback_tested':False};(R/'logs'/('v182-arm64-'+ROM+'-test.json')).write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(json.dumps(report,ensure_ascii=False,indent=2))
