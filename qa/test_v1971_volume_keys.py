from pathlib import Path
import subprocess,tempfile,os,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
results={}
with tempfile.TemporaryDirectory(prefix='nxt186-volume-keys-') as td:
 for rom in ['STOCK','NEXTUI','NXREDUX']:
  b=Path(td)/rom;cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-DPLATFORM_DESKTOP','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(R/'qa/v194_volume_keys.c'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
  with (R/f'logs/v1971-volume-keys-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  p=subprocess.run([str(b)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=15);(R/f'logs/v1971-volume-keys-{rom.lower()}-test.log').write_text(p.stdout+p.stderr);print(rom,p.stdout+p.stderr,end='');p.check_returncode();results[rom]={'exit':0,'real_global_custom_HID_volume_inputs':True,'ASan_UBSan':True}
(R/'logs/v1971-volume-keys-test.json').write_text(json.dumps(results,indent=2)+'\n')
