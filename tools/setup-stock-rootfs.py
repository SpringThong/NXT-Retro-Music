from pathlib import Path
import tempfile,urllib.request,hashlib,subprocess,json,os
R=Path(__file__).resolve().parents[1];meta=json.loads((R/'STOCK_SOURCE.json').read_text())['official_reference']['firmware'];w=Path(tempfile.mkdtemp(prefix='nxt-stock-fw-'));p=w/'firmware.7z';sha=hashlib.sha256()
with urllib.request.urlopen(meta['url'],timeout=40) as r,p.open('wb') as f:
 while b:=r.read(1024*1024):f.write(b);sha.update(b)
assert sha.hexdigest()==meta['sha256'];print('verified firmware',flush=True)
subprocess.run([str(R/'.host-tools/usr/lib/7zip/7z'),'x','-y','-o'+str(w),str(p)],stdout=subprocess.DEVNULL,check=True)
fw=next(w.rglob('*.awimg'));assert fw.stat().st_size==584305664
with fw.open('rb') as f,(w/'rootfs.ext4').open('wb') as out:
 f.seek(0x113c000);n=0
 while n<566231040:
  b=f.read(min(1024*1024,566231040-n));assert b;out.write(b);n+=len(b)
root=R/'.cache/stock-research/rootfs';root.mkdir(parents=True,exist_ok=True)
with (R/'logs/v1.3-rootfs-extract.log').open('w') as log:subprocess.run(['/usr/sbin/debugfs','-R','rdump / '+str(root),str(w/'rootfs.ext4')],stdout=log,stderr=log,check=True)
for p in root.rglob('*'):
 if not p.is_symlink():os.utime(p,None)
print('official rootfs ready',flush=True)
