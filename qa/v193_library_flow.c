#include "nxt_library.c"
#include <assert.h>
#include <stdarg.h>
#include <time.h>
NxtConfig nxt;
static char data_root[1024];static Uint32 fake_now=100;static int hidden,display_off,transfer_busy,probes,index_jobs,browser_calls,track_queries,frames,frame_limit=30,tab_mode,mutate_view;
static const char **prior_items;
Uint32 SDL_GetTicks(void){return fake_now;}
bool NxtSession_isHidden(void){return hidden;}
int PWR_getDisplayOff(void){return display_off;}
bool NxtTransfer_isBusy(void){return transfer_busy;}
int userdata_snpath(const char*rel,char*out,size_t n){return snprintf(out,n,"%s/%s",data_root,rel);}
bool userdata_mkdir(const char*p){return true;}
char*NxtText_searchKey(const char*s){return strdup(s?s:"");}
int NxtSort_compare(const char*a,const char*b){return strcmp(a,b);}
int NxtSort_strings(const void*a,const void*b){return strcmp(*(char**)a,*(char**)b);}
void NxtText_copy(char*out,const char*s,size_t n){snprintf(out,n,"%s",s?s:"");}
const char*Nxt_text(const char*a,const char*b){return a;}
int NxtPersonal_id(void){return 7;}
AudioFormat Player_detectFormat(const char*p){const char*ext=strrchr(p,'.');return ext&&(!strcmp(ext,".flac")||!strcmp(ext,".dff"))?AUDIO_FORMAT_FLAC:AUDIO_FORMAT_UNKNOWN;}
bool NxtDecoder_probe(const char*p,TrackInfo*t){__atomic_add_fetch(&probes,1,__ATOMIC_RELAXED);memset(t,0,sizeof(*t));snprintf(t->title,sizeof(t->title),"%s",strrchr(p,'/')+1);snprintf(t->artist,sizeof(t->artist),"Artist");snprintf(t->album,sizeof(t->album),"Album");return true;}
void NxtLog_step(const char*a,const char*b,bool ok,const char*fmt,...){if(!strcmp(a,"library")&&!strcmp(b,"index"))index_jobs++;}
ToastToken Toast_show(const char*s,uint32_t n){return 0;}
bool Nxt_save(void){return true;}void PlayerModule_invalidateBrowser(void){}
void PlayerModule_setResumePlaylistPath(const char*p){}
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext*d,PlaylistTrack*p,int n,int start){return MODULE_EXIT_TO_MENU;}
bool PlayerModule_enqueue(const char*a,const char*b,bool next){return true;}
bool Nxt_takeReturnHome(void){return false;}void Nxt_returnHome(void){}
char*Keyboard_open(const char*s,size_t n){return NULL;}
int M3U_listPlaylists(PlaylistInfo*p,int n){return 0;}
int M3U_addTrack(const char*a,const char*b,const char*c){return 0;}
void Browser_freeEntries(BrowserContext*b){free(b->entries);b->entries=NULL;}
void Browser_loadDirectory(BrowserContext*b,const char*path,const char*root){browser_calls++;b->entries=calloc(1,sizeof(*b->entries));b->entry_count=1;snprintf(b->entries[0].name,sizeof(b->entries[0].name),"one.flac");snprintf(b->entries[0].path,sizeof(b->entries[0].path),"%s/one.flac",path);}
NxtPick Nxt_pick(DisplayContext*d,const char*t,const char**rows,int n,int*sel,const char*hint){return (NxtPick){2,*sel};}
NxtPick Nxt_pickLive(DisplayContext*d,const char*t,const char**rows,int n,int*sel,const char*hint){if(++frames>frame_limit)return (NxtPick){2,*sel};return (NxtPick){7,*sel};}
NxtPick Nxt_pickTabs(DisplayContext*d,const char*t,const char**rows,int n,int*sel,const char*hint,const char**tabs,int ntabs,int tab){
 assert(n>=0);if(frames==0)prior_items=rows;else if(!tab_mode&&!mutate_view)assert(prior_items==rows);
 frames++;fake_now+=1000;NxtLibrary_tick();
 if(mutate_view&&frames==3){index_changed(NULL);prior_items=NULL;}
 if(tab_mode&&frames==1)return (NxtPick){12,*sel};
 if(frames>frame_limit)return (NxtPick){2,*sel};return (NxtPick){7,*sel};
}
static void trace(void*u,const char*sql){if(strstr(sql,"FROM tracks"))track_queries++;}
static void finish(void){for(int i=0;i<5000;i++){fake_now++;NxtLibrary_tick();pthread_mutex_lock(&scan_lock);bool active=scanning;pthread_mutex_unlock(&scan_lock);if(!active&&!scanner_joinable&&!index_pending)return;usleep(1000);}assert(!"index did not settle");}
static int checkpoint_returned;static bool checkpoint_value;
static void*check_pause(void*u){checkpoint_value=scan_checkpoint(NULL);__atomic_store_n(&checkpoint_returned,1,__ATOMIC_RELEASE);return NULL;}
static int qcount(void){int n=0;LibraryRow*r=query(0,NULL,NULL,NULL,false,&n);free(r);return n;}
int main(int argc,char**argv){assert(argc==4);snprintf(data_root,sizeof(data_root),"%s",argv[1]);snprintf(nxt.music_root,sizeof(nxt.music_root),"%s",argv[2]);nxt.recursive=1;
 assert(init());NxtLibrary_start();NxtLibrary_tick();assert(!scanning&&index_pending);int cached=qcount();if(atoi(argv[3]))assert(cached==2);else assert(cached==0);
 fake_now+=2000;finish();assert(qcount()==2&&index_jobs==1);if(atoi(argv[3]))assert(probes==0&&index_result.writes==0);else assert(probes==2);
 sqlite3_trace(library,trace,NULL);int before=track_queries;frames=0;assert(Nxt_library(NULL)==MODULE_EXIT_TO_MENU);assert(track_queries-before==1&&index_jobs==1);
 frames=0;before=track_queries;assert(Nxt_library(NULL)==MODULE_EXIT_TO_MENU);assert(track_queries-before==1&&index_jobs==1);
 tab_mode=1;frames=0;browser_calls=0;assert(Nxt_library(NULL)==MODULE_EXIT_TO_MENU);assert(browser_calls==1&&index_jobs==1);tab_mode=0;
 mutate_view=1;frames=0;before=track_queries;assert(Nxt_library(NULL)==MODULE_EXIT_TO_MENU);assert(track_queries-before==2);mutate_view=0;
 int old_jobs=index_jobs;hidden=1;NxtLibrary_request(false);fake_now+=2000;NxtLibrary_tick();assert(!scanning&&index_pending);hidden=0;finish();assert(index_jobs==old_jobs+1&&index_result.writes==0);
 old_jobs=index_jobs;transfer_busy=1;NxtLibrary_request(false);fake_now+=2000;NxtLibrary_tick();assert(!scanning&&index_pending);transfer_busy=0;finish();assert(index_jobs==old_jobs+1);
 old_jobs=index_jobs;for(int i=0;i<100;i++)NxtLibrary_notify();NxtLibrary_tick();fake_now+=999;NxtLibrary_tick();assert(!scanning);fake_now+=2;finish();assert(index_jobs==old_jobs+1);
 Nxt_favorite("/test/favorite.flac");assert(Nxt_isFavorite("/test/favorite.flac"));Nxt_favorite("/test/favorite.flac");assert(!Nxt_isFavorite("/test/favorite.flac"));
 /* Changing recursive scope hides nested tracks immediately but retains their cache. */
 nxt.recursive=0;assert(qcount()==1);fake_now+=2000;NxtLibrary_tick();fake_now+=2000;finish();assert(qcount()==1);nxt.recursive=1;assert(qcount()==2);NxtLibrary_tick();fake_now+=2000;finish();assert(index_result.decoded==0);
 old_jobs=index_jobs;Nxt_libraryQuit();assert(init());NxtLibrary_start();fake_now+=5000;NxtLibrary_tick();assert(!scanning&&!index_pending&&index_jobs==old_jobs);
 Nxt_libraryQuit();pthread_t pth;pthread_mutex_lock(&scan_lock);cancel_scan=false;index_paused=true;pthread_mutex_unlock(&scan_lock);checkpoint_returned=0;assert(!pthread_create(&pth,NULL,check_pause,NULL));usleep(20000);assert(!__atomic_load_n(&checkpoint_returned,__ATOMIC_ACQUIRE));pthread_mutex_lock(&scan_lock);index_paused=false;pthread_cond_broadcast(&scan_wake);pthread_mutex_unlock(&scan_lock);pthread_join(pth,NULL);assert(checkpoint_value);
 pthread_mutex_lock(&scan_lock);index_paused=true;pthread_mutex_unlock(&scan_lock);checkpoint_returned=0;assert(!pthread_create(&pth,NULL,check_pause,NULL));usleep(20000);Nxt_libraryQuit();pthread_join(pth,NULL);assert(!checkpoint_value);
 puts("PASS actual library UI/scheduler: saved rows before worker, one startup check, no re-entry scans, stable live/folder views, generation refresh, hidden pause, upload debounce, favorites and recursive scope");return 0;}
