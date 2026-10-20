#!/usr/bin/env python3
from pathlib import Path
import os,subprocess,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';w=R/'.cache/v161-live';w.mkdir(exist_ok=True);(w/'files').mkdir(exist_ok=True)
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/'.cache/nextui16-build/headers/samplerate',R/'.host-tools/usr/include']]
cmd=['gcc','-O1','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections',*incs,str(R/'qa/v161_wallpaper_live.c'),str(S/'nxt_wallpaper_store.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-lpthread','-lm','-o',str(w/'live')]
with (R/'logs/v161-live-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
r=subprocess.run([str(w/'live'),str(w)],cwd=R/'upstream',env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')},capture_output=True,text=True,timeout=120)
(R/'logs/v161-live-wallhaven.log').write_text(r.stdout+r.stderr);print(r.stdout+r.stderr,end='');r.check_returncode()
(R/'logs/v161-live-wallhaven.json').write_text(json.dumps({'version':'1.6.1','exit':0,'actual_https_worker':True,'bundled_ca':True,'catalog_thumbnail_full_and_resize':True,'provider':'Wallhaven','hardware_tested':False},indent=2)+'\n')
