#!/usr/bin/env python3
"""Explicit fake server; validates the real worker's config, never calls Groq."""
import io,json,os,sys,re,runpy,time
cfg=sys.stdin.read()
if 'api.groq.com' not in cfg:
 sys.stdin=io.StringIO(cfg);exec(compile('#!/usr/bin/env python3\n"""Explicit translation-server fixture; all production curl arguments/stdin run."""\nimport json,os,sys,re,urllib.parse,time\nleak=os.environ.get(\'QA_LEAK_FD\')\nif leak:\n try:os.fstat(int(leak));raise AssertionError(\'player descriptor inherited by curl\')\n except OSError:pass\ncfg=sys.stdin.read()\nassert \'-K\' in sys.argv and \'-\' in sys.argv\nassert all(\'FAKE_TEST_KEY_188\' not in x for x in sys.argv)\nurl=re.search(r\'^url = "(.*)"$\',cfg,re.M).group(1)\nif \'googleapis.com\' in url:\n encoded=re.search(r\'^data = "(.*)"$\',cfg,re.M).group(1)\n req=json.loads(json.loads(\'"\'+encoded+\'"\'))\n source=req.get(\'source\',\'auto\');target=req[\'target\'];text=req[\'q\'];cloud=True\nelse:\n req=urllib.parse.parse_qs(urllib.parse.urlsplit(url).query)\n text=req[\'q\'][0];source,target=req[\'langpair\'][0].split(\'|\');cloud=False\n assert len(text.encode())<=480\nwith open(os.environ[\'QA_REQUESTS\'],\'a\') as f:f.write(json.dumps({\'source\':source,\'target\':target,\'text\':text,\'google\':cloud,\'env_tag\':os.environ.get(\'QA_ENV_TAG\',\'\')},ensure_ascii=False)+\'\\n\')\nmode=open(os.environ[\'QA_CONTROL\']).read()\nif mode==\'network\':sys.exit(7)\nif mode==\'slow\':time.sleep(10)\nif mode==\'huge\':print(\'X\'*50000);sys.exit(0)\nif mode==\'malformed\':print(\'{invalid JSON\\nNXT_HTTP_STATUS:200\',end=\'\');sys.exit(0)\nif mode==\'quota\':\n print(json.dumps({\'responseStatus\':429,\'quotaFinished\':True})+\'\\nNXT_HTTP_STATUS:429\',end=\'\');sys.exit(0)\nif mode==\'googleerror\':\n print(json.dumps({\'error\':{\'code\':403,\'message\':\'fixture permission denied\'}})+\'\\nNXT_HTTP_STATUS:403\',end=\'\');sys.exit(0)\ntexts=text if isinstance(text,list) else text.split(\'\\n\')\ntranslated=\'\\n\'.join(target+\': \'+line for line in texts)\nif os.environ.get(\'QA_TRANSLATION_VISUAL\')==\'1\' and target==\'vi\':\n translated=\'\\n\'.join({\'The morning light is warm.\':\'Ánh sáng buổi sáng thật ấm áp.\',\'Today is a peaceful day.\':\'Hôm nay là một ngày bình yên.\'}.get(line,target+\': \'+line) for line in texts)\nif mode==\'flatten\':translated=translated.replace(\'\\n\',\' \')\nif mode==\'onebad\' and any(\'Bad-line\' in line for line in texts):\n print(\'{invalid JSON\\nNXT_HTTP_STATUS:200\',end=\'\');sys.exit(0)\nif cloud:reply={\'data\':{\'translations\':[{\'translatedText\':line} for line in translated.split(\'\\n\')]}}\nelse:reply={\'responseStatus\':200,\'responseData\':{\'translatedText\':translated},\'quotaFinished\':False}\nprint(json.dumps(reply,ensure_ascii=False)+\'\\nNXT_HTTP_STATUS:200\',end=\'\')\n\n','<legacy translation fixture>','exec'));sys.exit(0)
assert all('FAKE_GROQ_SECRET_189' not in x for x in sys.argv)
assert '--include' in sys.argv and '-K' in sys.argv
assert 'Authorization: Bearer FAKE_GROQ_SECRET_189' in cfg
leak=os.environ.get('QA_LEAK_FD')
if leak:
 try:os.fstat(int(leak));raise AssertionError('player descriptor inherited')
 except OSError:pass
encoded=re.search(r'^data = "(.*)"$',cfg,re.M).group(1)
req=json.loads(json.loads('"'+encoded+'"'))
assert req['response_format']['type']=='json_schema' and req['response_format']['json_schema']['strict'] is True
assert req['stream'] is False and req['max_completion_tokens']==2048
assert req['model'] in ['openai/gpt-oss-20b','openai/gpt-oss-120b','qwen/qwen3.8-27b']
assert req['reasoning_effort']==('none' if req['model'].startswith('qwen/') else 'low')
payload=json.loads(req['messages'][1]['content']);lines=payload['lines'];target=payload['target_language']
assert 1<=len(lines)<=24 and sum(len(x['text'].encode()) for x in lines)+len(lines)-1<=2400
with open(os.environ['QA_REQUESTS'],'a') as f:f.write(json.dumps({'groq':True,'source':payload['source_language'],'target':target,'model':req['model'],'text':[x['text'] for x in lines],'cfg_bytes':len(cfg)},ensure_ascii=False)+'\n')
mode=open(os.environ['QA_CONTROL']).read()
def emit(value,http=200,headers=''):
 print('HTTP/1.1 200 Connection established\r\n\r\nHTTP/2 '+str(http)+'\r\nContent-Type: application/json\r\n'+headers+'\r\n'+json.dumps(value,ensure_ascii=False)+'\nNXT_HTTP_STATUS:'+str(http),end='')
if mode=='slow':time.sleep(10)
if mode=='network':sys.exit(7)
if mode=='groqquota':emit({'error':{'message':'fixture rate limit'}},429,'Retry-After: 2\r\nx-ratelimit-remaining-requests: 999\r\nx-ratelimit-reset-requests: 1h\r\nx-ratelimit-remaining-tokens: 0\r\nx-ratelimit-reset-tokens: 2s\r\n');sys.exit(0)
if mode=='groqday':emit({'error':{'message':'fixture daily limit'}},429,'x-ratelimit-remaining-requests: 0\r\nx-ratelimit-reset-requests: 1h2m\r\n');sys.exit(0)
if mode=='groqkey':emit({'error':{'message':'fixture denied'}},401);sys.exit(0)
if mode=='groqmodel':emit({'error':{'message':'fixture unavailable model'}},404);sys.exit(0)
translations=[{'id':x['id'],'text':target+': '+x['text']} for x in reversed(lines)]
if mode=='groqduplicate' and len(translations)>1:translations[0]['id']=translations[1]['id']
if mode=='groqmissing':translations=translations[:-1]
if mode=='groqbadid':translations[0]['id']=0.5
if mode=='groqempty':translations[0]['text']=''
if mode=='groqmalformed':content='{bad JSON'
else:content=json.dumps({'translations':translations},ensure_ascii=False)
emit({'choices':[{'finish_reason':'length' if mode=='groqtruncated' or (mode=='groqtruncatedgroup' and len(lines)>1) else 'stop','message':{'role':'assistant','content':content}}]})
