#!/usr/bin/env python3
"""Freeze checked HomeDAC release and corresponding source; historical QA retained."""
from pathlib import Path
import hashlib,json,shutil,struct,zipfile
R=Path(__file__).resolve().parents[1];D=R.parent/'deliverables';D.mkdir(exist_ok=True)
install=D/'NXT_Retro_Music_v1.0_NextUI_BottomDAC_Fix1.zip';shutil.copyfile(R/'dist/NXT.Retro.Music.pak.zip',install)
elf=(R/'upstream/bin/tg5040/musicplayer.elf').read_bytes();assert elf[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',elf,18)[0]==183
for hook in [b'--test-control',b'QA USB DAC',b'NXT_QA_OFFLINE']:assert hook not in elf
pak='NXT Retro Music.pak/'
with zipfile.ZipFile(install) as z:
 assert z.testzip() is None
 assert z.read(pak+'bin/tg5040/musicplayer.elf')==elf
 assert z.getinfo(pak+'launch.sh').external_attr>>16&0o111
 assert z.getinfo(pak+'bin/tg5040/musicplayer.elf').external_attr>>16&0o111
 assert 'HUONG_DAN_NXT_Retro_Music_NextUI_HomeDAC.md' in z.namelist()
 for p in (R/'upstream/res').rglob('*'):
  if p.is_file():assert z.read(pak+'res/'+p.relative_to(R/'upstream/res').as_posix())==p.read_bytes()
with zipfile.ZipFile(R.parents[1]/'download/NXT_Retro_Music_v1.0_FooterFix_Source.zip') as baseline:
 prefix='NXT_Retro_Music_v1.0_FooterFix_Source/'
 for p in (R/'upstream/res').rglob('*'):
  if p.is_file():assert baseline.read(prefix+p.relative_to(R).as_posix())==p.read_bytes()
 for rel in ['upstream/src/nxt_player_core.c','upstream/src/nxt_decoder.c']:
  assert baseline.read(prefix+rel)==(R/rel).read_bytes()
print('PASS installer: AArch64 release, executable modes, CRC, exact payload; fonts/icons/album decoder/audio routing unchanged from approved NextUI FooterFix.',flush=True)
manifest=json.loads((R/'overlay-manifest.json').read_text())
for item in manifest['files']:
 for base in ['overlay','upstream']:assert hashlib.sha256((R/base/item['path']).read_bytes()).hexdigest()==item['sha256']
excluded={'dist','backups','.cache','.nxt-sdk','.host-tools','.build-headers','.git','__pycache__'}
files=[]
for p in sorted(R.rglob('*')):
 rel=p.relative_to(R)
 if any(x in excluded for x in rel.parts) or not p.is_file() or p.name=='FILES_SHA256.json':continue
 if rel.parts[0]=='upstream' and ('build' in rel.parts or p.suffix in ('.o','.d')):continue
 if rel.parts[:3]==('upstream','bin','desktop'):continue
 if rel.parts[0]=='qa' and p.name.endswith('.arm64'):continue
 files.append(p)
m={p.relative_to(R).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files};(R/'FILES_SHA256.json').write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n');files.append(R/'FILES_SHA256.json')
source=D/'NXT_Retro_Music_v1.0_FooterFix_Source.zip';prefix='NXT_Retro_Music_v1.0_FooterFix_Source/'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in files:z.write(p,prefix+p.relative_to(R).as_posix())
with zipfile.ZipFile(source) as z:
 assert z.testzip() is None
 for rel,digest in m.items():assert hashlib.sha256(z.read(prefix+rel)).hexdigest()==digest,rel
for p in [install,source]:
 digest=hashlib.sha256(p.read_bytes()).hexdigest();p.with_suffix('.zip.sha256').write_text(digest+'  '+p.name+'\n');print(p.name,p.stat().st_size,digest,flush=True)
shutil.copyfile(R/'HUONG_DAN_NXT_Retro_Music_NextUI_HomeDAC.md',D/'HUONG_DAN_NXT_Retro_Music_NextUI_HomeDAC.md')
print('PASS corresponding source ZIP CRC and all',len(m),'file hashes.',flush=True)
