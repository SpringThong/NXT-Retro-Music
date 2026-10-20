#!/usr/bin/env python3
"""Minimal ALSA-only pkg-config reader for the pinned ARM64 SDK.
Uses actual alsa.pc metadata and real sysroot paths; rejects all other packages.
"""
import os,re,sys
from pathlib import Path
args=sys.argv[1:]
if not args or args[-1]!='alsa':raise SystemExit('Only pinned SDK alsa.pc is supported')
root=Path(os.environ['PKG_CONFIG_SYSROOT_DIR']);pc=root/'usr/lib/pkgconfig/alsa.pc'
variables={};fields={}
for line in pc.read_text().splitlines():
 if not line or line.startswith('#'):continue
 if '=' in line and (':' not in line or line.index('=')<line.index(':')):
  k,v=line.split('=',1);variables[k]=v
 elif ':' in line:
  k,v=line.split(':',1);fields[k]=v.strip()
def expand(s):
 for _ in range(10):
  n=re.sub(r'\$\{([^}]+)\}',lambda m:variables[m[1]],s)
  if n==s:return n
  s=n
 raise ValueError('recursive variable')
if '--modversion' in args:print(fields['Version'])
else:
 out=[]
 for option,field in [('--libs','Libs'),('--cflags','Cflags')]:
  if option in args:
   for word in expand(fields[field]).split():
    if word.startswith(('-L/','-I/')):word=word[:2]+str(root/word[3:])
    out.append(word)
 if not out:raise SystemExit('Unsupported pkg-config query')
 print(' '.join(out))
