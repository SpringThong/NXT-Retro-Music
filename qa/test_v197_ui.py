#!/usr/bin/env python3
"""Compile and exercise the actual production UI, with explicit ROM fixtures."""
from pathlib import Path
import json,os,shutil,subprocess,tempfile
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';ROM=os.environ.get('NXT_TEST_ROM','NXT_ROM_STOCK')
if not (q/'includes.txt').is_file():subprocess.run(['python3',str(R/'qa/setup_render_spotify.py')],check=True)
stage=q/'stage';
for folder in ['fonts','icons','themes','emoji3d','locales','ime']:shutil.copytree(R/'upstream/res'/folder,stage/'res'/folder,dirs_exist_ok=True)
art=Image.new('RGB',(500,500),(22,60,68));draw=ImageDraw.Draw(art)
for y in range(500):draw.line((0,y,500,y),fill=(22+y//8,60+y//7,68+y//8))
draw.ellipse((220,50,420,250),fill=(243,210,153));draw.polygon(((0,370),(145,150),(320,390),(420,280),(500,350),(500,500),(0,500)),fill=(16,44,60));art.save(stage/'res/avatar-test.png')
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/{'NXT_ROM_STOCK':'.cache/stock-build/headers/samplerate','NXT_ROM_NEXTUI':'.cache/nextui18-build/headers/samplerate','NXT_ROM_NXREDUX':'.cache/nxredux-build/headers/samplerate'}[ROM],R/'.host-tools/usr/include']]
out=R/'qa/screens-v1.9.7';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='nxt-v197-ui-') as td:
 w=Path(td);data=w/'data';data.mkdir();frames=w/'frames';frames.mkdir();binary=w/'render'
 fake=w/'bin';fake.mkdir();shutil.copy2(R/'qa/fake_translation_curl.py',fake/'curl');(fake/'curl').chmod(0o755);control=w/'control';control.write_text('ok');requests=w/'requests.jsonl'
 sources=['nxt_motion.c','nxt_keyfile.c','nxt_auralis.c','nxt_skin.c','nxt_theme.c','nxt_custom.c','nxt_monitor.c','nxt_performance.c','nxt_eq.c','nxt_presets.c','playlist_m3u.c','utf8.c','nxt_dsp.c','nxt_record.c','spectrum.c','spectrum_ceiling.c','audio/kiss_fft.c','audio/kiss_fftr.c','nxt_appearance.c','nxt_wallpaper_store.c','ui_theme.c','ui_theme_utils.c','nxt_ui.c','nxt_pad.c','nxt_locale.c','nxt_ime.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c','nxt_startup.c','nxt_cpu.c','nxt_write.c','nxt_translate.c','include/mbedtls_lib/sha256.c','include/mbedtls_lib/platform_util.c']
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-Werror=implicit-function-declaration','-Werror=int-conversion','-DUSE_SDL2','-D'+ROM,'-DPLATFORM_DESKTOP','-DNXT_UI_V18','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*incs,'-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-include',str(S/'nxt_i18n.h'),str(R/'qa/v197_render.c'),*[str(S/f) for f in sources],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v197-ui-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 result=subprocess.run([str(binary),str(frames)],cwd=stage,env={**os.environ,'QA_DATA':str(data),'QA_CONTROL':str(control),'QA_REQUESTS':str(requests),'QA_TRANSLATION_VISUAL':'1','PATH':str(fake)+':'+os.environ['PATH'],'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=180)
 (R/'logs/v197-ui-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for p in frames.glob('*.bmp'):
  with Image.open(p) as im:im.save(out/(p.stem+'.png'))
(R/'logs/v197-ui-test.json').write_text(json.dumps({'exit':0,'ASan_UBSan':True,'native_sizes':[640,1024],'legacy_themes':50,'total_themes':51,'fonts':22,'real_translation_worker_local_fixture':True,'physical_device_tested':False},indent=2)+'\n')
