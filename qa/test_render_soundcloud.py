#!/usr/bin/env python3
"""Software-surface render of production NXT/Spotify UI; fixture audio and DAC."""
from pathlib import Path
import subprocess,os
from PIL import Image
r=Path(__file__).resolve().parents[1];q=r/'.cache/render-sp';s=r/'upstream/src'
if not (q/'includes.txt').exists():subprocess.run(['python3',str(r/'qa/setup_render_spotify.py')],check=True)
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(r/'.cache/stock-build/headers/samplerate')];incs+=['-I'+str(s),'-I'+str(s/'include'),'-I'+str(r/'upstream/NextUI/workspace/all/common'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/libmsettings'),'-I'+str(r/'.nxt-sdk/tg5040/local/include')]
cmd=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections',*incs,'-include',str(s/'nxt_i18n.h'),str(r/'qa/soundcloud_render.c'),str(s/'nxt_skin.c'),str(s/'nxt_drive.c'),str(s/'nxt_text.c'),str(s/'nxt_ui.c'),str(s/'nxt_pad.c'),str(s/'lyrics.c'),str(s/'lyric_window.c'),str(s/'spectrum.c'),str(s/'spectrum_ceiling.c'),str(s/'audio/kiss_fft.c'),str(s/'audio/kiss_fftr.c'),str(s/'include/parson/parson.c'),str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-lpthread','-lm','-o',str(q/'render-soundcloud')]
with (q/'soundcloud-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
out=r/'qa/screens-v1.2-soundcloud';out.mkdir(exist_ok=True);data=q/'data';data.mkdir(exist_ok=True)
subprocess.run([str(q/'render-soundcloud'),str(out),str(q/'fixture.lrc'),str(q/'stage/cover.png')],cwd=q/'stage',env={**os.environ,'QA_DATA':str(data)},check=True)
for f in out.glob('*.bmp'):Image.open(f).save(f.with_suffix('.png'));f.unlink()
