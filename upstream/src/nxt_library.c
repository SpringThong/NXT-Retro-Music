#include "nxt_personal.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "nxt_decoder.h"
#include "nxt_text.h"
#include "nxt_library_index.h"
#include "nxt_transfer.h"
#include "nxt_log.h"
#include "api.h"
#include <sys/resource.h>
#include <sys/syscall.h>
#include "file_utils.h"
#include "browser.h"
#include "module_player.h"
#include "module_playlist.h"
#include "module_downloader.h"
#include "playlist_m3u.h"
#include "keyboard.h"
#include "toast.h"
#include "nxt_sort.h"
#include <sqlite3.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct { char label[600],path[512],artist[256],album[256]; } LibraryRow;
static sqlite3 *library;
static pthread_t scanner;static bool scanning,scanner_joinable,cancel_scan;
static pthread_mutex_t scan_lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t scan_wake=PTHREAD_COND_INITIALIZER;
static char dbpath[1024],scan_root[512];static int scan_count;static bool scan_recursive,scan_force;
static bool index_enabled,index_pending,index_force,index_paused,index_dirty;static unsigned index_revision;
static char checked_root[512];static int checked_recursive=-1;static uint32_t index_due;static int index_retries;
static NxtIndexResult index_result;
static void scope_key(sqlite3_context*ctx,int n,sqlite3_value**v){(void)n;sqlite3_result_int(ctx,NxtIndex_inScope((const char*)sqlite3_value_text(v[0]),nxt.music_root,nxt.recursive));}
static void search_key(sqlite3_context *ctx,int argc,sqlite3_value **args){(void)argc;char *key=NxtText_searchKey((const char*)sqlite3_value_text(args[0]));if(key)sqlite3_result_text(ctx,key,-1,free);else sqlite3_result_error_nomem(ctx);}
static int natural_collation(void *unused,int a_len,const void *a,int b_len,const void *b) {
    (void)unused;char *as=strndup(a,a_len),*bs=strndup(b,b_len);int result=NxtSort_compare(as,bs);free(as);free(bs);return result;
}
static bool init(void) {
    if(library)return true;userdata_mkdir("");userdata_snpath("library.sqlite3",dbpath,sizeof(dbpath));
    if(sqlite3_open_v2(dbpath,&library,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX,NULL)!=SQLITE_OK)goto failed;
    sqlite3_create_collation(library,"NXT_NATURAL",SQLITE_UTF8,NULL,natural_collation);sqlite3_busy_timeout(library,50);
    sqlite3_create_function(library,"nxt_key",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,NULL,search_key,NULL,NULL);
    sqlite3_create_function(library,"nxt_scope",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,NULL,scope_key,NULL,NULL);
    const char*schema="PRAGMA journal_mode=WAL;PRAGMA cache_size=-1024;CREATE TABLE IF NOT EXISTS favorites(path TEXT PRIMARY KEY);CREATE INDEX IF NOT EXISTS artists ON tracks(artist);CREATE INDEX IF NOT EXISTS albums ON tracks(album);CREATE TABLE IF NOT EXISTS listener_favorites(listener INTEGER,path TEXT,PRIMARY KEY(listener,path));CREATE TABLE IF NOT EXISTS library_meta(key TEXT PRIMARY KEY);";
    if(!NxtIndex_schema(library)||sqlite3_exec(library,schema,NULL,NULL,NULL)!=SQLITE_OK)goto failed;
    sqlite3_stmt*q=NULL;bool migrated=false;
    if(sqlite3_prepare_v2(library,"SELECT 1 FROM library_meta WHERE key='profile-favorites'",-1,&q,NULL)==SQLITE_OK)migrated=sqlite3_step(q)==SQLITE_ROW;sqlite3_finalize(q);
    if(!migrated&&sqlite3_exec(library,"BEGIN;INSERT OR IGNORE INTO listener_favorites SELECT 0,path FROM favorites;INSERT OR IGNORE INTO library_meta VALUES('profile-favorites');COMMIT",NULL,NULL,NULL)!=SQLITE_OK){sqlite3_exec(library,"ROLLBACK",NULL,NULL,NULL);goto failed;}
    return true;
failed:
    if(library)sqlite3_close(library);library=NULL;return false;
}
static bool scan_checkpoint(void*unused){
 (void)unused;pthread_mutex_lock(&scan_lock);while(index_paused&&!cancel_scan)pthread_cond_wait(&scan_wake,&scan_lock);bool ok=!cancel_scan;pthread_mutex_unlock(&scan_lock);return ok;
}
static bool index_audio(const char*p,void*u){(void)u;return Player_detectFormat(p)!=AUDIO_FORMAT_UNKNOWN;}
static bool index_probe(const char*p,NxtIndexTrack*out,void*u){(void)u;TrackInfo info;if(NxtTransfer_isBusy()||!NxtDecoder_probe(p,&info)||NxtTransfer_isBusy())return false;snprintf(out->title,sizeof(out->title),"%s",info.title);snprintf(out->artist,sizeof(out->artist),"%s",info.artist);snprintf(out->album,sizeof(out->album),"%s",info.album);out->track=info.track_number;return true;}
static void index_changed(void*u){(void)u;__atomic_add_fetch(&index_revision,1,__ATOMIC_RELAXED);}
static void index_progress(const NxtIndexResult*r,void*u){(void)u;pthread_mutex_lock(&scan_lock);scan_count=r->checked;pthread_mutex_unlock(&scan_lock);}
static void *scan(void *unused) {
    (void)unused;setpriority(PRIO_PROCESS,0,10);
#ifdef SYS_ioprio_set
    syscall(SYS_ioprio_set,1,0,3<<13); /* Linux per-thread idle I/O; best effort. */
#endif
    NxtIndexHooks hooks={index_audio,index_probe,scan_checkpoint,index_changed,index_progress,NULL};NxtIndexResult result;
    bool ok=NxtIndex_run(dbpath,scan_root,scan_recursive,scan_force,&hooks,&result);
    NxtLog_step("library","index",ok,"checked=%d; decoded=%d; added=%d; updated=%d; removed=%d; writes=%d; commits=%d; deferred=%d; complete=%d; cancelled=%d; unavailable=%d",result.checked,result.decoded,result.added,result.updated,result.removed,result.writes,result.commits,result.deferred,result.complete,result.cancelled,result.unavailable);
    pthread_mutex_lock(&scan_lock);index_result=result;scanning=false;pthread_mutex_unlock(&scan_lock);return NULL;
}
/* App/UI thread owns scheduling and DB access. The upload server only sets an
 * atomic dirty flag, so it never touches the UI or its SQLite connection. */
