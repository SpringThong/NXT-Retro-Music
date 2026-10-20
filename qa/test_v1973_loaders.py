#!/usr/bin/env python3
from pathlib import Path
import json,os,subprocess,hashlib,struct
R=Path(__file__).resolve().parents[1];SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';qemu=R/'.host-tools/usr/bin/qemu-aarch64'
targets=[('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NX Redux','nxredux')]
for rom,label,d in targets:
 if rom not in os.environ.get('QA_ROMS','stock,nextui').split(','):continue
 b=R/d/'bin/tg5040';elf=b/'musicplayer.elf';elf.chmod(0o755);h=elf.read_bytes()[:64];assert h[:6]==b'\x7fELF\2\1' and struct.unpack_from('<H',h,18)[0]==183;checks=[]
 for arg in ['--build-info','--help']:
  env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[b,R/'upstream/bin/tg5040',SDK/'lib',SDK/'usr/lib']))};p=subprocess.run([str(qemu),'-L',str(SDK),str(elf),arg],env=env,capture_output=True,text=True,timeout=25);assert p.returncode==0,(rom,p.stdout,p.stderr);assert '1.9.7.3' in p.stdout;if_label=label in p.stdout if arg=='--build-info' else '--build-info' in p.stdout;assert if_label;checks.append({'argument':arg,'exit':0,'output':p.stdout[:400]})
 (R/f'logs/v1973-loader-{rom}.json').write_text(json.dumps({'version':'1.9.7.3','rom':rom,'exit':0,'binary_sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),'real_ARM64_release':True,'QEMU_with_pinned_SDK':True,'checks':checks,'physical_ROM_boot_tested':False},indent=2)+'\n');print('PASS',rom,'real ARM64 loader 1.9.7')
