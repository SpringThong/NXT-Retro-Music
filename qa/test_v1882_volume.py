from pathlib import Path
import subprocess,os,json,tempfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.host-tools/usr/include',SDK/'usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
results={}
with tempfile.TemporaryDirectory(prefix='nxt186-volume-') as td:
 for rom in ['STOCK','NEXTUI','NXREDUX']:
  b=Path(td)/rom;cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-fsanitize=address,undefined','-Werror=implicit-function-declaration','-Werror=int-conversion',*incs,str(R/'qa/v1882_volume.c'),str(S/'nxt_volume.c'),'-o',str(b)]
  with (R/f'logs/v1882-volume-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  p=subprocess.run([str(b)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=20);(R/f'logs/v1882-volume-{rom.lower()}-test.log').write_text(p.stdout+p.stderr);print(rom,p.stdout+p.stderr,end='');p.check_returncode();results[rom]={'exit':0,'actual_volume_controller':True,'ASan_UBSan':True}
results['source_sha256']=hashlib.sha256((S/'nxt_volume.c').read_bytes()).hexdigest();results['physical_DAC_tested']=False
(R/'logs/v1882-volume-test.json').write_text(json.dumps(results,indent=2)+'\n')
