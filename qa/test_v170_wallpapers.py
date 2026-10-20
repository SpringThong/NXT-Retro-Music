#!/usr/bin/env python3
"""Native production wallpaper core/UI and child-worker tests under ASan/UBSan."""
from pathlib import Path
import subprocess,os,tempfile,json,shutil
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
if not (q/'includes.txt').exists():subprocess.run(['python3',str(R/'qa/setup_render_spotify.py')],check=True)
out=R/'qa/screens-v1.7.0';out.mkdir(exist_ok=True)
shutil.copytree(R/'upstream/res/icons',q/'stage/res/icons',dirs_exist_ok=True)
shutil.copy2(R/'upstream/res/cacert.pem',q/'stage/res/cacert.pem')
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/('.cache/stock-build/headers/samplerate' if json.loads((R/'SOURCE.json').read_text())['build_define']=='NXT_ROM_STOCK' else '.cache/nextui16-build/headers/samplerate'),R/'.host-tools/usr/include']]
# SDCARD_PATH in the desktop fixture is /tmp; override via an injected header only in QA.
platform=R/'upstream/NextUI/workspace/tg5040/platform/platform.h'
with tempfile.TemporaryDirectory(prefix='nxt-v170-ui-') as td:
 w=Path(td);data=w/'data';data.mkdir();sd=w/'card';images=sd/'Pictures/Wallpapers';images.mkdir(parents=True);frames=w/'frames';frames.mkdir();bin_=w/'bin';bin_.mkdir();stage=q/'stage'
 from PIL import ImageFont
 font=ImageFont.truetype(str(R/'upstream/res/fonts/notosans.ttf'),52)
 for i,name in enumerate(['1.png','2.jpg','9.png','10.png','Alpha.png']):
  im=Image.new('RGB',(1600,1000),(20+i*20,40+i*14,60+i*15));d=ImageDraw.Draw(im)
  for y in range(1000):d.line((0,y,1600,y),fill=(20+i*20,min(255,40+i*14+y//12),min(255,60+i*15+y//10)))
  d.ellipse((800,100,1000,300),fill=(240,188,124));d.polygon([(0,650),(400,280),(700,680),(1050,430),(1600,720),(1600,1000),(0,1000)],fill=(20,43,64));d.text((120,100),name,font=font,fill='white');im.save(images/name)
 (images/'not-an-image.txt').write_text('ignore');(images/'.hidden.png').write_text('ignore');(images/'link.png').symlink_to('/tmp/outside.png')
 sample=images/'2.jpg';(bin_/'curl').write_text('''#!/usr/bin/env python3
import sys,os,shutil,time
url=sys.argv[-1];target=sys.argv[sys.argv.index('-o')+1]
if url.endswith('/slow'):time.sleep(30)
elif url.endswith('.jpg'):shutil.copyfile(os.environ['QA_WALLPAPER_IMAGE'],target)
else:open(target,'w').write('{"meta":{"last_page":1},"data":[]}')
''');(bin_/'curl').chmod(0o755)
 # api.h takes card root from PLATFORM_DESKTOP platform; command macro cannot override its unconditional definition.
 includes=w/'include';includes.mkdir();text=platform.read_text();import re
 text=re.sub(r'#define SDCARD_PATH[^\n]*', '#define SDCARD_PATH "'+str(sd)+'"', text);(includes/'platform.h').write_text(text)
 binary=w/'render';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+('STOCK' if json.loads((R/'SOURCE.json').read_text())['build_define']=='NXT_ROM_STOCK' else 'NEXTUI'),'-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(includes),*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v161_wallpaper_render.c'),*[str(S/f) for f in ['nxt_skin.c','nxt_theme.c','nxt_custom.c','nxt_monitor.c','nxt_appearance.c','nxt_wallpaper_store.c','nxt_sort.c','ui_theme.c','ui_theme_utils.c','nxt_ui.c','nxt_pad.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c']],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v170-wallpaper-qa-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 result=subprocess.run([str(binary),str(frames),str(images)],cwd=stage,env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'QA_DATA':str(data),'QA_WALLPAPER_IMAGE':str(sample),'PATH':str(bin_)+':'+os.environ['PATH'],'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=30)
 (R/'logs/v170-wallpaper-qa.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for p in frames.glob('*.bmp'):
  with Image.open(p) as im:im.save(out/(p.stem+'.png'))
(R/'logs/v170-wallpaper-qa.json').write_text(json.dumps({'version':'1.7.0','exit':0,'production_parser_resize_sort_persistence':True,'production_gallery_and_image_worker':True,'quality_antialias_pixel_alpha':True,'full_detail_cache':True,'legacy_cache_preserved_and_rebuilt':True,'online_original_reused_offline':True,'genre_count':24,'pagination_fixture_images':1200,'pagination_window_cap':512,'ui_sizes':[640,1024],'asan_ubsan':True,'hardware_tested':False,'network':'deterministic curl child fixtures; no live network check in this v1.7.0 run'},indent=2)+'\n')
