"""Exercise the real PC PKCE callback server and file write, with Spotify mocked."""
from pathlib import Path
import importlib.util,urllib.parse,urllib.request,urllib.error,tempfile,threading,sys,json,io,contextlib,os
p=Path(__file__).with_name('login-pc.py');s=importlib.util.spec_from_file_location('login',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
original=urllib.request.urlopen;worker=[]
class Response(io.BytesIO):
 def __enter__(self):return self
 def __exit__(self,*args):self.close()
seen={}
def fake_open(req,*args,**kwargs):
 if isinstance(req,urllib.request.Request) and req.full_url=='https://accounts.spotify.com/api/token':
  form=urllib.parse.parse_qs(req.data.decode());assert form['client_id']==['320f1ca055564990bcdf4de6fe58b397'];assert form['redirect_uri']==['http://127.0.0.1:8080/callback'];assert form['code']==['QA-CODE'];assert 'client_secret' not in form;assert len(form['code_verifier'][0])>=43;seen['exchange']=True
  return Response(json.dumps({'access_token':'QA-FAKE-TOKEN','refresh_token':'QA-FAKE-REFRESH','expires_in':3600}).encode())
 return original(req,*args,**kwargs)
def fake_browser(url):
 q=urllib.parse.parse_qs(urllib.parse.urlsplit(url).query);callback=q['redirect_uri'][0]
 def approve():
  try:original(callback+'?code=QA-CODE&state=INVALID',timeout=5)
  except urllib.error.HTTPError as e:assert e.code==400
  with original(callback+'?'+urllib.parse.urlencode({'code':'QA-CODE','state':q['state'][0]}),timeout=5) as f:assert f.status==200
 thread=threading.Thread(target=approve);thread.start();worker.append(thread);return True
m.webbrowser.open=fake_browser;m.urllib.request.urlopen=fake_open
with tempfile.TemporaryDirectory() as td:
 target=Path(td)/'credentials.json';target.write_text('old-file')
 old=sys.argv;sys.argv=['Spotify_Login.py','--output',str(target)];stdout=io.StringIO()
 try:
  with contextlib.redirect_stdout(stdout):m.main()
 finally:sys.argv=old
 for t in worker:t.join(5);assert not t.is_alive()
 data=json.loads(target.read_text());assert data['access_token']=='QA-FAKE-TOKEN';assert data['client_id']=='320f1ca055564990bcdf4de6fe58b397';assert data['obtained_at']>0;assert target.stat().st_mode&0o777==0o600
 assert 'QA-FAKE' not in stdout.getvalue();assert seen.get('exchange')
print('PASS actual PC login tool: registered 8080 callback, invalid state rejected, PKCE token exchange request, atomic credential replacement, private permissions, token-free console output; Spotify responses mocked.')
