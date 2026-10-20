#include "nxt_library_index.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
static int calls,stop_at,mutate,root_move;static char *root;
static bool audio(const char*p,void*u){const char*e=strrchr(p,'.');return e&&(!strcasecmp(e,".mp3")||!strcasecmp(e,".flac")||!strcasecmp(e,".dff")||!strcasecmp(e,".dsf"));}
static bool probe(const char*p,NxtIndexTrack*t,void*u){
 FILE*f=fopen(p,"rb");if(!f)return false;char text[256]={0};bool read=fgets(text,sizeof(text),f)!=NULL;fclose(f);if(!read)return false;if(!strncmp(text,"BAD",3))return false;
 const char*name=strrchr(p,'/');snprintf(t->title,sizeof(t->title),"%s",text);snprintf(t->artist,sizeof(t->artist),"Artist");snprintf(t->album,sizeof(t->album),"Album");t->track=atoi(name?name+1:p);
 if(mutate){mutate=0;f=fopen(p,"ab");assert(f);fputs("copy-still-growing",f);fclose(f);}
 if(root_move){root_move=0;char moved[1024];snprintf(moved,sizeof(moved),"%s-offline",root);assert(!rename(root,moved));}
 return true;
}
static bool check(void*u){return !stop_at||++calls<stop_at;}
int main(int n,char**v){assert(n==7);sqlite3*d=NULL;assert(sqlite3_open(v[1],&d)==SQLITE_OK);assert(NxtIndex_schema(d));sqlite3_close(d);root=v[2];stop_at=atoi(v[4]);mutate=atoi(v[5]);root_move=atoi(v[6]);NxtIndexHooks h={.audio=audio,.probe=probe,.checkpoint=check};NxtIndexResult r;
 bool ok=NxtIndex_run(v[1],root,true,atoi(v[3]),&h,&r);struct rusage usage;getrusage(RUSAGE_SELF,&usage);
 printf("{\"ok\":%d,\"complete\":%d,\"checked\":%d,\"decoded\":%d,\"added\":%d,\"updated\":%d,\"removed\":%d,\"writes\":%d,\"commits\":%d,\"deferred\":%d,\"cancelled\":%d,\"unavailable\":%d,\"rss_kib\":%ld}\n",ok,r.complete,r.checked,r.decoded,r.added,r.updated,r.removed,r.writes,r.commits,r.deferred,r.cancelled,r.unavailable,usage.ru_maxrss);return 0;}
