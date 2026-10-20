from pathlib import Path
import subprocess,tempfile,sqlite3,os,json,hashlib,time
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/v193-host-include';H.mkdir(exist_ok=True);(H/'sqlite3.h').write_bytes((R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include/sqlite3.h').read_bytes());env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
with tempfile.TemporaryDirectory(prefix='nxt193-index-') as td:
 w=Path(td);b=w/'index';subprocess.run(['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-fsanitize=address,undefined','-I'+str(H),'-I'+str(S),str(R/'qa/v193_index_driver.c'),str(S/'nxt_library_index.c'),'-l:libsqlite3.so.0','-o',str(b)],check=True)
 root=w/'Music';root.mkdir();db=w/'library.sqlite3';con=sqlite3.connect(db);con.executescript('CREATE TABLE tracks(path TEXT PRIMARY KEY,title TEXT,artist TEXT,album TEXT,track INTEGER,mtime INTEGER,size INTEGER,seen INTEGER); CREATE TABLE listener_favorites(listener INTEGER,path TEXT,PRIMARY KEY(listener,path));');con.commit()
 def put(p,text):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text);return p
 for i in range(3):
  p=put(root/f'{i}.flac',f'old-title-{i}');st=p.stat();con.execute('INSERT INTO tracks VALUES(?,?,?,?,?,?,?,?)',(str(p),f'cached-{i}','Artist','Album',i,int(st.st_mtime),st.st_size,100))
 con.execute('INSERT INTO listener_favorites VALUES(7,?)',(str(root/'0.flac'),));con.commit();con.close()
 runs=[]
 def run(force=0,stop=0,mutate=0,move=0,path=root):
  p=subprocess.run([str(b),str(db),str(path),str(force),str(stop),str(mutate),str(move)],capture_output=True,text=True,env=env,timeout=60);assert p.returncode==0,p.stdout+p.stderr;r=json.loads(p.stdout);runs.append(r);return r
 def rows():
  with sqlite3.connect(db) as c:return dict(c.execute('SELECT path,title FROM tracks'))
 x=run();
 with sqlite3.connect(db) as c:assert len(c.execute('PRAGMA table_info(tracks)').fetchall())==8
 assert x['complete'] and x['decoded']==0 and x['writes']==3 and x['commits']==1,x
 initial_hash=hashlib.sha256(db.read_bytes()).hexdigest();x=run();assert x['complete'] and x['decoded']==x['writes']==x['commits']==0,x;assert hashlib.sha256(db.read_bytes()).hexdigest()==initial_hash
 assert rows()[str(root/'0.flac')]=='cached-0'
 # Copy into an already-existing nested directory, even with root mtime restored.
 nested=root/'Artist/Album';nested.mkdir(parents=True);stamp=root.stat().st_mtime_ns
 p=put(nested/'Nhạc mới.dff','new-DSD');os.utime(root,ns=(stamp,stamp));x=run();assert x['decoded']==x['added']==1 and x['writes']==2 and x['complete'],x
 # Same length and preserved mtime still changes ctime/ns.
 p=root/'0.flac';st=p.stat();p.write_text('NEW-title-0');os.utime(p,ns=(st.st_atime_ns,st.st_mtime_ns));x=run();assert x['decoded']==x['updated']==1 and x['writes']==2,x;assert rows()[str(p)]=='NEW-title-0'
 (root/'1.flac').rename(root/'renamed.flac');(root/'2.flac').unlink();x=run();assert x['added']==1 and x['removed']==2 and x['writes']==6,x
 put(root/'.nxt-upload-1.part','hidden');put(root/'unfinished.flac.part','partial');(root/'link.flac').symlink_to(root/'0.flac');(root/'cycle').symlink_to(root,target_is_directory=True);put(root/'cover.png','image');x=run();assert x['writes']==x['decoded']==0,x
 before=rows();p=put(root/'growing.flac','copy');x=run(mutate=1);assert x['deferred']==1 and str(p) not in rows(),x;x=run();assert x['added']==1 and x['decoded']==1,x
 p=put(root/'invalid.flac','BAD file');x=run();assert x['deferred']==1 and str(p) not in rows(),x;p.unlink()
 # Cancellation never sweeps untouched records based on a partial seen pass.
 before=rows();(root/'0.flac').unlink();x=run(stop=4);assert x['cancelled'] and not x['complete'] and rows()==before,x
 x=run();assert x['removed']==1 and x['complete'],x
 before=rows();(root/'new.flac').write_text('new');x=run(move=1);assert x['unavailable'] and not x['complete'] and rows()==before,x
 (w/'Music-offline').rename(root);x=run();assert x['added']==1 and x['complete'],x
 (root).rename(w/'Music-offline');before=rows();x=run();assert x['unavailable'] and rows()==before,x;(w/'Music-offline').rename(root)
 # Force explicitly re-reads metadata; favorites and other cached roots survive.
 other=w/'Music2';put(other/'else.flac','other-root');x=run(path=other);assert x['added']==1 and str(other/'else.flac') in rows(),x
 x=run(force=1);assert x['decoded']==4 and x['writes']==8,x
 with sqlite3.connect(db) as c:assert c.execute('SELECT COUNT(*) FROM listener_favorites WHERE listener=7').fetchone()[0]==1
 x=run();assert x['decoded']==x['writes']==0,x
 # A failed SQL batch rolls back and preserves the old library.
 put(root/'sql-fail.flac','fail');before=rows()
 with sqlite3.connect(db) as c:c.execute("CREATE TRIGGER reject_insert BEFORE INSERT ON tracks BEGIN SELECT RAISE(ABORT,'fixture'); END")
 x=run();assert not x['ok'] and rows()==before,x
 with sqlite3.connect(db) as c:c.execute('DROP TRIGGER reject_insert')
 (root/'sql-fail.flac').unlink()
 # Large legacy migration: no media probes and fixed-size write batches.
 large=w/'Big';large.mkdir()
 with sqlite3.connect(db) as c:
  for i in range(2048):
   p=put(large/f'{i}.flac','x');st=p.stat();c.execute('INSERT INTO tracks(path,title,artist,album,track,mtime,size,seen) VALUES(?,?,?,?,?,?,?,?)',(str(p),'Cached','A','B',i,int(st.st_mtime),st.st_size,1))
 x=run(path=large);assert x['decoded']==0 and x['writes']==2048 and x['commits']==128,x
 before=hashlib.sha256(db.read_bytes()).hexdigest();x=run(path=large);assert x['complete'] and x['checked']==2048 and x['decoded']==x['writes']==x['commits']==0,x;assert hashlib.sha256(db.read_bytes()).hexdigest()==before
 report={'exit':0,'ASan_UBSan':True,'legacy_favorites_and_metadata_preserved':True,'warm_2048_tracks_zero_media_probes_zero_index_writes':True,'warm_database_bytes_unchanged':True,'nested_add_rename_delete_same_size_timestamp_change':True,'hidden_partial_symlink_ignored':True,'growing_file_deferred':True,'missing_root_and_cancellation_preserve_index':True,'original_tracks_schema_backward_compatible':True,'SQL_failure_batch_rollback':True,'explicit_full_rebuild':True,'fixed_16_track_batches':True,'runs':runs,'hardware_tested':False,'engine_sha256':hashlib.sha256((S/'nxt_library_index.c').read_bytes()).hexdigest()}
 (R/'logs/v194-index.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS persistent incremental index: legacy migration, warm zero writes/probes, nested changes, partial copies, cancellation, missing card, rollback and bounded 2048-track fixture')