void NxtLibrary_notify(void){__atomic_store_n(&index_dirty,true,__ATOMIC_RELEASE);}
void NxtLibrary_start(void){index_enabled=true;}
void NxtLibrary_request(bool full){index_enabled=true;index_pending=true;index_force|=full;index_retries=0;index_due=SDL_GetTicks()+50;}
unsigned NxtLibrary_revision(void){return __atomic_load_n(&index_revision,__ATOMIC_RELAXED);}
void NxtLibrary_tick(void){
    if(!index_enabled)return;uint32_t now=SDL_GetTicks();
    bool paused=NxtSession_isHidden()||PWR_getDisplayOff()||NxtTransfer_isBusy();pthread_mutex_lock(&scan_lock);bool was_paused=index_paused;index_paused=paused;bool active=scanning;if(was_paused&&!paused)pthread_cond_broadcast(&scan_wake);pthread_mutex_unlock(&scan_lock);
    if(strcmp(checked_root,nxt.music_root)||checked_recursive!=nxt.recursive){
        snprintf(checked_root,sizeof(checked_root),"%s",nxt.music_root);checked_recursive=nxt.recursive;index_pending=true;index_retries=0;index_due=now+1500;
        if(active){pthread_mutex_lock(&scan_lock);cancel_scan=true;pthread_cond_broadcast(&scan_wake);pthread_mutex_unlock(&scan_lock);}
    }
    if(__atomic_exchange_n(&index_dirty,false,__ATOMIC_ACQ_REL)){index_pending=true;index_due=now+1000;index_retries=0;}
    if(active)return;
    if(scanner_joinable){pthread_join(scanner,NULL);scanner_joinable=false;
        if(!index_result.complete&&!index_pending&&index_retries<2){index_retries++;index_pending=true;index_due=now+3000;}
    }
    if(paused||!index_pending||(int32_t)(now-index_due)<0)return;
    if(!init()){index_pending=false;NxtLog_step("library","open",false,"Cannot open saved library index");return;}
    snprintf(scan_root,sizeof(scan_root),"%s",nxt.music_root);scan_recursive=nxt.recursive;scan_force=index_force;
    index_pending=index_force=false;pthread_mutex_lock(&scan_lock);cancel_scan=false;scan_count=0;scanning=true;memset(&index_result,0,sizeof(index_result));pthread_mutex_unlock(&scan_lock);
    scanner_joinable=pthread_create(&scanner,NULL,scan,NULL)==0;if(!scanner_joinable){pthread_mutex_lock(&scan_lock);scanning=false;pthread_mutex_unlock(&scan_lock);NxtLog_step("library","thread",false,"Cannot start background index check");}
}
static void refresh(void){NxtLibrary_request(false);}
void Nxt_libraryQuit(void){
    pthread_mutex_lock(&scan_lock);bool active=scanning;cancel_scan=true;pthread_cond_broadcast(&scan_wake);pthread_mutex_unlock(&scan_lock);
    if(scanner_joinable){pthread_join(scanner,NULL);scanner_joinable=false;}if(active){index_pending=true;index_due=SDL_GetTicks()+1500;}
    if(library)sqlite3_close(library);library=NULL;
}
bool Nxt_isFavorite(const char *path){if(!init())return false;sqlite3_stmt *q=NULL;sqlite3_prepare_v2(library,"SELECT 1 FROM listener_favorites WHERE path=? AND listener=?",-1,&q,NULL);sqlite3_bind_text(q,1,path,-1,SQLITE_TRANSIENT);sqlite3_bind_int(q,2,NxtPersonal_id());bool found=sqlite3_step(q)==SQLITE_ROW;sqlite3_finalize(q);return found;}
void Nxt_favorite(const char *path){
 if(!path||!*path||!init())return;bool favorite=Nxt_isFavorite(path);sqlite3_stmt*q=NULL;
 bool ok=sqlite3_prepare_v2(library,favorite?"DELETE FROM listener_favorites WHERE path=? AND listener=?":"INSERT OR IGNORE INTO listener_favorites(path,listener) VALUES(?,?)",-1,&q,NULL)==SQLITE_OK;
 if(ok){sqlite3_bind_text(q,1,path,-1,SQLITE_TRANSIENT);sqlite3_bind_int(q,2,NxtPersonal_id());ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);
 if(ok)index_changed(NULL);Toast_show(!ok?Nxt_text("Cannot save favorite; please retry","Chưa lưu được yêu thích; hãy thử lại"):favorite?Nxt_text("Removed from favorites","Đã bỏ yêu thích"):Nxt_text("Added to favorites","Đã thêm yêu thích"),2000);
}
static LibraryRow *query(int mode,const char *artist,const char *album,const char *search,bool order,int *count) {
    *count=0;if(!init())return NULL;const char *sql;
    if(mode==1)sql="SELECT DISTINCT artist,'',artist,'' FROM tracks WHERE nxt_scope(path) ORDER BY artist COLLATE NXT_NATURAL";
    else if(mode==2)sql="SELECT DISTINCT album,'',artist,album FROM tracks WHERE nxt_scope(path) ORDER BY album COLLATE NXT_NATURAL,artist";
    else if(mode==4)sql="SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND path IN(SELECT path FROM listener_favorites WHERE listener=?) ORDER BY title COLLATE NXT_NATURAL";
    else if(search&&*search)sql="SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND (instr(nxt_key(title),nxt_key(?))>0 OR instr(nxt_key(artist),nxt_key(?))>0 OR instr(nxt_key(album),nxt_key(?))>0) ORDER BY title COLLATE NXT_NATURAL";
    else if(album)sql=order?"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND artist=? AND album=? ORDER BY track,title COLLATE NXT_NATURAL":"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND artist=? AND album=? ORDER BY title COLLATE NXT_NATURAL";
    else if(artist)sql=order?"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND artist=? ORDER BY album,track,title COLLATE NXT_NATURAL":"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) AND artist=? ORDER BY title COLLATE NXT_NATURAL";
    else sql=order?"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) ORDER BY album,track,title COLLATE NXT_NATURAL":"SELECT title,path,artist,album FROM tracks WHERE nxt_scope(path) ORDER BY title COLLATE NXT_NATURAL";
    sqlite3_stmt *q=NULL;if(sqlite3_prepare_v2(library,sql,-1,&q,NULL)!=SQLITE_OK)return NULL;
    if(search&&*search){char pattern[600];snprintf(pattern,sizeof(pattern),"%s",search);for(int i=1;i<=3;i++)sqlite3_bind_text(q,i,pattern,-1,SQLITE_TRANSIENT);}else if(artist){sqlite3_bind_text(q,1,artist,-1,SQLITE_TRANSIENT);if(album)sqlite3_bind_text(q,2,album,-1,SQLITE_TRANSIENT);}
    if(mode==4)sqlite3_bind_int(q,1,NxtPersonal_id());
    int cap=128;LibraryRow *rows=malloc(cap*sizeof(*rows));if(!rows){sqlite3_finalize(q);return NULL;}
    while(sqlite3_step(q)==SQLITE_ROW){if(*count==cap){cap*=2;LibraryRow *r=realloc(rows,cap*sizeof(*rows));if(!r)break;rows=r;}LibraryRow *r=&rows[(*count)++];const char *title=(const char*)sqlite3_column_text(q,0);snprintf(r->path,sizeof(r->path),"%s",sqlite3_column_text(q,1));NxtText_copy(r->artist,(const char*)sqlite3_column_text(q,2),sizeof(r->artist));NxtText_copy(r->album,(const char*)sqlite3_column_text(q,3),sizeof(r->album));snprintf(r->label,sizeof(r->label),"%s%s%s",title&&*title?title:Nxt_text("Unknown","Chưa rõ"),mode!=1&&r->artist[0]?"\n":"",mode!=1?r->artist:"");}
    sqlite3_finalize(q);return rows;
}
static void add_playlist(const LibraryRow *row,DisplayContext *display) {
    PlaylistInfo lists[MAX_PLAYLISTS];int count=M3U_listPlaylists(lists,MAX_PLAYLISTS),selected=0;const char *items[MAX_PLAYLISTS];for(int i=0;i<count;i++)items[i]=lists[i].name;
    NxtPick p=Nxt_pick(display,Nxt_text("Add to playlist","Thêm vào danh sách phát"),items,count,&selected,Nxt_text("A Add • B Back","A Thêm • B Quay lại"));if(p.action==1&&selected<count){int r=M3U_addTrack(lists[selected].path,row->path,row->label);Toast_show(r==0?Nxt_text("Added","Đã thêm"):Nxt_text("Already in playlist / cannot write","Đã có trong danh sách / không ghi được"),2000);}
}
static ModuleExitReason tracks(DisplayContext *display,int mode,const char *artist,const char *album,const char *search) {
    int selected=0,count=0;bool order=false,reload=true;unsigned revision=0;LibraryRow*rows=NULL;const char**labels=NULL;
    for(;;){if(reload||revision!=NxtLibrary_revision()){free(labels);free(rows);revision=NxtLibrary_revision();rows=query(mode,artist,album,search,order,&count);labels=calloc(count?count:1,sizeof(char*));if(!labels){free(labels);free(rows);return MODULE_EXIT_TO_MENU;}for(int i=0;i<count;i++)labels[i]=rows[i].label;reload=false;}
        NxtPick p=Nxt_pickLive(display,mode==1?Nxt_text("Artists","Nghệ sĩ"):mode==2?Nxt_text("Albums","Album"):mode==4?Nxt_text("Favorites","Yêu thích"):Nxt_text("Tracks","Bài hát"),labels,count,&selected,Nxt_text("A Play/Open • X Actions • Y Sort • ←→ Page • B Back","A Phát/Mở • X Thao tác • Y Sắp xếp • ←→ Trang • B Quay lại"));
        if(p.action==2||p.action==9){free(labels);free(rows);return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
        if(p.action==4){order=!order;reload=true;}
        if(p.action==5)selected=selected>9?selected-10:0;if(p.action==6)selected=selected+10<count?selected+10:count-1;
        if(p.action==3&&!count)refresh();
        if(selected>=0&&selected<count){
            if(p.action==1){ModuleExitReason r=MODULE_EXIT_TO_MENU;if(mode==1)r=tracks(display,0,rows[selected].artist,NULL,NULL);else if(mode==2)r=tracks(display,0,rows[selected].artist,rows[selected].album,NULL);else {int n=count>PLAYLIST_MAX_TRACKS?PLAYLIST_MAX_TRACKS:count;int start=selected;if(start>=n){Toast_show(Nxt_text("Queue limit reached (10000)","Hàng đợi đạt giới hạn 10000 bài"),2000);}else {PlaylistTrack *list=calloc(n,sizeof(*list));if(list){for(int i=0;i<n;i++){snprintf(list[i].path,sizeof(list[i].path),"%s",rows[i].path);snprintf(list[i].name,sizeof(list[i].name),"%s",rows[i].label);list[i].format=Player_detectFormat(rows[i].path);}PlayerModule_setResumePlaylistPath("");r=PlayerModule_runWithPlaylist(display,list,n,start);free(list);}}}if(r==MODULE_EXIT_QUIT){free(labels);free(rows);return r;}if(Nxt_takeReturnHome()){Nxt_returnHome();free(labels);free(rows);return r;}}
            else if(p.action==3&&mode!=1&&mode!=2){const char *actions[]={Nxt_text("Play next","Phát tiếp theo"),Nxt_text("Append to queue","Thêm cuối hàng đợi"),Nxt_text("Toggle favorite","Thêm / bỏ yêu thích"),Nxt_text("Add to playlist","Thêm vào danh sách phát")};int a=0;NxtPick pick=Nxt_pick(display,rows[selected].label,actions,4,&a,Nxt_text("A Select • B Back","A Chọn • B Quay lại"));if(pick.action==1){if(a==0||a==1)PlayerModule_enqueue(rows[selected].path,rows[selected].label,a==0);else if(a==2)Nxt_favorite(rows[selected].path);else add_playlist(&rows[selected],display);}}
        }
    }
}
bool Nxt_chooseFolder(DisplayContext *display) {
    char path[512];snprintf(path,sizeof(path),"%s",nxt.music_root);int selected=0;
    for(;;){DIR *dir=opendir(path);if(!dir){snprintf(path,sizeof(path),"%s",SDCARD_PATH);dir=opendir(path);}char **names=calloc(2048,sizeof(char*));int count=0;names[count++]=strdup(Nxt_text("Use this folder (all subfolders)","Chọn thư mục này (gồm thư mục con)"));if(strcmp(path,SDCARD_PATH))names[count++]=strdup("..");
        if(dir){struct dirent *ent;while((ent=readdir(dir))&&count<2048){if(ent->d_name[0]=='.')continue;char file[1024];snprintf(file,sizeof(file),"%s/%s",path,ent->d_name);struct stat st;if(lstat(file,&st)==0&&S_ISDIR(st.st_mode))names[count++]=strdup(ent->d_name);}closedir(dir);}
        int first=strcmp(path,SDCARD_PATH)?2:1;qsort(names+first,count-first,sizeof(*names),NxtSort_strings);
        NxtPick p=Nxt_pick(display,path,(const char**)names,count,&selected,Nxt_text("A Open / choose • B Cancel","A Mở / chọn • B Hủy"));bool done=p.action==1&&selected==0;if(done){snprintf(nxt.music_root,sizeof(nxt.music_root),"%s",path);PlayerModule_invalidateBrowser();Nxt_save();Nxt_libraryQuit();refresh();}
        if(p.action==1&&selected>0&&selected<count){char next[512];if(!strcmp(names[selected],"..")){snprintf(next,sizeof(next),"%s",path);char *slash=strrchr(next,'/');if(slash)*slash=0;}else snprintf(next,sizeof(next),"%s/%s",path,names[selected]);snprintf(path,sizeof(path),"%s",next);selected=0;}
        for(int i=0;i<count;i++)free(names[i]);free(names);if(done)return true;if(p.action==2||p.action==9)return false;
    }
}

ModuleExitReason Nxt_favorites(DisplayContext *d){init();NxtLibrary_start();return tracks(d,4,NULL,NULL,NULL);}
void Nxt_libraryAddTracks(DisplayContext *d,const char *path){init();NxtLibrary_start();int selected=0,count=0;unsigned revision=~NxtLibrary_revision();LibraryRow*rows=NULL;const char**labels=NULL;for(;;){if(!labels||revision!=NxtLibrary_revision()){free(labels);free(rows);revision=NxtLibrary_revision();rows=query(0,NULL,NULL,NULL,false,&count);labels=calloc(count?count:1,sizeof(char*));if(!labels){free(rows);return;}for(int i=0;i<count;i++)labels[i]=rows[i].label;}NxtPick p=Nxt_pickLive(d,Nxt_text("Add tracks · A adds selected track","Thêm bài · A thêm bài đang chọn"),labels,count,&selected,Nxt_text("A Add • B Done","A Thêm • B Xong"));if(p.action==1&&selected>=0&&selected<count){char name[256];snprintf(name,sizeof(name),"%s",rows[selected].label);char *newline=strchr(name,'\n');if(newline)*newline=0;int status=M3U_addTrack(path,rows[selected].path,name);Toast_show(status==0?Nxt_text("Track added","Đã thêm bài"):Nxt_text("Already added / cannot write","Đã có bài / không ghi được"),1000);}if(p.action==2||p.action==9){free(labels);free(rows);return;}}}
static ModuleExitReason play_rows(DisplayContext *d,LibraryRow *rows,int count,int selected){int n=count>PLAYLIST_MAX_TRACKS?PLAYLIST_MAX_TRACKS:count;if(selected>=n){Toast_show(Nxt_text("Queue limit: 10000 tracks","Hàng đợi tối đa 10000 bài"),2000);return MODULE_EXIT_TO_MENU;}PlaylistTrack *list=calloc(n,sizeof(*list));if(!list)return MODULE_EXIT_TO_MENU;for(int i=0;i<n;i++){snprintf(list[i].path,sizeof(list[i].path),"%s",rows[i].path);snprintf(list[i].name,sizeof(list[i].name),"%s",rows[i].label);char *newline=strchr(list[i].name,'\n');if(newline)*newline=0;list[i].format=Player_detectFormat(list[i].path);}PlayerModule_setResumePlaylistPath("");ModuleExitReason r=PlayerModule_runWithPlaylist(d,list,n,selected);free(list);return r;}
static void row_actions(DisplayContext *d,LibraryRow *row){const char *items[]={Nxt_text("Play next","Phát tiếp theo"),Nxt_text("Append to queue","Thêm cuối hàng đợi"),Nxt_text("Favorite / unfavorite","Thêm / bỏ yêu thích"),Nxt_text("Add to playlist","Thêm vào danh sách phát")};int selected=0;NxtPick p=Nxt_pick(d,row->label,items,4,&selected,Nxt_text("A Select • B Back","A Chọn • B Quay lại"));if(p.action==1){char title[256];NxtText_copy(title,row->label,sizeof(title));char *line=strchr(title,'\n');if(line)*line=0;if(selected<2)PlayerModule_enqueue(row->path,title,selected==0);else if(selected==2)Nxt_favorite(row->path);else add_playlist(row,d);}}
ModuleExitReason Nxt_library(DisplayContext *d){
 init();NxtLibrary_start();int tab=0,selected[4]={0};bool order=false;char search[256]="",folder[512];snprintf(folder,sizeof(folder),"%s",nxt.music_root);
 const char *tabs[]={Nxt_text("All tracks","Tất cả bài"),Nxt_text("Folders","Thư mục"),Nxt_text("Artists","Nghệ sĩ"),Nxt_text("Albums","Album")};
 int count=0,ndirs=0;LibraryRow *rows=NULL;char (*dirs)[512]=NULL;const char **labels=NULL;unsigned revision=0;bool reload=true;
 for(;;){char title[600];
  if(reload||revision!=NxtLibrary_revision()){
   free(labels);free(rows);free(dirs);labels=NULL;rows=NULL;dirs=NULL;count=ndirs=0;revision=NxtLibrary_revision();
  if(tab==1){
   BrowserContext browser={0};Browser_loadDirectory(&browser,folder,nxt.music_root);
   int capacity=browser.entry_count>4096?4096:browser.entry_count;
   rows=calloc(capacity?capacity:1,sizeof(*rows));dirs=calloc(capacity?capacity:1,512);
   if(!rows||!dirs){Browser_freeEntries(&browser);free(labels);free(rows);free(dirs);return MODULE_EXIT_TO_MENU;}
   for(int i=0;i<browser.entry_count&&count<capacity;i++){
    FileEntry *e=&browser.entries[i];if(e->is_play_all)continue;
    LibraryRow *r=&rows[count];
    if(e->is_dir){snprintf(r->label,sizeof(r->label),"%s\n%s",e->name,Nxt_text("Folder","Thư mục"));snprintf(dirs[count],512,"%s",e->path);ndirs++;}
    else{snprintf(r->label,sizeof(r->label),"%s",e->name);snprintf(r->path,sizeof(r->path),"%s",e->path);}
    count++;
   }
   Browser_freeEntries(&browser);snprintf(title,sizeof(title),"%s · %s",Nxt_text("Library","Thư viện"),folder);
  }
  else rows=query(tab==2?1:tab==3?2:0,NULL,NULL,tab==0?search:NULL,order,&count);
  labels=calloc(count?count:1,sizeof(char*));if(!labels){free(labels);free(rows);free(dirs);return MODULE_EXIT_TO_MENU;}for(int i=0;i<count;i++)labels[i]=rows[i].label;reload=false;
  }
  if(tab==1)snprintf(title,sizeof(title),"%s · %s",Nxt_text("Library","Thư viện"),folder);
  else{pthread_mutex_lock(&scan_lock);bool active=scanning;pthread_mutex_unlock(&scan_lock);snprintf(title,sizeof(title),"%s · %d%s%s",Nxt_text("Library","Thư viện"),count,active?Nxt_text(" · checking changes"," · kiểm tra thay đổi"):"",search[0]?" · Search":"");}
  NxtPick p=Nxt_pickTabs(d,title,labels,count,&selected[tab],Nxt_text("A Play/Open • X Search • Y Sort • L1/R1 Tabs • START Options • B Back","A Phát/Mở • X Tìm • Y Sắp xếp • L1/R1 Mục • START Tùy chọn • B Quay lại"),tabs,4,tab);ModuleExitReason result=MODULE_EXIT_TO_MENU;int sel=selected[tab];
  if(p.action==2){if(tab==1&&strcmp(folder,nxt.music_root)){char *slash=strrchr(folder,'/');if(slash)*slash=0;selected[1]=0;reload=true;}else{free(labels);free(rows);free(dirs);return MODULE_EXIT_TO_MENU;}}
  if(p.action==9){free(labels);free(rows);free(dirs);return MODULE_EXIT_QUIT;}
  if(p.action==11){tab=(tab+3)%4;reload=true;}if(p.action==12){tab=(tab+1)%4;reload=true;}
  if(p.action==3){char *text=Keyboard_open(Nxt_text("Search titles / artists / albums","Tìm tên bài / nghệ sĩ / album"),255);if(text){snprintf(search,sizeof(search),"%s",text);free(text);tab=0;selected[0]=0;reload=true;}}
  if(p.action==4){order=!order;reload=true;}
  if(p.action==5)selected[tab]=selected[tab]>9?selected[tab]-10:0;if(p.action==6)selected[tab]=selected[tab]+10<count?selected[tab]+10:count-1;
  if(p.action==10){const char *options[]={Nxt_text("Check for new music","Kiểm tra nhạc mới"),Nxt_text("Clear search","Xóa tìm kiếm"),Nxt_text("Choose music folder","Chọn thư mục nhạc"),Nxt_text("Selected track actions","Thao tác bài đang chọn"),Nxt_text("Rebuild all metadata\nUse if tags or the saved index look incorrect","Quét lại toàn bộ thông tin\nDùng khi thông tin bài hoặc chỉ mục chưa đúng")};int opt=0;NxtPick o=Nxt_pick(d,Nxt_text("Library options","Tùy chọn thư viện"),options,5,&opt,Nxt_text("A Select • B Back","A Chọn • B Quay lại"));if(o.action==1){if(opt==0)refresh();if(opt==4)NxtLibrary_request(true);if(opt==1){search[0]=0;reload=true;}if(opt==2){Nxt_chooseFolder(d);snprintf(folder,sizeof(folder),"%s",nxt.music_root);reload=true;}if(opt==3&&count&&sel<count&&rows[sel].path[0])row_actions(d,&rows[sel]);}}
  if(p.action==1&&sel>=0&&sel<count){if(tab==1&&sel<ndirs){snprintf(folder,sizeof(folder),"%s",dirs[sel]);selected[1]=0;reload=true;}else if(tab==2)result=tracks(d,0,rows[sel].artist,NULL,NULL);else if(tab==3)result=tracks(d,0,rows[sel].artist,rows[sel].album,NULL);else if(tab==1)result=play_rows(d,rows+ndirs,count-ndirs,sel-ndirs);else result=play_rows(d,rows,count,sel);}
  if(result==MODULE_EXIT_QUIT||Nxt_takeReturnHome()){free(labels);free(rows);free(dirs);return result;}
 }
}
