#!/usr/bin/env python3
"""Download the pinned official Brick Pro firmware only as a QA reference."""
from pathlib import Path
import hashlib,io,os,urllib.request,zipfile
R=Path(__file__).resolve().parents[1];C=R/'.cache/nx-redux';C.mkdir(parents=True,exist_ok=True)
URL='https://github.com/mohammadsyuhada/nx-redux/releases/download/v2.0.0/NXRedux-v2.0.0-brickpro.zip'
SHA='3e1dd4f8500d761b67ee74ad153f704c649f7ea9952978f423c48ae803209233'
P=C/'NXRedux-v2.0.0-brickpro.zip'
def digest(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
if not P.exists() or digest(P)!=SHA:
 with urllib.request.urlopen(URL,timeout=45) as source,P.with_suffix('.part').open('wb') as out:
  while b:=source.read(1024*1024):out.write(b)
  out.flush();os.fsync(out.fileno())
 assert digest(P.with_suffix('.part'))==SHA;os.replace(P.with_suffix('.part'),P)
with zipfile.ZipFile(P) as out:
 with zipfile.ZipFile(io.BytesIO(out.read('MinUI.zip'))) as z:
  for i in z.infolist():
   if not i.is_dir() and (i.filename.startswith(('.system/lib/','.system/bin/')) or i.filename=='.system/version.txt'):
    assert '..' not in Path(i.filename).parts and not Path(i.filename).is_absolute()
    z.extract(i,C/'root');(C/'root'/i.filename).chmod((i.external_attr>>16&0o777) or 0o644)
print('Verified NX Redux v2.0.0 Brick Pro QA runtime:',SHA)
