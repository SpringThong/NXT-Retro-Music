from pathlib import Path
import hashlib,json,os,shutil,subprocess
R=Path(__file__).resolve().parents[1]
sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
rom=os.environ['QA_ROMS'];folder={'stock':'stock','nextui':'upstream','nxredux':'nxredux'}[rom]
label={'stock':'Stock','nextui':'NextUI','nxredux':'NX Redux'}[rom]
binary=R/folder/'bin/tg5040/musicplayer.elf'
env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[R/folder/'bin/tg5040',R/'upstream/bin/tg5040',sdk/'lib',sdk/'usr/lib']))}
checks=[]
for arg in ['--build-info','--help']:
    p=subprocess.run([shutil.which('qemu-aarch64'),'-L',str(sdk),str(binary),arg],env=env,capture_output=True,text=True,timeout=25)
    record={'argument':arg,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr}
    checks.append(record)
    (R/f'logs/v1102-loader-{rom}.json').write_text(json.dumps({'version':'1.10.2','rom':rom,'checks':checks},indent=2)+'\n')
    assert p.returncode==0,(rom,arg,p.returncode,p.stdout,p.stderr)
    assert '1.10.2' in p.stdout,(rom,arg,'missing version output',p.stdout,p.stderr)
    if arg=='--build-info':assert label in p.stdout,(rom,p.stdout)
report={'version':'1.10.2','rom':rom,'exit':0,'binary_sha256':hashlib.file_digest(binary.open('rb'),'sha256').hexdigest(),'real_ARM64_release':True,'QEMU_with_pinned_SDK':True,'checks':checks,'physical_ROM_boot_tested':False}
(R/f'logs/v1102-loader-{rom}.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS',rom,'ARM64 --build-info / --help exit, version and ROM label')
