"""Compile the exact production power functions with hardware/clock fixtures."""
from pathlib import Path
import subprocess,tempfile,os,json
R=Path(__file__).resolve().parents[1];q=R/'.cache/render-sp'
common=R/'upstream/NextUI/workspace/all/common'
api=(common/'api.c').read_text();start=api.index('void PWR_setDisplayOff(int off)');end=api.index('// TODO: this isn\'t whether',start)
production=api[start:end]
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [R/'upstream/src',R/'upstream/src/include',common,R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
results={}
for rom in ['STOCK','NEXTUI','NXREDUX']:
 with tempfile.TemporaryDirectory() as td:
  p=Path(td);c=p/'power.c';c.write_text((R/'qa/v1973_power_fixture.h').read_text()+production+(R/'qa/v1973_power_test.inc').read_text());exe=p/'test'
  cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-DPLATFORM_DESKTOP','-fsanitize=address,undefined',*incs,str(c),'-l:libSDL2-2.0.so.0','-lm','-o',str(exe)]
  with (R/f'logs/v1973-screenoff-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  out=subprocess.run([str(exe)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});print(rom,out.stdout+out.stderr);out.check_returncode();results[rom]={'exit':0,'production_power_functions':True,'all_controls_stay_dark':True,'power_guard_and_timeout':True,'ASan_UBSan':True}
(R/'logs/v1973-screenoff.json').write_text(json.dumps(results,indent=2)+'\n')

# Exercise production app key routing and the exact SELECT lock implementation.
ui=(R/'upstream/src/nxt_ui.c').read_text();a=ui.index('bool Nxt_lockInput(void)');b=ui.index('static void fonts',a)
for rom in ['STOCK','NEXTUI','NXREDUX']:
 with tempfile.TemporaryDirectory() as td:
  p=Path(td);c=p/'keys.c';c.write_text((R/'qa/v1973_dark_keys.c').read_text()+ui[a:b]+(R/'qa/v1973_dark_keys_test.inc').read_text());exe=p/'keys'
  cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-DPLATFORM_DESKTOP','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(c),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-lm','-o',str(exe)]
  with (R/f'logs/v1973-dark-keys-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  out=subprocess.run([str(exe)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});print(rom,out.stdout+out.stderr);out.check_returncode()
  results[rom]['production_dark_control_routing_and_lock']=True
(R/'logs/v1973-screenoff.json').write_text(json.dumps(results,indent=2)+'\n')
