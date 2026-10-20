from pathlib import Path
import subprocess,json,os,tempfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',SDK/'usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt185-raw-') as td:
 b=Path(td)/'worker';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,str(R/'qa/v1881_bit_worker.c'),str(S/'nxt_bitperfect.c'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v1881-bit-worker-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=45);(R/'logs/v1881-bit-worker-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v1881-bit-worker-test.json').write_text(json.dumps({'exit':0,'source_sha256':hashlib.sha256((S/'nxt_bitperfect.c').read_bytes()).hexdigest(),'production_worker':True,'four_integer_formats_byte_equal':True,'ASan_UBSan':True,'ALSA_and_decoder_explicit_simulators':True,'hardware_tested':False},indent=2)+'\n')
