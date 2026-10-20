from pathlib import Path
import zipfile,hashlib,json,os,subprocess
R=Path(__file__).resolve().parents[1]
original=Path('/workspace/scratch/d7cab07fb248/recovery-source/ứng dụng NXT Retro Music cho Trimui Brick Pro/NXT_Retro_Music_v1.8.9_Source.zip')
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
assert sha(original)=='30311ea905005623f793b13c517728e12dd83920db9ef1cc25883ba19dd90332'
with zipfile.ZipFile(original) as z:
 names=z.namelist();prefix=names[0].split('/')[0]+'/';verified={};historical={}
 for rel in ['stock/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/nxt-spotify-backend','upstream/bin/tg5040/nxt-soundcloud-backend','upstream/src/nxt_dsp.c','upstream/src/nxt_bitperfect.c','upstream/src/nxt_audio_mode.c','upstream/src/nxt_startup.c','upstream/src/nxt_theme_v18.inc','upstream/src/nxt_ota.c','upstream/src/nxt_ota_package.c']:
  assert hashlib.sha256(z.read(prefix+rel)).hexdigest()==sha(R/rel),rel;verified[rel]=sha(R/rel)
  if rel.endswith('musicplayer.elf'):historical[rel]=verified[rel]
 assets=[n for n in names if n.startswith(prefix+'upstream/res/') and not n.endswith('/')]
 for n in assets:
  p=R/n[len(prefix):];assert p.exists() and sha(p)==hashlib.sha256(z.read(n)).hexdigest(),n
(R/'logs/v1891-baseline-preserved.json').write_text(json.dumps({'exit':0,'baseline_version':'1.8.9','baseline_sha256':sha(original),'unchanged_core':verified,'historical_frontends_preserved':historical,'unchanged_UI_assets':len(assets),'audio_hardware_tested':False},indent=2)+'\n')
sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';qemu=R/'.host-tools/usr/bin/qemu-aarch64';env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[R/'nxredux/bin/tg5040',R/'upstream/bin/tg5040',sdk/'lib',sdk/'usr/lib']))};out=[]
for arg in ['--build-info','--help']:
 p=subprocess.run([str(qemu),'-L',str(sdk),str(R/'nxredux/bin/tg5040/musicplayer.elf'),arg],env=env,capture_output=True,text=True,timeout=20);out.append(p.stdout+p.stderr);assert p.returncode==0,p.stderr
assert '1.8.9.1' in out[0] and 'NX Redux' in out[0]
(R/'logs/v1891-loader-test.log').write_text('\n'.join(out));(R/'logs/v1891-loader-test.json').write_text(json.dumps({'exit':0,'real_ARM64_release':True,'build_info_and_help':True,'version':'1.8.9.1','hardware_tested':False},indent=2)+'\n')
print('PASS baseline: unchanged core files,',len(assets),'UI assets, historical Stock/NextUI binaries; real ARM64 loader.');print(out[0])
