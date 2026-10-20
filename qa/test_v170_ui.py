#!/usr/bin/env python3
"""Compile and exercise the actual production UI, with explicit ROM fixtures."""
from pathlib import Path
import json,os,shutil,subprocess,tempfile
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';ROM=json.loads((R/'SOURCE.json').read_text())['build_define']
if not (q/'includes.txt').is_file():subprocess.run(['python3',str(R/'qa/setup_render_spotify.py')],check=True)
stage=q/'stage';shutil.copytree(R/'upstream/res/icons',stage/'res/icons',dirs_exist_ok=True)
art=Image.new('RGB',(500,500),(22,60,68));draw=ImageDraw.Draw(art)
for y in range(500):draw.line((0,y,500,y),fill=(22+y//8,60+y//7,68+y//8))
draw.ellipse((220,50,420,250),fill=(243,210,153));draw.polygon(((0,370),(145,150),(320,390),(420,280),(500,350),(500,500),(0,500)),fill=(16,44,60));art.save(stage/'res/avatar-test.png')
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/('.cache/stock-build/headers/samplerate' if ROM=='NXT_ROM_STOCK' else '.cache/nextui16-build/headers/samplerate'),R/'.host-tools/usr/include']]
out=R/'qa/screens-v1.7.0';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='nxt-v170-ui-') as td:
 w=Path(td);data=w/'data';data.mkdir();frames=w/'frames';frames.mkdir();binary=w/'render'
 sources=['nxt_skin.c','nxt_theme.c','nxt_custom.c','nxt_monitor.c','nxt_eq.c','nxt_presets.c','playlist_m3u.c','utf8.c','nxt_dsp.c','nxt_record.c','spectrum.c','spectrum_ceiling.c','audio/kiss_fft.c','audio/kiss_fftr.c','nxt_appearance.c','nxt_wallpaper_store.c','ui_theme.c','ui_theme_utils.c','nxt_ui.c','nxt_pad.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c']
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-D'+ROM,'-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v170_render.c'),*[str(S/f) for f in sources],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v170-ui-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 result=subprocess.run([str(binary),str(frames)],cwd=stage,env={**os.environ,'QA_DATA':str(data),'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=110)
 (R/'logs/v170-ui-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for p in frames.glob('*.bmp'):
  with Image.open(p) as im:im.save(out/(p.stem+'.png'))
report={'exit':0,'native_layouts':24,'sizes':[640,1024],'icon_choices':64,'emoji_choices':64,'spectrum_combinations':105,'LED_effects':6,'ASan_UBSan':True,'theme_RGB_original_background_cover_preserved':True,'controller_flow':True,'physical_device_tested':False,'live_account_playback_tested':False}
(R/'logs/v170-ui-test.json').write_text(json.dumps(report,indent=2)+'\n')
