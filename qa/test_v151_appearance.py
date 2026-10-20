#!/usr/bin/env python3
"""Validate real image-header bounds and profile wallpaper migration."""
from pathlib import Path
import json, os, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'upstream/src';render=ROOT/'.cache/render-sp'
includes=(render/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [SRC,SRC/'include',ROOT/'upstream/NextUI/workspace/all/common',ROOT/'upstream/NextUI/workspace/tg5040/platform',ROOT/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-appearance-') as temp:
    folder=Path(temp);source=folder/'headers.c';binary=folder/'headers'
    png=b'\x89PNG\r\n\x1a\n'+b'\0\0\0\rIHDR'
    for name,w,h in [('ok',1024,768),('large',8192,8192),('negative',0x80000000,500),('zero',0,1),('wide',8193,1)]:
        (folder/(name+'.png')).write_bytes(png+w.to_bytes(4,'big')+h.to_bytes(4,'big'))
    (folder/'short.png').write_bytes(png[:12]);(folder/'ok.jpg').write_bytes(b'\xff\xd8\xff\xc0\x00\x11\x08\x03\x00\x04\x00')
    source.write_text('''#include "nxt_appearance.c"
#include <assert.h>
int main(int argc,char **argv){char p[1500];const char *files[]={"ok.png","ok.jpg","large.png","negative.png","zero.png","wide.png","short.png"};for(int i=0;i<7;i++){snprintf(p,sizeof(p),"%s/%s",argv[1],files[i]);assert(image_size(p)==(i<2));}return 0;}
''')
    flags=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*includes]
    env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
    with (ROOT/'logs/v151-appearance-build.log').open('w') as log:
        subprocess.run([*flags,str(source),'-Wl,--gc-sections','-lm','-o',str(binary)],stdout=log,stderr=log,check=True)
        subprocess.run([str(binary),str(folder)],capture_output=True,text=True,check=True,env=env)
        source.write_text('''#include "nxt_config.c"
#include <assert.h>
int main(int argc,char **argv){nxt.font_size=100;nxt.lyric_size=36;nxt.panel_opacity=35;nxt.wallpaper_dim=20;Nxt_loadPreferences(argv[1]);assert(nxt.wallpaper_dim==20&&nxt.panel_opacity==35&&nxt.wallpaper_mode==4);Nxt_loadPreferences(argv[2]);assert(nxt.wallpaper_dim==5&&nxt.panel_opacity==15&&nxt.wallpaper_mode==21);return 0;}
''')
        old=folder/'old.json';old.write_text(json.dumps({'wallpaper_mode':4,'wallpaper_dim':82}))
        new=folder/'new.json';new.write_text(json.dumps({'wallpaper_visibility_fix1':True,'wallpaper_mode':21,'wallpaper_dim':5,'panel_opacity':15}))
        subprocess.run([*flags,str(source),str(SRC/'nxt_pad.c'),str(SRC/'include/parson/parson.c'),'-Wl,--gc-sections','-lm','-o',str(binary)],stdout=log,stderr=log,check=True)
        subprocess.run([str(binary),str(old),str(new)],capture_output=True,text=True,check=True,env=env)
report={'wallpaper_bounds_and_migration':{'exit':0,'actual_production_header_decoder':True,'signed_shift_regression':True,'png_jpeg_and_truncation_bounds':True,'actual_profile_preferences':True,'v15_overlay_migrates_once':True,'later_custom_opacity_preserved':True,'wallpaper_21_roundtrip':True,'asan_ubsan':True}}
(ROOT/'logs/v151-appearance-test.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS actual JPG/PNG header limits, high-bit PNG width without UB, old v1.5 overlay migration and later profile opacity preservation.')
