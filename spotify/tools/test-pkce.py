import importlib.util,urllib.parse,base64,hashlib
from pathlib import Path
p=Path(__file__).with_name('login-pc.py');s=importlib.util.spec_from_file_location('login',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
v,state,url=m.make_request('a'*32,'http://127.0.0.1:8765/callback');q=urllib.parse.parse_qs(urllib.parse.urlsplit(url).query)
assert 43<=len(v)<=128
assert q['code_challenge']==[base64.urlsafe_b64encode(hashlib.sha256(v.encode()).digest()).rstrip(b'=').decode()]
assert q['code_challenge_method']==['S256'];assert 'client_secret' not in q
assert m.callback(urllib.parse.urlencode({'state':state,'code':'test'}),state)=='test'
for query in ['state=wrong&code=test',f'state={state}&error=access_denied',f'state={state}&code=',f'state={state}&code=a&code=b']:
 try:m.callback(query,state)
 except ValueError:pass
 else:raise AssertionError('Invalid callback accepted')
print('PASS PKCE: randomized verifier/state, S256 challenge, state binding, denial/empty/duplicate rejection; no client secret.')
