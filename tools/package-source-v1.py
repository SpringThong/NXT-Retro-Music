#!/usr/bin/env python3
"""Freeze the corresponding source, relink objects, QA and byte manifest."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,struct,zipfile
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT.parent/'deliverables';OUT.mkdir(exist_ok=True)
parser=argparse.ArgumentParser();parser.add_argument('--variant',choices=['','FooterFix'],default='FooterFix');args=parser.parse_args()
variant_suffix='_'+args.variant if args.variant else ''
install=OUT/f'NXT_Retro_Music_v1.0{variant_suffix}_NextUI.zip';shutil.copyfile(ROOT/'dist/NXT.Retro.Music.pak.zip',install)
elf=ROOT/'upstream/bin/tg5040/musicplayer.elf';b=elf.read_bytes()
assert b[:4]==b'\x7fELF' and b[4:6]==b'\x02\x01' and struct.unpack_from('<H',b,18)[0]==183
for forbidden in (b'--test-control',b'NXT_QA_OFFLINE',b'QA USB DAC'):
 assert forbidden not in b,forbidden
pak='NXT Retro Music.pak/'
with zipfile.ZipFile(install) as z:
 assert z.testzip() is None
 assert z.read(pak+'bin/tg5040/musicplayer.elf')==b
 assert json.loads(z.read(pak+'pak.json'))['version']=='v1.0'
 assert z.read(pak+'state/app_version.txt').strip()==b'1.0'
 assert f'HUONG_DAN_NXT_Retro_Music_v1.0{variant_suffix}.md' in z.namelist()
 assert not any('/.kind' in name for name in z.namelist())
 assert z.getinfo(pak+'launch.sh').external_attr>>16&0o111
 assert z.getinfo(pak+'bin/tg5040/musicplayer.elf').external_attr>>16&0o111
 for p in (ROOT/'upstream/res/fonts').rglob('*'):
  if p.is_file():assert z.read(pak+'res/fonts/'+p.relative_to(ROOT/'upstream/res/fonts').as_posix())==p.read_bytes()
with zipfile.ZipFile(ROOT.parent/'restored/NXT_Retro_Music_0.3.0_Source.zip') as old:
 baseline_candidates=[n for n in old.namelist() if n.endswith('overlay/src/module_player.c')]
 assert baseline_candidates
 for p in (ROOT/'overlay/res/fonts').glob('*.ttf'):
  suffix='overlay/res/fonts/'+p.name
  matches=[n for n in old.namelist() if n.endswith(suffix)]
  assert len(matches)==1 and old.read(matches[0])==p.read_bytes(),p.name
print('PASS: release AArch64, v1.0 metadata, executable modes, payload CRC, embedded guide, exact binary, no QA hooks; fonts match approved 0.3.0.',flush=True)
checks={
 'build':'ARM64 release, GCC 8.3 pinned TG5040 SDK; native link and LGPL relink pass',
 'font':'576 ARM64 renders / 48 NFC pixel comparisons, 16 fonts, 12 scripts, 3 sizes; coverage/Thai/search pass',
 'audio':'10 ARM64 codecs + seek; DSP bypass, ReplayGain, 10/15/31 EQ, limiter pass',
 'persistence':'ARM64 presets/profiles/malformed-data/UTF-8 M3U8 pass',
 'gui':(ROOT/'logs/v1-gui-test.log').read_text().strip(),
 'lifecycle':(ROOT/'logs/v1-lifecycle-test.log').read_text().strip(),
 'hardware':'v1.0 reported running on Brick Pro by user; footer update not tested on physical device here',
 'connections':'simulated only in debug; release uses libmsettings GetJack and /proc/asound USB detection',
 'qa_network':'native GUI QA runs offline; metadata lookup blocked in debug only',
 'install_sha256':hashlib.sha256(install.read_bytes()).hexdigest(),
 'binary_sha256':hashlib.sha256(b).hexdigest()
}
assert checks['gui'].startswith('PASS:') and checks['lifecycle'].count('PASS:')==4
if args.variant:
 checks['variant']=args.variant
 checks['change']='Only the Now Playing footer layout: seven equal cells, centered key/icon pairs, common caption baseline, full remapped key names.'
 checks['baseline_checks']='Font/audio/persistence/GUI/lifecycle results above are from the approved v1.0 baseline; underlying functionality and font assets are unchanged.'
 checks['footer']=(ROOT/'logs/footer-layout-test.log').read_text().strip()
 checks['relink']=(ROOT/'logs/footer-relink-test.log').read_text().strip()
 assert checks['footer'].startswith('PASS:') and checks['relink'].startswith('PASS:')
(ROOT/f'BUILD_REPORT_V1{variant_suffix.upper()}.json').write_text(json.dumps(checks,ensure_ascii=False,indent=2)+'\n')
# Exclude transient SDKs, private host packages, Git state and intermediate builds.
excluded={'dist','backups','.cache','.nxt-sdk','.host-tools','.git','__pycache__'}
files=[]
for p in sorted(ROOT.rglob('*')):
 rel=p.relative_to(ROOT)
 if any(part in excluded for part in rel.parts):continue
 if not p.is_file() or p.name=='FILES_SHA256.json':continue
 if rel.parts[0]=='upstream' and ('build' in rel.parts or p.suffix in ('.o','.d')):continue
 if rel.parts[:3]==('upstream','bin','desktop'):continue
 if rel.parts[0]=='qa' and p.name.endswith('.arm64'):continue
 if rel.parts[0]=='logs' and (p.name.startswith('setup-') or p.name.startswith('host-assets') or p.name.startswith('host-deps') or p.name.startswith('2026') or p.name.startswith('sdk-') or p.name.startswith('build-desktop')):continue
 files.append(p)
manifest={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(ROOT/'FILES_SHA256.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n');files.append(ROOT/'FILES_SHA256.json')
source=OUT/f'NXT_Retro_Music_v1.0{variant_suffix}_Source.zip';prefix=f'NXT_Retro_Music_v1.0{variant_suffix}_Source/'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in files:z.write(p,prefix+p.relative_to(ROOT).as_posix())
with zipfile.ZipFile(source) as z:
 assert z.testzip() is None
 for rel,digest in manifest.items():assert hashlib.sha256(z.read(prefix+rel)).hexdigest()==digest,rel
for p in (install,source):
 p.with_suffix('.zip.sha256').write_text(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n')
print(f'PASS: {len(manifest)} corresponding-source files verified against SHA256; source ZIP CRC pass.',flush=True)
print(json.dumps({'install_bytes':install.stat().st_size,'source_bytes':source.stat().st_size,'install_sha256':checks['install_sha256']},indent=2))
