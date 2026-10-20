/* SPDX-License-Identifier: GPL-3.0-only
 * Incremental SD library. Fixed batches, no per-track seen writes, no media
 * reads for unchanged files, and no destructive prune after an incomplete walk. */
#include "nxt_library_index.h"
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define INDEX_BATCH 16
#define INDEX_PAGE 32
#define INDEX_PATH 512
#define INDEX_DEPTH 64
typedef struct {char path[INDEX_PATH];NxtIndexTrack info;struct stat st;int type;} Pending; /* 0 upsert, 1 signature migration, 2 delete */
typedef struct {sqlite3*db;const NxtIndexHooks*h;NxtIndexResult*r;char root[INDEX_PATH];struct stat root_stat;bool recursive,force,healthy;Pending pending[INDEX_BATCH];int count;sqlite3_stmt*lookup;} Scan;
static bool sql(sqlite3*d,const char*s){return sqlite3_exec(d,s,NULL,NULL,NULL)==SQLITE_OK;}
bool NxtIndex_schema(sqlite3*d){
 /* Keep the original eight-column tracks table compatible with the previous
  * release/OTA backup. File stamps live in a separate optional table. */
 return sql(d,"CREATE TABLE IF NOT EXISTS tracks(path TEXT PRIMARY KEY,title TEXT,artist TEXT,album TEXT,track INTEGER,mtime INTEGER,size INTEGER,seen INTEGER);CREATE TABLE IF NOT EXISTS library_signatures(path TEXT PRIMARY KEY,mtime INTEGER,size INTEGER,mtime_ns INTEGER,ctime INTEGER,ctime_ns INTEGER)");
}
bool NxtIndex_inScope(const char*p,const char*root,bool recursive){
 if(!p||!root||!*root)return false;size_t n=strlen(root);while(n>1&&root[n-1]=='/')n--;
 if(strncmp(p,root,n))return false;const char*rest=p+n;
 if(n==1&&*root=='/'){if(!*rest)return false;}else{if(*rest!='/')return false;rest++;}
 return *rest&&(recursive||!strchr(rest,'/'));
}
static bool checkpoint(Scan*s){if(s->h->checkpoint&&!s->h->checkpoint(s->h->context)){s->r->cancelled=true;return false;}return true;}
static bool same_root(Scan*s){struct stat st;return !lstat(s->root,&st)&&S_ISDIR(st.st_mode)&&!S_ISLNK(st.st_mode)&&st.st_dev==s->root_stat.st_dev&&st.st_ino==s->root_stat.st_ino;}
static bool stamp_equal(const struct stat*a,const struct stat*b){return a->st_size==b->st_size&&a->st_mtim.tv_sec==b->st_mtim.tv_sec&&a->st_mtim.tv_nsec==b->st_mtim.tv_nsec&&a->st_ctim.tv_sec==b->st_ctim.tv_sec&&a->st_ctim.tv_nsec==b->st_ctim.tv_nsec&&a->st_dev==b->st_dev&&a->st_ino==b->st_ino;}
static void progress(Scan*s){if(s->h->progress)s->h->progress(s->r,s->h->context);}
static bool flush(Scan*s){
 if(!s->count)return true;if(!checkpoint(s)||!same_root(s)){s->r->unavailable=!s->r->cancelled;return false;}
 int attempts=0;
 while(!sql(s->db,"BEGIN IMMEDIATE")){if((sqlite3_errcode(s->db)!=SQLITE_BUSY&&sqlite3_errcode(s->db)!=SQLITE_LOCKED)||++attempts>=20||!checkpoint(s))return false;usleep(5000);}
 int changes=0,visible=0;bool ok=true;
 for(int i=0;i<s->count&&ok;i++){
  Pending*p=&s->pending[i];sqlite3_stmt*q=NULL;
  if(p->type!=1){
   const char*statement=p->type==2?"DELETE FROM tracks WHERE path=?":"INSERT OR REPLACE INTO tracks(path,title,artist,album,track,mtime,size,seen) VALUES(?,?,?,?,?,?,?,0)";
   ok=sqlite3_prepare_v2(s->db,statement,-1,&q,NULL)==SQLITE_OK;
   if(ok){sqlite3_bind_text(q,1,p->path,-1,SQLITE_TRANSIENT);if(p->type==0){sqlite3_bind_text(q,2,p->info.title,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,3,p->info.artist,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,4,p->info.album,-1,SQLITE_TRANSIENT);sqlite3_bind_int(q,5,p->info.track);sqlite3_bind_int64(q,6,p->st.st_mtime);sqlite3_bind_int64(q,7,p->st.st_size);}ok=sqlite3_step(q)==SQLITE_DONE;if(ok){int count=sqlite3_changes(s->db);changes+=count;visible+=count;}}sqlite3_finalize(q);q=NULL;
  }
  if(ok){
   ok=sqlite3_prepare_v2(s->db,p->type==2?"DELETE FROM library_signatures WHERE path=?":"INSERT OR REPLACE INTO library_signatures(path,mtime,size,mtime_ns,ctime,ctime_ns) VALUES(?,?,?,?,?,?)",-1,&q,NULL)==SQLITE_OK;
   if(ok){sqlite3_bind_text(q,1,p->path,-1,SQLITE_TRANSIENT);if(p->type!=2){sqlite3_bind_int64(q,2,p->st.st_mtime);sqlite3_bind_int64(q,3,p->st.st_size);sqlite3_bind_int64(q,4,p->st.st_mtim.tv_nsec);sqlite3_bind_int64(q,5,p->st.st_ctim.tv_sec);sqlite3_bind_int64(q,6,p->st.st_ctim.tv_nsec);}ok=sqlite3_step(q)==SQLITE_DONE;if(ok)changes+=sqlite3_changes(s->db);}sqlite3_finalize(q);
  }
 }
 if(ok)ok=checkpoint(s)&&same_root(s);if(ok)ok=sql(s->db,"COMMIT");
 if(!ok){sql(s->db,"ROLLBACK");return false;}
 s->r->writes+=changes;s->r->commits++;s->count=0;if(visible&&s->h->changed)s->h->changed(s->h->context);progress(s);return true;
}
static bool queue(Scan*s,const char*path,const struct stat*st,const NxtIndexTrack*info,int type){
 Pending*p=&s->pending[s->count++];memset(p,0,sizeof(*p));snprintf(p->path,sizeof(p->path),"%s",path);p->type=type;if(st)p->st=*st;if(info)p->info=*info;
 return s->count<INDEX_BATCH||flush(s);
}
static bool visit(Scan*s,const char*path,const struct stat*st){
 sqlite3_stmt*q=s->lookup;sqlite3_reset(q);sqlite3_clear_bindings(q);sqlite3_bind_text(q,1,path,-1,SQLITE_TRANSIENT);int state=sqlite3_step(q);bool found=state==SQLITE_ROW;
 if(!found&&state!=SQLITE_DONE){sqlite3_reset(q);return false;}
 bool basic=found&&sqlite3_column_int64(q,0)==st->st_mtime&&sqlite3_column_int64(q,1)==st->st_size;
 bool legacy=found&&sqlite3_column_type(q,2)==SQLITE_NULL;
 bool unchanged=basic&&(legacy||(sqlite3_column_int64(q,2)==st->st_mtim.tv_nsec&&sqlite3_column_int64(q,3)==st->st_ctim.tv_sec&&sqlite3_column_int64(q,4)==st->st_ctim.tv_nsec));
 sqlite3_reset(q);s->r->checked++;progress(s);
 if(unchanged&&!s->force)return !legacy||queue(s,path,st,NULL,1);
 if(!checkpoint(s))return false;
 /* Release the disk between metadata probes; never hold a DB write lock while
  * decoding. A scan has idle I/O priority and cannot flood the playback path. */
 usleep(8000);NxtIndexTrack info={0};s->r->decoded++;
 if(!s->h->probe(path,&info,s->h->context)){s->r->deferred++;return checkpoint(s);}
 struct stat after;if(lstat(path,&after)||!S_ISREG(after.st_mode)||!stamp_equal(st,&after)){s->r->deferred++;return checkpoint(s);}
 if(!checkpoint(s))return false;
 if(found)s->r->updated++;else s->r->added++;return queue(s,path,st,&info,0);
}
static bool walk(Scan*s,const char*path,int depth){
 if(!checkpoint(s))return false;if(depth>INDEX_DEPTH){s->healthy=false;return true;}
 DIR*d=opendir(path);if(!d){s->healthy=false;return true;}
 bool ok=true;unsigned entries=0;
 for(;;){errno=0;struct dirent*e=readdir(d);if(!e){if(errno)s->healthy=false;break;}
  if(e->d_name[0]=='.')continue;if(!checkpoint(s)){ok=false;break;}
  char full[INDEX_PATH];int n=snprintf(full,sizeof(full),"%s%s%s",path,!strcmp(path,"/")?"":"/",e->d_name);if(n<0||n>=(int)sizeof(full)){s->healthy=false;continue;}
  struct stat st;if(lstat(full,&st)){s->healthy=false;continue;}if(S_ISLNK(st.st_mode))continue;
  if(S_ISDIR(st.st_mode)){if(s->recursive&&!walk(s,full,depth+1)){ok=false;break;}}
  else if(S_ISREG(st.st_mode)&&s->h->audio(full,s->h->context)&&!visit(s,full,&st)){ok=false;break;}
  if(++entries%16==0)usleep(2000);
 }
 closedir(d);return ok;
}
static bool prune(Scan*s){
 /* Keyset pages are closed before any DELETE; never mutate a live SELECT
  * cursor over the same table. Only definitive absence removes an index row. */
 char last[INDEX_PATH]="",paths[INDEX_PAGE][INDEX_PATH];bool ok=true;
 while(ok&&checkpoint(s)){
  sqlite3_stmt*q=NULL;if(sqlite3_prepare_v2(s->db,"SELECT path FROM tracks WHERE path>? ORDER BY path LIMIT 32",-1,&q,NULL)!=SQLITE_OK)return false;sqlite3_bind_text(q,1,last,-1,SQLITE_TRANSIENT);int count=0,state;
  while((state=sqlite3_step(q))==SQLITE_ROW){const char*p=(const char*)sqlite3_column_text(q,0);if(!p||strlen(p)>=INDEX_PATH){ok=false;break;}snprintf(paths[count++],INDEX_PATH,"%s",p);}
  sqlite3_finalize(q);if(!ok||state!=SQLITE_DONE)return false;if(!count)break;snprintf(last,sizeof(last),"%s",paths[count-1]);
  for(int i=0;i<count;i++){
   if(!checkpoint(s))return false;if(!NxtIndex_inScope(paths[i],s->root,s->recursive))continue;
   struct stat st;bool gone=false;if(!lstat(paths[i],&st))gone=!S_ISREG(st.st_mode)||S_ISLNK(st.st_mode);else gone=errno==ENOENT||errno==ENOTDIR;
   if(gone){if(!same_root(s)){s->r->unavailable=true;return false;}s->r->removed++;if(!queue(s,paths[i],NULL,NULL,2))return false;}
  }usleep(2000);
 }
 return ok&&!s->r->cancelled;
}
bool NxtIndex_run(const char*dbpath,const char*root,bool recursive,bool force,const NxtIndexHooks*h,NxtIndexResult*out){
 if(!out)return false;memset(out,0,sizeof(*out));if(!dbpath||!root||!*root||strlen(root)>=INDEX_PATH||!h||!h->audio||!h->probe)return false;
 Scan*s=calloc(1,sizeof(*s));if(!s)return false;s->h=h;s->r=out;s->recursive=recursive;s->force=force;s->healthy=true;snprintf(s->root,sizeof(s->root),"%s",root);size_t n=strlen(s->root);while(n>1&&s->root[n-1]=='/')s->root[--n]=0;
 if(lstat(s->root,&s->root_stat)||!S_ISDIR(s->root_stat.st_mode)||S_ISLNK(s->root_stat.st_mode)){out->unavailable=true;free(s);return false;}
 bool ok=sqlite3_open_v2(dbpath,&s->db,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,NULL)==SQLITE_OK;
 if(ok){sqlite3_busy_timeout(s->db,50);sql(s->db,"PRAGMA cache_size=-1024");ok=sqlite3_prepare_v2(s->db,"SELECT t.mtime,t.size,s.mtime_ns,s.ctime,s.ctime_ns FROM tracks t LEFT JOIN library_signatures s ON s.path=t.path AND s.mtime=t.mtime AND s.size=t.size WHERE t.path=?",-1,&s->lookup,NULL)==SQLITE_OK;}
 if(ok)ok=walk(s,s->root,0);if(ok)ok=flush(s);if(ok&&s->healthy&&same_root(s))ok=prune(s);else if(ok&&!same_root(s)){out->unavailable=true;ok=false;}
 if(ok)ok=flush(s);out->complete=ok&&s->healthy&&!out->deferred&&!out->cancelled&&!out->unavailable;
 sqlite3_finalize(s->lookup);if(s->db)sqlite3_close(s->db);free(s);return ok;
}
