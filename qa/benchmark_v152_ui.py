#!/usr/bin/env python3
"""Compare production native paint CPU with the immutable v1.5.1 skin, no audio/hardware claim."""
from pathlib import Path
import subprocess,tempfile,os,json,statistics
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/'.cache/stock-build/headers/samplerate']]
results={}
with tempfile.TemporaryDirectory(prefix='nxt-v152-bench-') as td:
 w=Path(td)
 for variant,skin in [('v151',R/'qa/baseline-v151/nxt_skin.c'),('current',S/'nxt_skin.c')]:
  binary=w/variant
  sources=['nxt_appearance.c','ui_theme.c','ui_theme_utils.c','nxt_ui.c','nxt_pad.c','nxt_text.c','nxt_led.c','nxt_controls.c','nxt_screens.c','include/parson/parson.c']
  cmd=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections',*(["-I"+str(R/"qa/baseline-v151")] if variant=="v151" else []),*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v152_render_benchmark.c'),str(skin),*[str((R/"qa/baseline-v151"/f) if variant=="v151" and f in ("nxt_ui.c","nxt_appearance.c") else S/f) for f in sources],str(q/'ttf.o'),str(q/'utf8proc.o'),'-Wl,--gc-sections','-Wl,--wrap=NxtText_render','-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsamplerate.so.0','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
  with (R/'logs'/('v152-benchmark-'+variant+'-build.log')).open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
  runs=[]
  for n in range(3):
   data=w/(variant+str(n));data.mkdir();r=subprocess.run([str(binary)],cwd=q/'stage',env={**os.environ,'QA_DATA':str(data)},capture_output=True,text=True,check=True,timeout=30)
   rows={}
   for line in r.stdout.splitlines():
    name,ms,renders=line.split();rows[name]={'cpu_ms_per_paint':float(ms),'text_renders_150_paints':int(renders)}
   runs.append(rows)
  results[variant]={name:{'cpu_ms_per_paint':statistics.median(r[name]['cpu_ms_per_paint'] for r in runs),'text_renders_150_paints':runs[0][name]['text_renders_150_paints']} for name in runs[0]}
report={'native_render':{'exit':0,'dimensions':[1024,768],'results':results,'physical_hardware_tested':False,'scope':'paint CPU only; native x86 renderer, no audio, temperature, total-device CPU or battery claim'}}
(R/'logs/v152-render-benchmark.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
