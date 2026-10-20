from pathlib import Path
import subprocess,os,json,tempfile
root=Path(__file__).resolve().parents[1];binary=root/'backend/target/release/nxt-spotify-backend'
with tempfile.TemporaryDirectory() as td:
 p=Path(td)/'credentials.json'
 diag=Path(td)/'NXT_Spotify_Log.txt'
 requests=['NOT JSON',json.dumps({'cmd':'pair'}),json.dumps({'cmd':'cancel_pair'}),json.dumps({'cmd':'play','uri':'spotify:track:0123456789012345678901'}),json.dumps({'cmd':'login'}),json.dumps({'cmd':'unknown'}),json.dumps({'cmd':'quit'})]
 run=subprocess.run([str(binary)],input='\n'.join(requests)+'\n',text=True,capture_output=True,timeout=5,env={**os.environ,'NXT_SPOTIFY_CREDENTIALS':str(p),'NXT_SPOTIFY_DIAG':str(diag)});assert run.returncode==0,run.stderr
 events=[json.loads(s) for s in run.stdout.splitlines()];assert events[0]['event']=='ready' and events[0]['build']=='NextUI-Spotify-Test5' and events[-1]['event']=='closed';errors=[e.get('reason') for e in events if e['event']=='error'];assert errors==['invalid_json','not_logged_in','pair_required','unknown_command'],errors
 assert 'Backend build: NextUI-Spotify-Test5' in diag.read_text() and 'cancelled' in diag.read_text();assert diag.stat().st_mode&0o777==0o600
 p.write_text(json.dumps({'client_id':'not-this-app','access_token':'FAKE-SECRET','refresh_token':'FAKE-SECRET'}))
 run=subprocess.run([str(binary)],input='{"cmd":"login"}\n{"cmd":"quit"}\n',text=True,capture_output=True,timeout=5,env={**os.environ,'NXT_SPOTIFY_CREDENTIALS':str(p)});assert 'pair_required' in run.stdout;assert 'FAKE-SECRET' not in run.stdout+run.stderr
 run=subprocess.run([str(binary)],input='',text=True,capture_output=True,timeout=5);assert run.returncode==0 and json.loads(run.stdout.splitlines()[-1])['event']=='closed'
 proc=subprocess.Popen([str(binary)],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 assert json.loads(proc.stdout.readline())['event']=='ready';proc.stdout.close();proc.stdin.write('{"cmd":"quit"}\n');proc.stdin.flush();assert proc.wait(timeout=5)==0;assert 'panicked' not in proc.stderr.read()
print('PASS production Rust process: JSON protocol, missing/wrong credentials, pairing independent of library login, cancellation, private diagnostics, tokens absent, quit/EOF/closed stdout; no account playback tested.')
