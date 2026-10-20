#!/usr/bin/env python3
"""Host production helper handshake/config/QR/bookmarks over the real IPC.
Uses no SoundCloud account or network bypass; no credentials are needed.
Pass the path to a HOST binary built with cargo build, never the ARM binary.
"""
from pathlib import Path
import json,os,select,socket,subprocess,sys,tempfile
binary=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory() as td:
    directory=Path(td);(directory/'soundcloud').mkdir()
    left,right=socket.socketpair(socket.AF_UNIX,socket.SOCK_SEQPACKET)
    def configure_fd():
        os.dup2(right.fileno(),3)
    process=subprocess.Popen([str(binary)],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,
        pass_fds=tuple({right.fileno(),3}),preexec_fn=configure_fd,
        env={**os.environ,'NXT_SOUNDCLOUD_PCM_FD':'3','NXT_SOUNDCLOUD_CREDENTIALS':str(directory/'soundcloud/credentials.json')})
    right.close()
    def read():
        if not select.select([process.stdout],[],[],5)[0]:raise AssertionError('helper did not respond')
        return json.loads(process.stdout.readline())
    def send(value):
        process.stdin.write(json.dumps(value,ensure_ascii=False)+'\n');process.stdin.flush();return read()
    assert read()['event']=='ready'
    response=send({'cmd':'login'});assert response['event']=='logged_in' and response['mode']=='public_unofficial'
    assert not (directory/'soundcloud/credentials.json').exists()
    response=send({'cmd':'source_link','url':'https://soundcloud.com/nxt-demo/track','uri':'soundcloud:tracks:1','stream_id':1});assert response['event']=='source_link'
    rows=response['qr'];assert 1<=len(rows)<=57 and all(len(row)==len(rows) and set(row)<=set('01') for row in rows)
    response=send({'cmd':'favorite','uri':'soundcloud:tracks:1','stream_id':1,'saved':True,'title':'Việt 한국어 日本語 中文 ไทย','artist':'Uploader','duration_ms':3000});assert response['event']=='favorite' and response['saved'] and response['local']
    file=directory/'soundcloud/bookmarks.json';assert file.stat().st_mode&0o777==0o600;assert json.loads(file.read_text())[0]['title']=='Việt 한국어 日本語 中文 ไทย'
    response=send({'cmd':'check_favorite','uri':'soundcloud:tracks:1','stream_id':2});assert response['saved']
    response=send({'cmd':'favorite','uri':'soundcloud:tracks:1','stream_id':2,'saved':False});assert not response['saved']
    process.stdin.write('{"cmd":"quit"}\n');process.stdin.flush();assert process.wait(timeout=3)==0
    assert 'client_secret' not in process.stderr.read();left.close()
print('PASS production Rust process: real inherited seqpacket/control IPC, key-free handshake, original-source QR, atomic private local bookmarks with UTF-8, no account login, graceful quit.')
