#!/usr/bin/env python3
"""Fetch pinned build inputs and record byte-level provenance."""
from pathlib import Path
import concurrent.futures as futures
import hashlib, json, subprocess, tarfile, urllib.request, time

ROOT=Path(__file__).resolve().parents[1]
CACHE=ROOT/'.cache/downloads'
CACHE.mkdir(parents=True,exist_ok=True)
HOST=ROOT/'.host-tools'
HOST.mkdir(exist_ok=True)
def fetch(url,target,expected=None):
    target=Path(target);target.parent.mkdir(parents=True,exist_ok=True)
    if target.is_file() and (expected is None or hashlib.sha256(target.read_bytes()).hexdigest()==expected):return target
    for attempt in range(3):
        try:
            req=urllib.request.Request(url,headers={'User-Agent':'NXT-Retro-Music-build/0.3'})
            with urllib.request.urlopen(req,timeout=90) as incoming,open(str(target)+'.part','wb') as output:
                while chunk:=incoming.read(1024*1024):
                    view=memoryview(chunk)
                    while view:view=view[output.write(view):]
            temp=Path(str(target)+'.part')
            if expected and hashlib.sha256(temp.read_bytes()).hexdigest()!=expected:raise ValueError('Checksum mismatch: '+target.name)
            temp.replace(target);return target
        except Exception:
            if attempt==2:raise
            time.sleep(attempt+1)

packages=['libsdl2-dev','libsdl2-ttf-dev','libsdl2-ttf-2.0-0','libsdl2-image-dev','libsdl2-image-2.0-0','libfreetype-dev','libharfbuzz-dev','libfribidi-dev','libgl-dev','libglx-dev','libegl-dev','libgles-dev','libopengl-dev','libsqlite3-dev','libsamplerate0-dev','libzip-dev','libzip4t64','pkgconf-bin','qemu-user']
def deb(name):
    data=subprocess.check_output(['/usr/bin/apt-cache','show','--no-all-versions',name],text=True).split('\n\n')[0]
    fields=dict(line.split(': ',1) for line in data.splitlines() if ': ' in line and not line.startswith(' '))
    filename=fields['Filename'];target=CACHE/Path(filename).name
    url='https://snapshot.ubuntu.com/ubuntu/20260828T000000Z/'+filename
    try:fetch(url,target,fields['SHA256'])
    except Exception:fetch('https://archive.ubuntu.com/ubuntu/'+filename,target,fields['SHA256'])
    subprocess.run(['dpkg-deb','-x',str(target),str(HOST)],check=True)
    return {'package':name,'version':fields['Version'],'sha256':fields['SHA256'],'file':target.name}

sources={
 'freetype-2.13.2':('https://codeload.github.com/freetype/freetype/tar.gz/refs/tags/VER-2-13-2','freetype'),
 'harfbuzz-8.3.0':('https://codeload.github.com/harfbuzz/harfbuzz/tar.gz/refs/tags/8.3.0','harfbuzz'),
 'SDL2_ttf-2.24.0':('https://codeload.github.com/libsdl-org/SDL_ttf/tar.gz/refs/tags/release-2.24.0','sdl-ttf'),
 'utf8proc-2.9.0':('https://codeload.github.com/JuliaStrings/utf8proc/tar.gz/refs/tags/v2.9.0','utf8proc'),
 'fribidi-1.0.13':('https://github.com/fribidi/fribidi/releases/download/v1.0.13/fribidi-1.0.13.tar.xz','fribidi'),
}
def source(item):
    name,(url,directory)=item;target=fetch(url,ROOT/'sources'/(name+('.tar.xz' if url.endswith('.xz') else '.tar.gz')))
    dest=ROOT/'.cache/src'/directory
    if not dest.exists():
        with tarfile.open(target) as archive:
            members=archive.getmembers();prefix=members[0].name.split('/')[0]+'/'
            for member in members:
                if member.name==prefix[:-1]:continue
                if member.name.startswith(prefix):member.name=member.name[len(prefix):]
                else:raise ValueError('Unexpected archive root')
                archive.extract(member,dest,filter='data')
    digest=hashlib.sha256(target.read_bytes()).hexdigest()
    target.with_name(target.name+'.sha256').write_text(digest+'  '+target.name+'\n')
    return {'name':name,'url':url,'sha256':digest,'file':target.name}

with futures.ThreadPoolExecutor(max_workers=6) as pool:
    jobs=[pool.submit(deb,p) for p in packages]+[pool.submit(source,s) for s in sources.items()]
    result=[]
    for job in futures.as_completed(jobs):
        item=job.result();result.append(item);print(item.get('name',item.get('package')),flush=True)
(ROOT/'.cache/build-inputs.json').write_text(json.dumps(result,indent=2)+'\n')
