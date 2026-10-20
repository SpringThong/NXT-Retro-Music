#!/usr/bin/env python3
"""Explicit translation-server fixture; all production curl arguments/stdin run."""
import json,os,sys,re,urllib.parse,time
leak=os.environ.get('QA_LEAK_FD')
if leak:
 try:os.fstat(int(leak));raise AssertionError('player descriptor inherited by curl')
 except OSError:pass
cfg=sys.stdin.read()
assert '-K' in sys.argv and '-' in sys.argv
assert all('FAKE_TEST_KEY_188' not in x for x in sys.argv)
url=re.search(r'^url = "(.*)"$',cfg,re.M).group(1)
if 'googleapis.com' in url:
 encoded=re.search(r'^data = "(.*)"$',cfg,re.M).group(1)
 req=json.loads(json.loads('"'+encoded+'"'))
 source=req.get('source','auto');target=req['target'];text=req['q'];cloud=True
else:
 req=urllib.parse.parse_qs(urllib.parse.urlsplit(url).query)
 text=req['q'][0];source,target=req['langpair'][0].split('|');cloud=False
 assert len(text.encode())<=480
with open(os.environ['QA_REQUESTS'],'a') as f:f.write(json.dumps({'source':source,'target':target,'text':text,'google':cloud,'env_tag':os.environ.get('QA_ENV_TAG','')},ensure_ascii=False)+'\n')
mode=open(os.environ['QA_CONTROL']).read()
if mode=='network':sys.exit(7)
if mode=='slow':time.sleep(10)
if mode=='huge':print('X'*50000);sys.exit(0)
if mode=='malformed':print('{invalid JSON\nNXT_HTTP_STATUS:200',end='');sys.exit(0)
if mode=='quota':
 print(json.dumps({'responseStatus':429,'quotaFinished':True})+'\nNXT_HTTP_STATUS:429',end='');sys.exit(0)
if mode=='googleerror':
 print(json.dumps({'error':{'code':403,'message':'fixture permission denied'}})+'\nNXT_HTTP_STATUS:403',end='');sys.exit(0)
texts=text if isinstance(text,list) else text.split('\n')
translated='\n'.join(target+': '+line for line in texts)
if os.environ.get('QA_TRANSLATION_VISUAL')=='1' and target=='vi':
 translated='\n'.join({'The morning light is warm.':'Ánh sáng buổi sáng thật ấm áp.','Today is a peaceful day.':'Hôm nay là một ngày bình yên.'}.get(line,target+': '+line) for line in texts)
if mode=='flatten':translated=translated.replace('\n',' ')
if mode=='onebad' and any('Bad-line' in line for line in texts):
 print('{invalid JSON\nNXT_HTTP_STATUS:200',end='');sys.exit(0)
if cloud:reply={'data':{'translations':[{'translatedText':line} for line in translated.split('\n')]}}
else:reply={'responseStatus':200,'responseData':{'translatedText':translated},'quotaFinished':False}
print(json.dumps(reply,ensure_ascii=False)+'\nNXT_HTTP_STATUS:200',end='')

