#!/usr/bin/env python3
"""Generate indexed, offline Chinese and Japanese candidates from pinned dictionaries."""
from pathlib import Path
import json,urllib.request,hashlib,sqlite3,re
from pypinyin import lazy_pinyin
from opencc import OpenCC
R=Path(__file__).resolve().parents[1];C=R/'sources/ime';C.mkdir(parents=True,exist_ok=True);out=R/'upstream/res/ime';out.mkdir(parents=True,exist_ok=True);license=R/'upstream/licenses/ime';license.mkdir(parents=True,exist_ok=True)
def get(url):return urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NXT-IME-build'}),timeout=90).read()
PINNED={x['repository']:x['commit'] for x in json.loads((C/'sources.json').read_text())['sources']}
EXPECTED={(x['repository'],x['path']):x['sha256'] for x in json.loads((C/'sources.json').read_text())['sources']}
def file(repo,path):
 rev=PINNED[repo];p=C/repo.replace('/','-')/path;p.parent.mkdir(parents=True,exist_ok=True)
 if not p.exists():p.write_bytes(get('https://raw.githubusercontent.com/'+repo+'/'+rev+'/'+path))
 sha=hashlib.sha256(p.read_bytes()).hexdigest()
 if (repo,path) in EXPECTED and sha!=EXPECTED[(repo,path)]:raise ValueError('Pinned IME source hash mismatch: '+repo+'/'+path)
 return p,{'repository':repo,'commit':rev,'path':path,'sha256':sha}
db=sqlite3.connect(out/'candidates.sqlite3');db.executescript('DROP TABLE IF EXISTS candidates;CREATE TABLE candidates(lang TEXT,key TEXT,value TEXT,score INTEGER,PRIMARY KEY(lang,key,value)) WITHOUT ROWID;')
provenance=[];simplify=OpenCC('t2s').convert
p,source=file('rime/rime-luna-pinyin','luna_pinyin.dict.yaml');provenance.append(source)
rows=[]
for line in p.read_text().splitlines():
 parts=line.split('\t')
 if len(parts)>=2 and re.fullmatch('[a-z ]+',parts[1]):
  word,key=parts[:2];key=key.replace(' ','');score=int(float(parts[2].strip('%'))) if len(parts)>2 and re.fullmatch(r'[0-9.]+%?',parts[2]) else 1
  rows.extend([('zh-Hant',key,word,score),('zh-Hans',key,simplify(word),score)])
p,source=file('rime/rime-essay','essay.txt');provenance.append(source)
for line in p.read_text().splitlines():
 parts=line.split('\t')
 if len(parts)<2:continue
 word=parts[0]
 if len(word)>16 or not word or not all('\u3400'<=c<='\u9fff' for c in word):continue
 key=''.join(lazy_pinyin(word));score=int(parts[1]) if parts[1].isdigit() else 1
 rows.extend([('zh-Hant',key,word,score),('zh-Hans',key,simplify(word),score)])
 if len(rows)>20000:db.executemany('INSERT OR IGNORE INTO candidates VALUES(?,?,?,?)',rows);rows=[]
db.executemany('INSERT OR IGNORE INTO candidates VALUES(?,?,?,?)',rows)
p,source=file('skk-dev/dict','SKK-JISYO.L');provenance.append(source);raw=p.read_bytes().decode('euc_jp',errors='replace');rows=[]
(license/'SKK-dictionary-notice.txt').write_text('\n'.join(line for line in raw.splitlines() if line.startswith(';'))+'\n')
for line in raw.splitlines():
 if not line or line.startswith(';') or ' /' not in line:continue
 key,words=line.split(' /',1)
 if any('a'<=c<='z' for c in key):continue
 for i,word in enumerate(words.split('/')):
  word=word.split(';',1)[0]
  if word and len(word)<60 and not word.startswith('('):rows.append(('ja',key,word,10000-i))
db.executemany('INSERT OR IGNORE INTO candidates VALUES(?,?,?,?)',rows);db.execute('CREATE INDEX candidate_lookup ON candidates(lang,key,score DESC)');db.commit();db.execute('VACUUM');count=db.execute('SELECT COUNT(*) FROM candidates').fetchone()[0];db.close()
for repo in ['rime/rime-luna-pinyin','rime/rime-essay']:
 try:p,s=file(repo,'LICENSE');(license/(repo.replace('/','-')+'-LICENSE')).write_bytes(p.read_bytes())
 except Exception:p,s=file(repo,'LICENSE.txt');(license/(repo.replace('/','-')+'-LICENSE')).write_bytes(p.read_bytes())
(out/'sources.json').write_text(json.dumps({'rows':count,'sources':provenance},indent=2)+'\n');print('Indexed',count,'offline candidates',flush=True)
