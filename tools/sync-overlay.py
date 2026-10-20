#!/usr/bin/env python3
"""Refresh the current reviewed source/binary overlay, retaining baseline hashes."""
from pathlib import Path
import json,hashlib,shutil,os,re
r=Path(__file__).resolve().parents[1];p=r/'overlay-manifest.json';m=json.loads(p.read_text())
m['files']=[i for i in m['files'] if not any(part.startswith('.') for part in Path(i['path']).parts)]
known={i['path'] for i in m['files']}
additions=['NextUI/workspace/all/common/api.c']+[p.relative_to(r/'upstream').as_posix() for p in (r/'upstream/res/icons').glob('avatar-*.png')]
additions += ['src/nxt_wallpapers.c','src/nxt_wallpapers.h','src/nxt_wallpaper_store.c','src/nxt_wallpaper_store.h'] + [p.relative_to(r/'upstream').as_posix() for p in (r/'upstream/res/icons').glob('*.png')]
additions += [p.relative_to(r/'upstream').as_posix() for p in (r/'upstream/src').glob('nxt_*') if p.is_file()]
additions += [p.relative_to(r/'upstream').as_posix() for folder in ['res/themes/v18','res/locales','res/emoji3d','res/ime','licenses'] for p in (r/'upstream'/folder).rglob('*') if p.is_file()]
additions += [p.relative_to(r/'upstream').as_posix() for p in (r/'upstream/res/fonts').glob('*') if p.is_file() and p.name.startswith(('misans','harmonyos','opposans','robotocondensed','robotoslab','catalog-'))] + ['src/lyrics.h']
additions += ['src/keyboard.c','src/keyboard_map.c','src/musicplayer.c','src/Makefile','config.json','pak.json','state/app_version.txt','ota-source-nextui.url']
for path in additions:
    if any(part.startswith('.') for part in Path(path).parts):continue
    if path not in known:m['files'].append({'path':path,'sha256':''})
for i in m['files']:
    source=r/'upstream'/i['path'];target=r/'overlay'/i['path'];target.parent.mkdir(parents=True,exist_ok=True)
    temp=target.with_name(target.name+'.writing')
    with source.open('rb') as input,temp.open('wb') as output:
        shutil.copyfileobj(input,output);output.flush();os.fsync(output.fileno())
    shutil.copystat(source,temp);os.replace(temp,target)
    i['sha256']=hashlib.sha256(source.read_bytes()).hexdigest()
    assert hashlib.sha256(target.read_bytes()).hexdigest()==i['sha256'],i['path']
version=json.loads((r/'SOURCE.json').read_text())['version']
m['version']=version;p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n');(r/'upstream/.nxt-overlay-version').write_text(version+'\n')
print('Source and payload overlay synchronized.')
