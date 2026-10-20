#!/usr/bin/env python3
"""Private Ubuntu host tools for build/QA; verify archive SHA256, no apt install."""
from pathlib import Path
import concurrent.futures, hashlib, json, lzma, subprocess, urllib.request
ROOT=Path(__file__).resolve().parents[1]
CACHE=ROOT/'.cache/downloads'; CACHE.mkdir(parents=True,exist_ok=True)
HOST=ROOT/'.host-tools'; HOST.mkdir(exist_ok=True)
names=set('7zip libasound2-dev libsdl2-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 libsdl2-image-dev libsdl2-image-2.0-0 libfreetype-dev libharfbuzz-dev libfribidi-dev libgl-dev libglx-dev libegl-dev libgles-dev libopengl-dev libopengl0 libgles1 libgles2 libsqlite3-dev libsamplerate0-dev libzip-dev libzip4t64 pkgconf-bin libpkgconf3 qemu-user libcapstone4 liburing2 libunwind8'.split())
def get(url,path,sha=None):
    if path.exists() and (sha is None or hashlib.sha256(path.read_bytes()).hexdigest()==sha):return
    with urllib.request.urlopen(url,timeout=45) as r,open(str(path)+'.part','wb') as f:
        while b:=r.read(1024*1024):f.write(b)
    temp=Path(str(path)+'.part')
    if sha and hashlib.sha256(temp.read_bytes()).hexdigest()!=sha:raise RuntimeError('SHA256 mismatch '+path.name)
    temp.replace(path)
records={}
for component in ('main','universe'):
    path=CACHE/f'noble-{component}-Packages.xz'
    get(f'https://archive.ubuntu.com/ubuntu/dists/noble/{component}/binary-amd64/Packages.xz',path)
    for entry in lzma.decompress(path.read_bytes()).decode().split('\n\n'):
        fields={}
        for line in entry.splitlines():
            if ': ' in line and not line.startswith(' '):
                k,v=line.split(': ',1);fields[k]=v
        if fields.get('Package') in names:records[fields['Package']]=fields
missing=names-records.keys()
if missing:raise RuntimeError('Missing package metadata '+repr(missing))
def fetch(f):
    path=CACHE/Path(f['Filename']).name
    get('https://archive.ubuntu.com/ubuntu/'+f['Filename'],path,f['SHA256'])
    subprocess.run(['dpkg-deb','-x',str(path),str(HOST)],check=True)
    print(f['Package'],f['Version'],flush=True)
    return {'package':f['Package'],'version':f['Version'],'sha256':f['SHA256'],'archive':path.name}
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as p:result=list(p.map(fetch,records.values()))
(ROOT/'logs/host-deps-v1.json').write_text(json.dumps(result,indent=2)+'\n')
