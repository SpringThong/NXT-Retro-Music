from pathlib import Path
import subprocess,tempfile,os,json,sqlite3,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';CC=SDK.parents[1]/'bin/aarch64-nextui-linux-gnu-gcc';Q=R/'.host-tools/usr/bin/qemu-aarch64'
with tempfile.TemporaryDirectory(prefix='nxt193-index-arm64-') as td:
 w=Path(td);b=w/'index';subprocess.run([str(CC),'-O2','-D_GNU_SOURCE','-I'+str(S),str(R/'qa/v193_index_driver.c'),str(S/'nxt_library_index.c'),'-lsqlite3','-o',str(b)],check=True)
 root=w/'Music';root.mkdir();p=root/'existing.dff';p.write_text('DSD cached');st=p.stat();db=w/'library.sqlite3'
 with sqlite3.connect(db) as c:
  c.execute('CREATE TABLE tracks(path TEXT PRIMARY KEY,title TEXT,artist TEXT,album TEXT,track INTEGER,mtime INTEGER,size INTEGER,seen INTEGER)');c.execute('INSERT INTO tracks VALUES(?,?,?,?,?,?,?,?)',(str(p),'Existing title','A','B',1,int(st.st_mtime),st.st_size,1));c.execute('CREATE TABLE listener_favorites(listener INTEGER,path TEXT,PRIMARY KEY(listener,path))');c.execute('INSERT INTO listener_favorites VALUES(4,?)',(str(p),))
 env={**os.environ,'LD_LIBRARY_PATH':':'.join(map(str,[R/'nxredux/bin/tg5040',R/'upstream/bin/tg5040',SDK/'lib',SDK/'usr/lib']))};runs=[]
 def run():
  p=subprocess.run([str(Q),'-L',str(SDK),str(b),str(db),str(root),'0','0','0','0'],env=env,capture_output=True,text=True,timeout=30);assert not p.returncode,p.stdout+p.stderr;value=json.loads(p.stdout);runs.append(value);return value
 x=run();assert x['complete'] and x['decoded']==0 and x['writes']==1,x
 before=hashlib.sha256(db.read_bytes()).hexdigest();x=run();assert x['complete'] and x['decoded']==x['writes']==x['commits']==0,x;assert hashlib.sha256(db.read_bytes()).hexdigest()==before
 (root/'new.flac').write_text('New title');x=run();assert x['added']==1 and x['decoded']==1 and x['writes']==2,x
 with sqlite3.connect(db) as c:assert c.execute('SELECT COUNT(*) FROM listener_favorites').fetchone()[0]==1;assert c.execute('SELECT COUNT(*) FROM tracks').fetchone()[0]==2
 report={'exit':0,'actual_ARM64_engine_with_pinned_SDK_SQLite':True,'legacy_8_column_migration':True,'warm_zero_media_reads_and_index_writes':True,'new_track_and_favorites':True,'runs':runs,'physical_ROM_tested':False};(R/'logs/v193-index-arm64.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS ARM64 index with firmware SDK SQLite: migration, warm zero writes, new DFF/FLAC paths, favorites retained')
