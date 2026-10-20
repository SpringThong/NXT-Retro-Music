#!/usr/bin/env python3
from pathlib import Path
import subprocess,os,tempfile,json
from PIL import Image
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';out=R/'qa/screens-stock-v1.3';out.mkdir(exist_ok=True)
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/'.cache/stock-build/headers/samplerate']]
with tempfile.TemporaryDirectory(prefix='nxt-v13-ui-') as td:
 w=Path(td);binary=w/'render';frames=w/'frames';frames.mkdir();data=w/'data';data.mkdir()
 cmd=['gcc','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v13_ui_render.c'),*[str(S/f) for f in ['nxt_skin.c','nxt_ui.c','nxt_pad.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c']],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v13-ui-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 result=subprocess.run([str(binary),str(frames)],cwd=q/'stage',env={**os.environ,'QA_DATA':str(data),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=45)
 (R/'logs/v13-ui-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for p in frames.glob('*.bmp'):
  with Image.open(p) as im:im.save(out/(p.stem+'.png'))
(R/'logs/v13-ui-test.json').write_text(json.dumps({'ui_and_controls':{'exit':0,'sizes':[640,1024],'action_button_combinations':21*16,'actual_text_pages':True,'actual_rgb_palette':True,'defaults_and_preview_restore':True,'asan_ubsan':True,'physical_hardware_tested':False}},indent=2)+'\n')
