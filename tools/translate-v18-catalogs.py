#!/usr/bin/env python3
"""Build-time translation only. No model or translation service runs on the device."""
from pathlib import Path
import urllib.request,json,hashlib,concurrent.futures,re,os
R=Path(__file__).resolve().parents[1];model=R/'.cache/v18-mt';model.mkdir(parents=True,exist_ok=True)
repo='JustFrederik/m2m_100_418m_ct2_int8'
def get(url):return urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NXT-v18-catalog-build'}),timeout=90).read()
provenance_file=R/'design/v1.8/translation-provenance.json'
previous=json.loads(provenance_file.read_text()) if provenance_file.exists() else None
if previous:
 revision=previous['revision'];files=[x['file'] for x in previous['files']]
 expected={x['file']:x['sha256'] for x in previous['files']}
else:
 info=json.loads(get('https://huggingface.co/api/models/'+repo));revision=info['sha']
 files=[x['rfilename'] for x in info['siblings'] if x['rfilename'].endswith(('.json','.bin','.model','.txt'))];expected={}
def fetch(file):
 p=model/file;p.parent.mkdir(parents=True,exist_ok=True)
 if not p.exists():p.write_bytes(get('https://huggingface.co/'+repo+'/resolve/'+revision+'/'+file))
 sha=hashlib.sha256(p.read_bytes()).hexdigest()
 if file in expected and sha!=expected[file]:raise ValueError('Pinned translation model hash mismatch: '+file)
 return {'file':file,'sha256':sha}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as ex:hashes=list(ex.map(fetch,files))
(R/'design/v1.8/translation-provenance.json').write_text(json.dumps({'model':repo,'revision':revision,'build_only':True,'files':hashes},indent=2))
import ctranslate2,sentencepiece
sp=sentencepiece.SentencePieceProcessor(model_file=str(next(model.glob('*.model'))));translator=ctranslate2.Translator(str(model),device='cpu',compute_type='int8',inter_threads=1,intra_threads=4)
strings=json.loads((R/'design/v1.8/locale-source.json').read_text());out=R/'design/v1.8/locale-generated.json';data=json.loads(out.read_text()) if out.exists() else {}
# printf placeholders and physical button names are kept literal.
protected=re.compile(r'(%(?:\d+\$)?[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[sdifuxX%]|\b(?:NXT|SDL|USB|DAC|EQ|DSP|RGB|HEX|Wi-Fi|Bluetooth|Spotify|SoundCloud|PayPal|SELECT|START|L1|R1|L2|R2|A|B|X|Y)\b)')
parts={s:protected.split(s) for s in strings};segments=list(dict.fromkeys(p.strip() for ps in parts.values() for i,p in enumerate(ps) if i%2==0 and p.strip() and re.search('[A-Za-z]{2}',p)))
for lang,code in [('fr','fr'),('ru','ru'),('ko','ko'),('ja','ja'),('th','th'),('zh-Hans','zh'),('zh-Hant','zh'),('de','de'),('es','es'),('pt-BR','pt'),('it','it'),('id','id')]:
 pending=[s for s in strings if s not in data.get(lang,{})]
 if not pending:continue
 pending_parts={s:protected.split(s) for s in pending}
 segments=list(dict.fromkeys(p.strip() for ps in pending_parts.values() for i,p in enumerate(ps) if i%2==0 and p.strip() and re.search('[A-Za-z]{2}',p)))
 translated={}
 for start in range(0,len(segments),48):
  batch=segments[start:start+48];tokens=[['__en__']+sp.encode(s,out_type=str)+['</s>'] for s in batch]
  results=translator.translate_batch(tokens,target_prefix=[['__'+code+'__']]*len(tokens),beam_size=1,max_decoding_length=220,max_batch_size=48)
  for text,res in zip(batch,results):translated[text]=sp.decode(res.hypotheses[0][1:])
 data.setdefault(lang,{})
 for s,ps in pending_parts.items():
  values=[]
  for i,p in enumerate(ps):
   key=p.strip();v=translated.get(key,key) if not i%2 else p
   if i%2==0 and p: v=p[:len(p)-len(p.lstrip())]+v+p[len(p.rstrip()):]
   values.append(v)
  data[lang][s]=''.join(values)
 if lang=='zh-Hant':
  from opencc import OpenCC
  convert=OpenCC('s2t').convert;data[lang]={k:convert(v) for k,v in data[lang].items()}
 out.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n');print(lang,len(data[lang]),flush=True)
