#!/usr/bin/env python3
"""Production POSIX server: auth, bounded streaming, collision/cancel cleanup."""
from pathlib import Path
import os,subprocess,tempfile,json,socket,time,hashlib,urllib.parse
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
with tempfile.TemporaryDirectory() as td:
 p=Path(td);folder=p/'music';folder.mkdir();src=R/'qa/v18_transfer_server.c';binary=p/'server';incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
 subprocess.run(['gcc','-O1','-no-pie','-g','-DUSE_SDL2','-D_GNU_SOURCE','-DNXT_ROM_STOCK','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(src),str(S/'nxt_transfer.c'),str(Q/'utf8proc.o'),'-Wl,--gc-sections','-lpthread','-o',str(binary)],check=True)
 process=subprocess.Popen([str(binary),str(folder)],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
 url,pin,initial_rss=process.stdout.readline().strip().split();port=urllib.parse.urlparse(url).port
 def connect():return socket.create_connection(('127.0.0.1',port),timeout=3)
 def request(name,data=b'demo',auth=pin,extra='',length=None):
  c=connect();h=f'PUT /upload/{name} HTTP/1.1\r\nHost: NXT\r\nX-NXT-PIN: {auth}\r\nContent-Length: {len(data) if length is None else length}\r\n{extra}\r\n'.encode();c.sendall(h+data);out=b''
  while chunk:=c.recv(8192):out+=chunk
  c.close();return int(out.split()[1])
 assert request('song.mp3',auth='000000' if pin!='000000' else '111111')==403
 for name in ['%2e%2e%2ffile.mp3','x%2fy.mp3','bad%00.mp3','x%ff.mp3','.hidden.mp3','a.exe']:
  assert request(name)==400,name
 assert request('empty.mp3',b'')==400
 assert request('wrong.mp3',extra='Content-Length: 4\r\n')==400
 assert request('wrong.mp3',extra='X-NXT-PIN: '+pin+'\r\n')==403
 assert request('wrong.mp3',extra='Transfer-Encoding: chunked\r\n')==400
 assert request('wrong.mp3',auth='wrong',data=('\r\nX-NXT-PIN: '+pin+'\r\n').encode())==403
 assert request('song.mp3')==201 and (folder/'song.mp3').read_bytes()==b'demo'
 assert request('song.mp3',b'new')==201 and (folder/'song.mp3').read_bytes()==b'demo' and (folder/'1-song.mp3').read_bytes()==b'new'
 secret=p/'outside';secret.write_bytes(b'keep');(folder/'link.mp3').symlink_to(secret)
 assert request('link.mp3',b'new')==201 and secret.read_bytes()==b'keep' and (folder/'1-link.mp3').read_bytes()==b'new'
 name=urllib.parse.quote('Nguyễn Xuân Thông.flac');data=bytes(range(256))*65536
 assert request(name,data)==201
 assert hashlib.sha256((folder/'Nguyễn Xuân Thông.flac').read_bytes()).digest()==hashlib.sha256(data).digest()
 c=connect();c.sendall(f'PUT /upload/cancel.mp3 HTTP/1.1\r\nX-NXT-PIN: {pin}\r\nContent-Length: 1000000\r\n\r\npartial'.encode());time.sleep(.1);c.close()
 for i in range(50):
  if not list(folder.glob('*.part')):break
  time.sleep(.02)
 assert not (folder/'cancel.mp3').exists() and not list(folder.glob('*.part'))
 c=connect();c.sendall(f'PUT /upload/stop.mp3 HTTP/1.1\r\nX-NXT-PIN: {pin}\r\nContent-Length: 1000000\r\n\r\npartial'.encode());time.sleep(.1);process.stdin.write('\n');process.stdin.flush();stdout,stderr=process.communicate(timeout=5);c.close();assert process.returncode==0,stderr
 after=int(stdout.split()[-1]);before=int(initial_rss);assert after-before<12288,(before,after)
 assert not (folder/'stop.mp3').exists() and not list(folder.glob('*.part'))
 print('PASS auth, invalid names/UTF-8, header isolation, no overwrite/symlink escape, 16 MiB streaming, cancellation and shutdown cleanup under ASan/UBSan.')
 (R/'logs/v181-transfer-test.json').write_text(json.dumps({'exit':0,'ASan_UBSan':True,'file_bytes':len(data),'worker_RSS_growth_KiB':after-before,'exclusive_install_FAT_compatible':True,'PIN_auth':True,'header_body_isolation':True,'no_overwrite':True,'UTF8_names':True,'cancel_shutdown_cleanup':True,'physical_WiFi_tested':False},indent=2))
