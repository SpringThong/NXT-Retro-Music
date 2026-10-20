#!/usr/bin/env python3
"""Authorize the user's own Spotify app via PKCE; import credentials to SD later.
No client secret/password is requested or persisted. Loopback callback is PC-only.
"""
import argparse,base64,hashlib,http.server,json,os,secrets,time,urllib.parse,urllib.request,webbrowser

def make_request(client_id,redirect):
 verifier=secrets.token_urlsafe(64)
 challenge=base64.urlsafe_b64encode(hashlib.sha256(verifier.encode()).digest()).rstrip(b'=').decode()
 state=secrets.token_urlsafe(32)
 query=urllib.parse.urlencode({'client_id':client_id,'response_type':'code','redirect_uri':redirect,'code_challenge_method':'S256','code_challenge':challenge,'state':state,'scope':'streaming user-read-private playlist-read-private playlist-read-collaborative user-library-read user-library-modify'})
 return verifier,state,'https://accounts.spotify.com/authorize?'+query

def callback(query,wanted_state):
 p=urllib.parse.parse_qs(query,keep_blank_values=True)
 if p.get('state')!=[wanted_state]:raise ValueError('Invalid OAuth state')
 if 'error' in p:raise ValueError('Authorization declined')
 if len(p.get('code',[]))!=1 or not p['code'][0]:raise ValueError('Missing authorization code')
 return p['code'][0]

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--client-id',default='320f1ca055564990bcdf4de6fe58b397');ap.add_argument('--port',type=int,default=8080);ap.add_argument('--output',default='spotify-credentials.json');a=ap.parse_args()
 if len(a.client_id)!=32 or any(c not in '0123456789abcdefABCDEF' for c in a.client_id):ap.error('Client ID must be 32 hex characters')
 redirect=f'http://127.0.0.1:{a.port}/callback';verifier,state,url=make_request(a.client_id,redirect)
 result={}
 class Handler(http.server.BaseHTTPRequestHandler):
  def log_message(self,*args):pass # never log code-bearing request URL
  def do_GET(self):
   u=urllib.parse.urlsplit(self.path)
   if u.path!='/callback':self.send_error(404);return
   try:result['code']=callback(u.query,state)
   except ValueError as e:self.send_error(400,str(e));return
   self.send_response(200);self.send_header('Content-Type','text/plain; charset=utf-8');self.end_headers();self.wfile.write('Đã xác nhận. Bạn có thể đóng cửa sổ này.'.encode())
 with http.server.HTTPServer(('127.0.0.1',a.port),Handler) as server:
  server.timeout=1
  print('Registered Redirect URI:',redirect)
  print('Opening Spotify login in browser. Complete within 5 minutes.')
  webbrowser.open(url);deadline=time.monotonic()+300
  while 'code' not in result and time.monotonic()<deadline:server.handle_request()
 if 'code' not in result:raise SystemExit('Login timed out')
 data=urllib.parse.urlencode({'client_id':a.client_id,'grant_type':'authorization_code','code':result['code'],'redirect_uri':redirect,'code_verifier':verifier}).encode()
 req=urllib.request.Request('https://accounts.spotify.com/api/token',data=data,headers={'Content-Type':'application/x-www-form-urlencoded'})
 try:
  with urllib.request.urlopen(req,timeout=20) as f:tokens=json.load(f)
 except urllib.error.HTTPError as e:raise SystemExit(f'Spotify token exchange failed (HTTP {e.code}); no token saved')
 if not tokens.get('access_token') or not tokens.get('refresh_token'):raise SystemExit('Spotify did not issue expected tokens')
 tokens.update({'client_id':a.client_id,'obtained_at':int(time.time())})
 tmp=a.output+".tmp-"+secrets.token_hex(8)
 fd=os.open(tmp,os.O_CREAT|os.O_EXCL|os.O_WRONLY,0o600)
 with os.fdopen(fd,'w') as f:json.dump(tokens,f)
 os.replace(tmp,a.output)
 print('Credentials saved locally. Do not share this file or include it in project ZIPs.')
if __name__=='__main__':main()
