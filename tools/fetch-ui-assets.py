#!/usr/bin/env python3
"""Bundle offline typefaces and the approved NXT / Lucide visual assets."""
from pathlib import Path
import concurrent.futures as futures
import hashlib, json, urllib.request, importlib.util, re, tarfile
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
import cairosvg

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('fetch_inputs',ROOT/'tools/fetch-build-assets.py')
# Keep downloading independent of the command-line build-input fetcher.
def fetch(url,target):
    target=Path(target);target.parent.mkdir(parents=True,exist_ok=True)
    if target.is_file() and target.stat().st_size>0:return target
    req=urllib.request.Request(url,headers={'User-Agent':'NXT-Retro-Music-build/0.3'})
    with urllib.request.urlopen(req,timeout=90) as response,open(str(target)+'.part','wb') as output:
        while chunk:=response.read(1024*1024):
            view=memoryview(chunk)
            while view:view=view[output.write(view):]
    Path(str(target)+'.part').replace(target);return target
def getjson(url):
    req=urllib.request.Request(url,headers={'User-Agent':'NXT-Retro-Music-build/0.3'})
    with urllib.request.urlopen(req,timeout=60) as response:return json.load(response)
cache=ROOT/'.cache/fonts';cache.mkdir(parents=True,exist_ok=True)
commit_file=cache/'google-fonts-commit.txt'
if not commit_file.exists():commit_file.write_text(getjson('https://api.github.com/repos/google/fonts/commits/main')['sha'])
commit=commit_file.read_text().strip()
families=[('Noto Sans','notosans'),('Inter','inter'),('Roboto','roboto'),('Source Sans 3','sourcesans3'),('Nunito Sans','nunitosans'),('Manrope','manrope'),('Rubik','rubik'),('Ubuntu','ubuntu'),('Lato','lato'),('Montserrat','montserrat'),('Noto Serif','notoserif'),('Source Serif 4','sourceserif4'),('Lora','lora'),('Merriweather','merriweather'),('JetBrains Mono','jetbrainsmono'),('Fira Code','firacode')]
fallback=[('Noto Sans Thai','notosansthai'),('Noto Sans Lao','notosanslao'),('Noto Sans Khmer','notosanskhmer'),('Noto Sans Devanagari','notosansdevanagari'),('Noto Naskh Arabic','notonaskharabic'),('Noto Sans Symbols 2','notosanssymbols2')]
fontroot=ROOT/'overlay/res/fonts';fontroot.mkdir(parents=True,exist_ok=True)
licenses=ROOT/'overlay/licenses/fonts';licenses.mkdir(parents=True,exist_ok=True)
def font(item):
    name,family=item
    listing_file=cache/(family+'.json')
    if listing_file.exists():listing=json.loads(listing_file.read_text())
    else:
        directory='ufl' if family=='ubuntu' else 'ofl'
        base=f'https://raw.githubusercontent.com/google/fonts/{commit}/{directory}/{family}/'
        metadata=fetch(base+'METADATA.pb',cache/(family+'-metadata.pb')).read_text()
        filenames=re.findall(r'filename:\s*"([^"]+\.ttf)"',metadata)
        listing=[{'name':name,'download_url':base+urllib.parse.quote(name)} for name in filenames]
        license_name='UFL.txt' if family=='ubuntu' else 'OFL.txt'
        listing.append({'name':license_name,'download_url':base+license_name})
        listing_file.write_text(json.dumps(listing))
    candidates=[p for p in listing if p['name'].endswith('.ttf') and 'Italic' not in p['name']]
    selected=next((p for p in candidates if 'Regular' in p['name']),candidates[0])
    raw=fetch(selected['download_url'],cache/selected['name'])
    out=fontroot/(family+'.ttf')
    if not out.exists():
        face=TTFont(raw)
        if 'fvar' in face:
            values={axis.axisTag:(400 if axis.axisTag=='wght' else axis.defaultValue) for axis in face['fvar'].axes}
            face=instantiateVariableFont(face,values,inplace=True)
        face.save(out)
    license_entry=next((p for p in listing if p['name'] in ('OFL.txt','LICENSE.txt','UFL.txt')),None)
    if license_entry:fetch(license_entry['download_url'],licenses/(family+'.txt'))
    cmap=TTFont(out,lazy=True).getBestCmap()
    return {'name':name,'file':'./res/fonts/'+out.name,'glyphs':len(cmap),'sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'source':selected['download_url'],'license':str((licenses/(family+'.txt')).relative_to(ROOT/'overlay'))}
icons=['headphones','house','library','list-music','heart','radio','podcast','youtube','settings-2','info','wifi','wifi-off','bluetooth','usb','battery-full','battery-medium','battery-low','battery-charging','play','pause','skip-back','skip-forward','shuffle','repeat','repeat-1','timer','timer-off','captions','sliders-horizontal','ellipsis','volume-2','volume-x','folder','folder-open','mic-2','disc-3','search','refresh-cw','plus','minus','check','x','arrow-left','arrow-up','arrow-down','chevron-right','chevron-left','sun-moon','type','languages','gamepad-2','lightbulb','music-2','lock-keyhole','unlock-keyhole','power','moon','file-input','file-output','pencil','list-start','list-end','list-plus','monitor-off','book-open']
def icon(name):
    source={'mic-2':'mic-vocal','unlock-keyhole':'lock-keyhole-open'}.get(name,name)
    try:raw=fetch(f'https://unpkg.com/lucide-static@0.468.0/icons/{source}.svg',ROOT/'.cache/icons'/(name+'.svg'))
    except Exception as exc:raise RuntimeError('Icon download failed: '+name) from exc
    out=ROOT/'overlay/res/icons'/(name+'.png');out.parent.mkdir(parents=True,exist_ok=True)
    svg=raw.read_text().replace('currentColor','#ffffff')
    cairosvg.svg2png(bytestring=svg.encode(),write_to=str(out),output_width=96,output_height=96)
    return name
with futures.ThreadPoolExecutor(max_workers=6) as pool:
    jobs=[pool.submit(font,item) for item in families+fallback]
    records=[job.result() for job in jobs]
    for _ in pool.map(icon,icons):pass
cjk=fetch('https://raw.githubusercontent.com/notofonts/noto-cjk/Sans2.004/Sans/OTC/NotoSansCJK-Regular.ttc',fontroot/'noto-cjk.ttc')
fetch('https://raw.githubusercontent.com/notofonts/noto-cjk/main/Sans/LICENSE',licenses/'noto-cjk.txt')
fetch('https://unpkg.com/lucide-static@0.468.0/LICENSE',ROOT/'overlay/licenses/Lucide-ISC.txt')
for filename,basename in [('unifont-16.0.04.otf','unifont.otf'),('unifont_upper-16.0.04.otf','unifont-upper.otf')]:
    fetch('https://unifoundry.com/pub/unifont/unifont-16.0.04/font-builds/'+filename,fontroot/basename)
archive=fetch('https://unifoundry.com/pub/unifont/unifont-16.0.04/unifont-16.0.04.tar.gz',ROOT/'sources/unifont-16.0.04.tar.gz')
with tarfile.open(archive) as source:
    for entry in source.getmembers():
        if entry.name.endswith('/COPYING'):
            (licenses/'unifont-GPL-font-exception.txt').write_bytes(source.extractfile(entry).read());break
logo=Path('/workspace/scratch/766af1647819/deliverables/nxt-retro-music-logo.svg')
if logo.exists():
    for name,size in [('icon-brick.png',128),('icon-brickpro.png',128),('icon-trimui.png',128),('nxt-logo.png',256)]:
        cairosvg.svg2png(url=str(logo),write_to=str(ROOT/'overlay/res'/name),output_width=size,output_height=size)
catalog={'google_fonts_commit':commit,'primary':records[:16],'fallback':records[16:],'cjk':{'file':'./res/fonts/noto-cjk.ttc','faces':{'ja':0,'ko':1,'zh-Hans':2,'zh-Hant':3},'sha256':hashlib.sha256(cjk.read_bytes()).hexdigest(),'release':'Sans2.004'}}
(fontroot/'catalog.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n')
print('Bundled',len(families),'primary fonts,',len(fallback)+1,'fallback families and',len(icons),'icons.')
