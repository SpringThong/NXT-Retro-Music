#!/usr/bin/env python3
"""Build a pinned PNG avatar catalogue; the target only decodes small PNGs."""
from pathlib import Path
import json,re,urllib.request,urllib.parse,hashlib,concurrent.futures,shutil,base64
ROOT=Path(__file__).resolve().parents[1]
def get(url):
 return urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NXT-v18-assets'}),timeout=45).read()
cache=ROOT/'.cache/v18-avatars';cache.mkdir(parents=True,exist_ok=True)
catalog_file=ROOT/'upstream/res/emoji3d/catalog.json'
existing=json.loads(catalog_file.read_text()) if catalog_file.exists() else {'items':[]}
commit=existing.get('commit','1ffb34c752ecf5d402f04cfb4b392c77f57c54bc')
treefile=cache/'tree.json'
if not treefile.exists():treefile.write_bytes(get('https://api.github.com/repos/microsoft/fluentui-emoji/git/trees/'+commit+'?recursive=1'))
tree=json.loads(treefile.read_bytes())
if tree['sha']!=commit:raise ValueError('Pinned Fluent Emoji tree mismatch')
files=[x['path'] for x in tree['tree'] if '/3D/' in x['path'] and x['path'].endswith('.png') and '/Default/' not in x['path']]
files.sort(key=lambda p:(not any(k in p.lower() for k in ['face','cat','heart','music','headphone','moon','star','rocket','flower']),p))
import emoji
legacy_source=(ROOT/'upstream/src/nxt_custom.c').read_text()
legacy_match=re.search(r'static const char \*emoji\[\]=\{(.*?)\};',legacy_source,re.S)
old=json.loads('['+legacy_match[1]+']')
def normal(s):return re.sub('[^a-z0-9]','',s.lower())
byname={normal(p.split('/')[1]):p for p in files}
chosen=[]
for ch in old:
 name=emoji.EMOJI_DATA.get(ch,emoji.EMOJI_DATA.get(ch+'\ufe0f',{})).get('en','').strip(':')
 p=byname.get(normal(name))
 if not p:raise RuntimeError('Missing legacy emoji '+ch+' '+name)
 chosen.append(p)
chosen += [p for p in files if p not in chosen][:180-len(chosen)]
if len(existing['items'])==180:chosen=[x['source'] for x in existing['items']]
for item in existing['items']:
 p=ROOT/'upstream/res/emoji3d'/f"{item['id']:03}.png"
 store=cache/(hashlib.sha256(item['source'].encode()).hexdigest()+'.png')
 if p.exists() and not store.exists():shutil.copy2(p,store)

out=ROOT/'upstream/res/emoji3d';out.mkdir(parents=True,exist_ok=True)
def fetch(pair):
 i,p=pair;target=out/f'{i:03}.png'
 store=cache/(hashlib.sha256(p.encode()).hexdigest()+'.png')
 if not store.exists():store.write_bytes(get('https://raw.githubusercontent.com/microsoft/fluentui-emoji/'+commit+'/'+urllib.parse.quote(p)))
 from PIL import Image
 with Image.open(store) as im:im.convert('RGBA').resize((160,160),Image.Resampling.LANCZOS).save(target)
 return {'id':i,'name':p.split('/')[1],'source':p,'sha256':hashlib.sha256(target.read_bytes()).hexdigest()}
with concurrent.futures.ThreadPoolExecutor(max_workers=8) as ex:items=list(ex.map(fetch,enumerate(chosen)))
(out/'catalog.json').write_text(json.dumps({'repository':'https://github.com/microsoft/fluentui-emoji','commit':commit,'items':items},ensure_ascii=False,indent=2)+'\n')
license=ROOT/'upstream/licenses/fluent-emoji-MIT.txt';license.parent.mkdir(parents=True,exist_ok=True);license.write_bytes(get('https://raw.githubusercontent.com/microsoft/fluentui-emoji/'+commit+'/LICENSE'))
legacyfile=ROOT/'upstream/src/nxt_custom.c'
legacy=re.search(r'static const char \*icons\[\]=\{(.*?)\};',legacyfile.read_text(),re.S)
if legacy:names=json.loads('['+legacy[1]+']')
else:names=json.loads((ROOT/'upstream/res/icons/v18-catalog.json').read_text())[:64]
design=json.loads((ROOT/'design/v1.8/icon-map.json').read_text())
names+= [k for k in design if k not in names][:300-len(names)]
for k in names:
 if k in design:(ROOT/'upstream/res/icons'/f'{k}.png').write_bytes(base64.b64decode(design[k].split(',',1)[1]))
(ROOT/'upstream/res/icons/v18-catalog.json').write_text(json.dumps(names,ensure_ascii=False)+'\n')
(ROOT/'upstream/src/nxt_v18_icons.inc').write_text('static const char *icons[]={'+','.join(json.dumps(n) for n in names)+'};\n')
(ROOT/'upstream/src/nxt_v18_emojis.inc').write_text('static const char *emoji_names[]={'+','.join(json.dumps(n['name']) for n in items)+'};\n')
print('Prepared',len(names),'icons and',len(items),'3D emoji at',commit,flush=True)
