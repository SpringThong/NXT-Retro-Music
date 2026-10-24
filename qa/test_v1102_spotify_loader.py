"""Load the actual ARM64 Spotify helper, exercise unauthenticated IPC, then quit."""
from pathlib import Path
import hashlib,json,os,shutil,struct,subprocess
R=Path(__file__).resolve().parents[1]
binary=R/'upstream/bin/tg5040/nxt-spotify-backend'
data=binary.read_bytes()
assert data[:6]==b'\x7fELF\2\1' and struct.unpack_from('<H',data,18)[0]==183
sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[R/'upstream/bin/tg5040',sdk/'lib',sdk/'usr/lib']))}
for key in ['NXT_SPOTIFY_CREDENTIALS','NXT_SPOTIFY_PLAYBACK_CREDS','NXT_SPOTIFY_DIAG']:env.pop(key,None)
p=subprocess.run([shutil.which('qemu-aarch64'),'-L',str(sdk),str(binary)],input='{"cmd":"search","query":"loader-test"}\n{"cmd":"quit"}\n',env=env,capture_output=True,text=True,timeout=25)
events=[json.loads(line) for line in p.stdout.splitlines()]
report={'version':'1.10.2','exit':p.returncode,'backend_sha256':hashlib.sha256(data).hexdigest(),'stdout_events':events,'stderr':p.stderr,'real_ARM64_release':True,'QEMU_with_pinned_SDK':True,'live_account_tested':False,'physical_device_tested':False}
(R/'logs/v1102-spotify-loader.json').write_text(json.dumps(report,indent=2)+'\n')
assert p.returncode==0,report
assert events[0]['event']=='ready' and events[0]['version']=='1.10.2' and events[0]['build']=='NXT-v1.10.2-NativeCatalog',report
assert events[1:]==[{'event':'error','reason':'not_logged_in'}],report
assert 'Backend build: NXT-v1.10.2-NativeCatalog' in p.stderr,report
print('PASS actual ARM64 Spotify helper: loader, version, unauthenticated IPC and clean quit')
