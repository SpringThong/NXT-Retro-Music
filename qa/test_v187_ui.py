#!/usr/bin/env python3
"""Compile and exercise the actual production UI, with explicit ROM fixtures."""
from pathlib import Path
import json,os,shutil,subprocess,tempfile
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';ROM=os.environ.get('NXT_TEST_ROM','NXT_ROM_STOCK')
if not (q/'includes.txt').is_file():subprocess.run(['python3',str(R/'qa/setup_render_spotify.py')],check=True)
stage=q/'stage';
for folder in ['icons','themes','emoji3d','locales','ime']:shutil.copytree(R/'upstream/res'/folder,stage/'res'/folder,dirs_exist_ok=True)
art=Image.new('RGB',(500,500),(22,60,68));draw=ImageDraw.Draw(art)
for y in range(500):draw.line((0,y,500,y),fill=(22+y//8,60+y//7,68+y//8))
draw.ellipse((220,50,420,250),fill=(243,210,153));draw.polygon(((0,370),(145,150),(320,390),(420,280),(500,350),(500,500),(0,500)),fill=(16,44,60));art.save(stage/'res/avatar-test.png')
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/{'NXT_ROM_STOCK':'.cache/stock-build/headers/samplerate','NXT_ROM_NEXTUI':'.cache/nextui18-build/headers/samplerate','NXT_ROM_NXREDUX':'.cache/nxredux-build/headers/samplerate'}[ROM],R/'.host-tools/usr/include']]
out=R/'qa/screens-v1.8.7';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='nxt-v187-ui-') as td:
 w=Path(td);data=w/'data';data.mkdir();frames=w/'frames';frames.mkdir();binary=w/'render'
 sources=['nxt_skin.c','nxt_theme.c','nxt_custom.c','nxt_monitor.c','nxt_performance.c','nxt_eq.c','nxt_presets.c','playlist_m3u.c','utf8.c','nxt_dsp.c','nxt_record.c','spectrum.c','spectrum_ceiling.c','audio/kiss_fft.c','audio/kiss_fftr.c','nxt_appearance.c','nxt_wallpaper_store.c','ui_theme.c','ui_theme_utils.c','nxt_ui.c','nxt_pad.c','nxt_locale.c','nxt_ime.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c','nxt_startup.c','nxt_cpu.c','nxt_write.c']
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-Werror=implicit-function-declaration','-Werror=int-conversion','-DUSE_SDL2','-D'+ROM,'-DPLATFORM_DESKTOP','-DNXT_UI_V18','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v187_render.c'),*[str(S/f) for f in sources],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v187-ui-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 result=subprocess.run([str(binary),str(frames)],cwd=stage,env={**os.environ,'QA_DATA':str(data),'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=180)
 (R/'logs/v187-ui-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for p in frames.glob('*.bmp'):
  with Image.open(p) as im:im.save(out/(p.stem+'.png'))
visible_colors={}
for mode,palette in [(2,[0xff4242,0xff952b,0xffe54a,0x5ee664,0x47e7ea,0x4c86ff,0xc65dff]),(4,[0x83ddb0,0x50e6ce,0xffd36a,0xffa4c9,0xfa77e0,0x61e76d,0xff9758])]:
 pixels=dict((rgb,n) for n,rgb in Image.open(out/f'spectrum-20-{mode}-1024.png').convert('RGB').getcolors(2000000))
 counts={f'{v:06x}':pixels.get(((v>>16)&255,(v>>8)&255,v&255),0) for v in palette};assert all(counts.values()),counts;visible_colors[str(mode)]=counts
report={'exit':0,'seven_and_mixed_colors_native_pixels':visible_colors,'rom':ROM,'system_languages':14,'native_layouts':50,'sizes':[640,1024],'icon_choices':300,'emoji_choices':180,'spectrum_combinations':105,'LED_effects':6,'ASan_UBSan':True,'theme_RGB_original_background_cover_preserved':True,'original_player_restored':True,'performance_controller_modes_apply_cancel_save_failure':True,'controller_flow':True,'static_active_lyric_pixels_all_50_layouts_both_sizes':True,'cached_render_host_benchmark':True,'select_toggle_preserves_preset':True,'startup_restores_paused_without_questionnaire':True,'profile_local_startup_card':True,'physical_device_tested':False,'live_account_playback_tested':False}
(R/'logs'/({'NXT_ROM_NEXTUI':'v187-ui-nextui-test.json','NXT_ROM_NXREDUX':'v187-ui-nxredux-test.json','NXT_ROM_STOCK':'v187-ui-test.json'}[ROM])).write_text(json.dumps(report,indent=2)+'\n')
