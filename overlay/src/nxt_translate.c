/* SPDX-License-Identifier: GPL-3.0-only
 * One low-priority worker, bounded memory, parked when idle/hidden, generation
 * cancellation and cached text. No work in the audio callback or paint path.
 * MyMemory public GET, Google Cloud Basic v2 and Groq chat-completions APIs.
 */
#define _GNU_SOURCE
#include "nxt_keyfile.h"
#include "nxt_translate.h"
#include "nxt_karaoke.h"
#include "lyrics.h"
#include "nxt_playback.h"
#include "nxt_personal.h"
#include "nxt_locale.h"
#include "nxt_text.h"
#include "file_utils.h"
#include "keyboard.h"
#include "toast.h"
#include "api.h"
#include "parson/parson.h"
#include "mbedtls/sha256.h"
#include <utf8proc.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <time.h>
#include <spawn.h>
#include <sched.h>
extern char **environ;
#define INPUT_LIMIT (128*1024)
#define OUTPUT_LIMIT (256*1024)
#define CACHE_LIMIT (512*1024)
#define REPLY_LIMIT (32*1024)
#define LINE_LIMIT 4096
#define CHUNK_LIMIT 480 /* documented MyMemory maximum is 500 UTF-8 bytes */
#define GROUP_MAX 24
#define GROQ_BYTES 2400
#define CACHE_BYTES (256LL*1024*1024) /* SD storage, not an allocation. */
#define CACHE_PRUNE_BATCH 256 /* Bounded metadata heap, independent of song count. */
enum {ST_OFF,ST_WAIT,ST_BUSY,ST_READY,ST_NETWORK,ST_QUOTA,ST_KEY,ST_LONG,ST_SAVE,ST_SERVICE};
enum {PROVIDER_MYMEMORY,PROVIDER_GOOGLE,PROVIDER_GROQ,PROVIDER_COUNT};
static const char *service_names[]={"MyMemory","Google Cloud","Groq"};
static const char *groq_models[]={"openai/gpt-oss-20b","openai/gpt-oss-120b","qwen/qwen3.8-27b"};
static const char *codes[]={"en","vi","fr","ru","ko","ja","th","zh-CN","zh-TW","de","es","pt","it","id"};
typedef struct {
 unsigned generation;int count,source,target,provider;bool source_auto;
 char key[256],model[80];int retry_after;
 char cache_dir[1400],hash[65],curl_path[1024],diagnostic[160];char **env;
 char *input[LYRICS_MAX_LINES],*output[LYRICS_MAX_LINES];
 size_t output_bytes,cache_saved_bytes;off_t cache_room;int state,done,groq_group_limit;bool batch_disabled,pruned;
 time_t due[LYRICS_MAX_LINES];unsigned char failures[LYRICS_MAX_LINES];
} Job;
static pthread_t thread;static bool started,service_loaded;
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wake=PTHREAD_COND_INITIALIZER;
static atomic_uint generation=1;
static atomic_bool quitting=false,paused=false;
static atomic_int focus=0;
static atomic_long cooldown[PROVIDER_COUNT];
static atomic_int cooldown_state[PROVIDER_COUNT];
static atomic_bool pending_dirty=false;
static Job *queued;
static char *pending[LYRICS_MAX_LINES],*shown[LYRICS_MAX_LINES];
static int pending_state=ST_OFF,pending_done,ui_state=ST_OFF,ui_done,ui_total,provider;
static unsigned pending_gen,view_revision=1,seen_lyrics;
static int seen_mode=-1,seen_language=-1,seen_source=-1,seen_listener=-1,service_revision,seen_service=-1;
static char service_key[256],groq_key[256];static int groq_model;
static char status_text[400],pending_diagnostic[160],ui_diagnostic[160];

static bool valid_text(const char *s,size_t maximum){
 if(!s||strlen(s)>maximum)return false;
 const unsigned char *p=(const unsigned char *)s;
 while(*p){utf8proc_int32_t cp;utf8proc_ssize_t n=utf8proc_iterate(p,-1,&cp);if(n<=0||cp==0)return false;p+=n;}
 return true;
}
static void free_job(Job *j){if(!j)return;for(int i=0;i<j->count;i++){free(j->input[i]);free(j->output[i]);}if(j->env){for(int i=0;j->env[i];i++)free(j->env[i]);free(j->env);}free(j);}
/* Snapshot on the UI thread. Playback may change ALSA/provider environment
 * while a worker is spawning: never give posix_spawn a mutable environ array. */
static bool snapshot_environment(Job *j){
 int count=0;size_t bytes=0;while(environ[count]){bytes+=strlen(environ[count])+1;if(bytes>256*1024||count>=2048)return false;count++;}
 j->env=calloc(count+1,sizeof(char*));if(!j->env)return false;
 for(int i=0;i<count;i++)if(!(j->env[i]=strdup(environ[i])))return false;
 const char *path=getenv("PATH");char *copy=strdup(path?path:"/usr/bin:/bin");if(!copy)return false;
 char *part=copy;while(part){char *next=strchr(part,':');if(next)*next++=0;char candidate[1400];snprintf(candidate,sizeof(candidate),"%s/curl",*part?part:".");if(!access(candidate,X_OK)){char *resolved=realpath(candidate,NULL);if(resolved){snprintf(j->curl_path,sizeof(j->curl_path),"%s",resolved);free(resolved);break;}}part=next;}
 free(copy);return true;
}
static const char *language_code(int n){return codes[n>=0&&n<14?n:0];}
static bool key_valid(const char *s){size_t n=s?strlen(s):0;if(!n||n>=sizeof(service_key))return false;for(size_t i=0;i<n;i++)if(!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-",s[i]))return false;return true;}
static void service_path(char *p,size_t n){userdata_snpath("translation-service.json",p,n);}
static void load_service(void){
 if(service_loaded)return;service_loaded=true;char p[1400];service_path(p,sizeof(p));
 JSON_Value *v=json_parse_file(p);JSON_Object *o=json_value_get_object(v);
 double value=json_object_get_number(o,"provider");provider=value>=0&&value<PROVIDER_COUNT&&value==(int)value?(int)value:0;
 const char *k=json_object_get_string(o,"google_key");if(k&&key_valid(k))snprintf(service_key,sizeof(service_key),"%s",k);
 k=json_object_get_string(o,"groq_key");if(k&&key_valid(k))snprintf(groq_key,sizeof(groq_key),"%s",k);
 const char *model=json_object_get_string(o,"groq_model");groq_model=0;for(int i=0;i<3;i++)if(model&&!strcmp(model,groq_models[i]))groq_model=i;
 json_value_free(v);
}
static bool save_service(void){
 char p[1400],tmp[1440];service_path(p,sizeof(p));snprintf(tmp,sizeof(tmp),"%s.nxt-writing",p);
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);
 json_object_set_number(o,"provider",provider);json_object_set_string(o,"google_key",service_key);
 json_object_set_string(o,"groq_key",groq_key);json_object_set_string(o,"groq_model",groq_models[groq_model]);
 char *s=json_serialize_to_string(v);json_value_free(v);if(!s)return false;
 int fd=open(tmp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);bool ok=fd>=0;
 if(ok){fchmod(fd,0600);size_t n=strlen(s),pos=0;while(pos<n){ssize_t w=write(fd,s+pos,n-pos);if(w<0&&errno==EINTR)continue;if(w<=0){ok=false;break;}pos+=w;}if(close(fd))ok=false;}
 if(ok)ok=rename(tmp,p)==0;if(!ok)unlink(tmp);json_free_serialized_string(s);return ok;
}
static bool cancelled(Job *j){return quitting||j->generation!=atomic_load(&generation);}
static bool rest(Job *j,int ms){
 struct timespec t;clock_gettime(CLOCK_REALTIME,&t);t.tv_nsec+=(long)(ms%1000)*1000000;t.tv_sec+=ms/1000+t.tv_nsec/1000000000;t.tv_nsec%=1000000000;
 pthread_mutex_lock(&lock);if(!cancelled(j))pthread_cond_timedwait(&wake,&lock,&t);pthread_mutex_unlock(&lock);return !cancelled(j);
}
static bool wait_visible(Job *j){
 pthread_mutex_lock(&lock);while(paused&&!cancelled(j))pthread_cond_wait(&wake,&lock);pthread_mutex_unlock(&lock);return !cancelled(j);
}
static void publish(Job *j,int i){
 char *copy=i>=0&&j->output[i]?strdup(j->output[i]):NULL;
 pthread_mutex_lock(&lock);
 if(!cancelled(j)){if(i>=0){free(pending[i]);pending[i]=copy;copy=NULL;}pending_state=j->state;pending_done=j->done;pending_gen=j->generation;snprintf(pending_diagnostic,sizeof(pending_diagnostic),"%s",j->diagnostic);atomic_store(&pending_dirty,true);}
 pthread_mutex_unlock(&lock);free(copy);
}
static int guess(Job *j){
 bool han=false,ru=false,th=false,vi=false,ko=false,ja=false;
 for(int i=0;i<j->count;i++)for(const unsigned char *p=(unsigned char *)j->input[i];*p;){
  utf8proc_int32_t c;int n=utf8proc_iterate(p,-1,&c);if(n<=0)break;p+=n;
  if((c>=0x3040&&c<=0x30ff)||(c>=0xff66&&c<=0xff9d))ja=true;
  if((c>=0xac00&&c<=0xd7af)||(c>=0x1100&&c<=0x11ff))ko=true;
  if(c>=0x4e00&&c<=0x9fff)han=true;if(c>=0x0400&&c<=0x052f)ru=true;if(c>=0x0e00&&c<=0x0e7f)th=true;
  if((c>=0x1ea0&&c<=0x1ef9)||c==0x0102||c==0x0103||c==0x0110||c==0x0111||c==0x01a0||c==0x01a1||c==0x01af||c==0x01b0)vi=true;
 }
 return ko?4:ja?5:han?7:th?6:ru?3:vi?1:0;
}
static void digest(Job *j){
 mbedtls_sha256_context c;unsigned char out[32];mbedtls_sha256_init(&c);mbedtls_sha256_starts(&c,0);
 char header[80];int n=snprintf(header,sizeof(header),"NXT-LYRIC-TRANSLATION-1/%d/%s/%s/%d",j->provider,j->provider&&j->source_auto?"auto":language_code(j->source),language_code(j->target),j->count);
 mbedtls_sha256_update(&c,(unsigned char *)header,n);
 if(j->provider==PROVIDER_GROQ)mbedtls_sha256_update(&c,(unsigned char *)j->model,strlen(j->model)+1);
 for(int i=0;i<j->count;i++)mbedtls_sha256_update(&c,(unsigned char *)j->input[i],strlen(j->input[i])+1);
 mbedtls_sha256_finish(&c,out);mbedtls_sha256_free(&c);
 for(int i=0;i<32;i++)sprintf(j->hash+i*2,"%02x",out[i]);
}
static void cache_path(Job *j,char *p,size_t n){snprintf(p,n,"%s/%s.json",j->cache_dir,j->hash);}
static void read_cache(Job *j){
 char p[1500];cache_path(j,p,sizeof(p));struct stat st;if(stat(p,&st)||st.st_size<=0||st.st_size>CACHE_LIMIT)return;
 JSON_Value *v=json_parse_file(p);JSON_Object *o=json_value_get_object(v);
 const char *h=json_object_get_string(o,"hash");JSON_Array *a=json_object_get_array(o,"lines");
 if(h&&!strcmp(h,j->hash)&&json_array_get_count(a)==(size_t)j->count){
  for(int i=0;i<j->count;i++){const char *s=json_array_get_string(a,i);if(s&&*s&&valid_text(s,LINE_LIMIT-1)&&j->output_bytes+strlen(s)+1<=OUTPUT_LIMIT){j->output[i]=strdup(s);if(j->output[i]){j->output_bytes+=strlen(s)+1;j->done++;publish(j,i);}}}
 }
 json_value_free(v);
}
typedef struct {char name[70];time_t age;off_t bytes;} CacheEntry;
static int older(const void *a,const void *b){const CacheEntry *x=a,*y=b;return x->age<y->age?-1:x->age>y->age?1:strcmp(x->name,y->name);}
/* Max-heap: retain the oldest bounded batch while counting the entire folder.
 * No fixed 512-file eviction and no unbounded filename list in RAM. */
static void cache_oldest(CacheEntry *heap,int *count,const CacheEntry *item){
 int at;if(*count<CACHE_PRUNE_BATCH){at=(*count)++;heap[at]=*item;while(at>0){int parent=(at-1)/2;if(older(&heap[parent],&heap[at])>=0)break;CacheEntry t=heap[parent];heap[parent]=heap[at];heap[at]=t;at=parent;}return;}
 if(older(item,&heap[0])>=0)return;heap[0]=*item;at=0;
 for(;;){int child=at*2+1;if(child>=*count)break;if(child+1<*count&&older(&heap[child+1],&heap[child])>0)child++;if(older(&heap[at],&heap[child])>=0)break;CacheEntry t=heap[at];heap[at]=heap[child];heap[child]=t;at=child;}
}
static bool prune(Job *j){
 CacheEntry *entries=calloc(CACHE_PRUNE_BATCH,sizeof(*entries));if(!entries)return false;char keep[70],path[1500];snprintf(keep,sizeof(keep),"%s.json",j->hash);bool ok=false;
 while(!cancelled(j)){
  DIR *dir=opendir(j->cache_dir);if(!dir)break;int count=0;off_t bytes=0;unsigned scanned=0;struct dirent *d;bool interrupted=false;
  while((d=readdir(dir))){
   if(cancelled(j)){interrupted=true;break;}
   /* Run only in the existing low-priority worker; yield SD/CPU regularly. */
   if(++scanned%64==0&&!rest(j,1)){interrupted=true;break;}
   size_t n=strlen(d->d_name);if(n!=69||strcmp(d->d_name+64,".json"))continue;bool good=true;for(int i=0;i<64;i++)if(!strchr("0123456789abcdef",d->d_name[i]))good=false;if(!good)continue;
   snprintf(path,sizeof(path),"%s/%s",j->cache_dir,d->d_name);struct stat st;if(lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size<0)continue;
   if(st.st_size>INT64_MAX-bytes){interrupted=true;break;}bytes+=st.st_size;
   if(strcmp(d->d_name,keep)){CacheEntry e={.age=st.st_mtime,.bytes=st.st_size};memcpy(e.name,d->d_name,70);cache_oldest(entries,&count,&e);}
  }
  closedir(dir);if(interrupted)break;if(bytes<=CACHE_BYTES){j->cache_room=CACHE_BYTES-bytes;ok=true;break;}
  qsort(entries,count,sizeof(*entries),older);int removed=0;
  for(int i=0;i<count&&bytes>CACHE_BYTES&&!cancelled(j);i++){snprintf(path,sizeof(path),"%s/%s",j->cache_dir,entries[i].name);if(!unlink(path)){bytes-=entries[i].bytes;removed++;}if(i%32==31&&!rest(j,1))break;}
  if(cancelled(j))break;if(bytes<=CACHE_BYTES){j->cache_room=CACHE_BYTES-bytes;ok=true;break;}if(!removed)break;
 }
 free(entries);return ok;
}
static bool save_cache(Job *j){
 if(!j->done)return true;if(mkdir(j->cache_dir,0700)&&errno!=EEXIST)return false;
 char p[1500],tmp[1530];cache_path(j,p,sizeof(p));snprintf(tmp,sizeof(tmp),"%s.nxt-writing",p);
 JSON_Value *v=json_value_init_object(),*a=json_value_init_array();JSON_Object *o=json_value_get_object(v);JSON_Array *lines=json_value_get_array(a);
 json_object_set_string(o,"hash",j->hash);json_object_set_number(o,"version",1);
 for(int i=0;i<j->count;i++)json_array_append_string(lines,j->output[i]?j->output[i]:"");
 json_object_set_value(o,"lines",a);char *s=json_serialize_to_string(v);json_value_free(v);if(!s)return false;
 size_t cache_size=strlen(s);bool ok=cache_size<=CACHE_LIMIT;int fd=ok?open(tmp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600):-1;ok=fd>=0;
 if(ok){fchmod(fd,0600);size_t len=strlen(s),at=0;while(at<len){ssize_t n=write(fd,s+at,len-at);if(n<0&&errno==EINTR)continue;if(n<=0){ok=false;break;}at+=n;}if(close(fd))ok=false;}
 json_free_serialized_string(s);if(ok)ok=rename(tmp,p)==0;if(!ok)unlink(tmp);
 else{size_t previous=j->cache_saved_bytes;
  if(!j->pruned||(cache_size>previous&&(off_t)(cache_size-previous)>j->cache_room))j->pruned=prune(j);
  else j->cache_room+=(off_t)previous-(off_t)cache_size;
  j->cache_saved_bytes=cache_size;
 }return ok;
}
/* UTF-8 split, preferring whitespace. Every request stays below the public
 * API byte limit; no split inside a codepoint. */
static size_t chunk_size(const char *s){
 size_t n=strlen(s);if(n<=CHUNK_LIMIT)return n;n=CHUNK_LIMIT;while(n&&((unsigned char)s[n]&0xc0)==0x80)n--;
 size_t word=n;while(word>n/2&&!strchr(" \t",s[word]))word--;return word>n/2?word:n;
}
static char *url_encode(const char *s,size_t n){
 char *out=malloc(n*3+1);if(!out)return NULL;char *p=out;const char *hex="0123456789ABCDEF";
 for(size_t i=0;i<n;i++){unsigned char c=s[i];if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||strchr("-_.~",c))*p++=c;else{*p++='%';*p++=hex[c>>4];*p++=hex[c&15];}}*p=0;return out;
}
static char *clean(const char *input);
#include "nxt_groq.inc"

/* curl config travels on stdin: no lyric text or API keys in process argv/logs.
 * stdout is a bounded pipe, so chunked/error replies cannot grow an SD file. */
static int transport_batch(Job *j,const char *text,const char *const *items,int count,char **reply){
 if(!*j->curl_path){snprintf(j->diagnostic,sizeof(j->diagnostic),"curl unavailable (PATH)");return ST_NETWORK;}
 int in[2],out[2];if(pipe2(in,O_CLOEXEC))return ST_NETWORK;if(pipe2(out,O_CLOEXEC)){close(in[0]);close(in[1]);return ST_NETWORK;}
 j->retry_after=0;char cfg[16384];if(j->provider==PROVIDER_GROQ){
  const char *single[]={text};char *request=groq_config(j,count>1?items:single,count);
  if(!request||strlen(request)>=sizeof(cfg)){free(request);close(in[0]);close(in[1]);close(out[0]);close(out[1]);return ST_LONG;}
  snprintf(cfg,sizeof(cfg),"%s",request);free(request);
 }else if(j->provider==0){
  char *q=url_encode(text,strlen(text));if(!q){close(in[0]);close(in[1]);close(out[0]);close(out[1]);return ST_NETWORK;}
  snprintf(cfg,sizeof(cfg),"url = \"https://api.mymemory.translated.net/get?q=%s&langpair=%s%%7C%s&mt=1\"\n",q,language_code(j->source),language_code(j->target));free(q);
 }else{
  JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);if(count>1){JSON_Value *a=json_value_init_array();for(int i=0;i<count;i++)json_array_append_string(json_value_get_array(a),items[i]);json_object_set_value(o,"q",a);}else json_object_set_string(o,"q",text);if(!j->source_auto)json_object_set_string(o,"source",language_code(j->source));json_object_set_string(o,"target",language_code(j->target));json_object_set_string(o,"format","text");
  char *body=json_serialize_to_string(v);json_value_free(v);if(!body){close(in[0]);close(in[1]);close(out[0]);close(out[1]);return ST_NETWORK;}
  char escaped[6144];size_t at=0;for(const char *p=body;*p&&at+3<sizeof(escaped);p++){if(*p=='\\'||*p=='"')escaped[at++]='\\';escaped[at++]=*p;}escaped[at]=0;json_free_serialized_string(body);
  snprintf(cfg,sizeof(cfg),"url = \"https://translation.googleapis.com/language/translate/v2?key=%s\"\nheader = \"Content-Type: application/json; charset=utf-8\"\ndata = \"%s\"\n",j->key,escaped);
 }
 /* Keep source descriptors above stdio, including old libc same-fd dup cases. */
 int *ends[]={&in[0],&in[1],&out[0],&out[1]};for(int k=0;k<4;k++)if(*ends[k]<3){int moved=fcntl(*ends[k],F_DUPFD_CLOEXEC,3);if(moved<0){for(int n=0;n<4;n++)close(*ends[n]);return ST_NETWORK;}close(*ends[k]);*ends[k]=moved;}
 /* glibc posix_spawn avoids copying the large audio/UI process for each request.
  * No non-async-safe code runs in a forked child of the threaded player. */
 pid_t child=-1;posix_spawn_file_actions_t actions;int spawn_error=posix_spawn_file_actions_init(&actions);
 if(!spawn_error){
  spawn_error=posix_spawn_file_actions_adddup2(&actions,in[0],STDIN_FILENO);
  if(!spawn_error)spawn_error=posix_spawn_file_actions_adddup2(&actions,out[1],STDOUT_FILENO);
  if(!spawn_error)spawn_error=posix_spawn_file_actions_addopen(&actions,STDERR_FILENO,"/dev/null",O_WRONLY,0);
  /* Curl must not keep the player's PCM/backend sockets or session listener
   * alive after a track switch/crash. Only stdin/stdout/stderr are inherited. */
  DIR *fds=opendir("/proc/self/fd");if(fds){int own=dirfd(fds);struct dirent *e;
   while(!spawn_error&&(e=readdir(fds))){char *end=NULL;long fd=strtol(e->d_name,&end,10);if(end&&!*end&&fd>=3&&fd!=own)spawn_error=posix_spawn_file_actions_addclose(&actions,(int)fd);}closedir(fds);
  }else{int descriptors[]={in[0],in[1],out[0],out[1]};for(int i=0;i<4&&!spawn_error;i++)spawn_error=posix_spawn_file_actions_addclose(&actions,descriptors[i]);}
  char *argv[]={"curl","-sS","--proto","=https","--proto-redir","=https","--cacert","./res/cacert.pem","--connect-timeout","4","--max-time",j->provider==PROVIDER_GROQ?"15":"8","--max-redirs","0","-A","NXT-Retro-Music-Lyrics-v1.8.9","--include","-w","\nNXT_HTTP_STATUS:%{http_code}","-K","-",NULL};
  if(!spawn_error)spawn_error=posix_spawn(&child,j->curl_path,&actions,NULL,argv,j->env);posix_spawn_file_actions_destroy(&actions);
 }
 close(in[0]);close(out[1]);if(spawn_error){snprintf(j->diagnostic,sizeof(j->diagnostic),"spawn errno=%d",spawn_error);close(in[1]);close(out[0]);return ST_NETWORK;}
 /* Nonblocking stdin keeps cancellation bounded for larger Groq JSON batches. */
 fcntl(in[1],F_SETFL,fcntl(in[1],F_GETFL)|O_NONBLOCK);
 size_t len=strlen(cfg),sent=0;struct timespec write_begin;clock_gettime(CLOCK_MONOTONIC,&write_begin);
 while(sent<len&&!cancelled(j)&&!paused){
  struct timespec now;clock_gettime(CLOCK_MONOTONIC,&now);if(now.tv_sec-write_begin.tv_sec>4)break;
  ssize_t n=write(in[1],cfg+sent,len-sent);if(n>0){sent+=n;continue;}if(n<0&&errno==EINTR)continue;
  if(n<0&&(errno==EAGAIN||errno==EWOULDBLOCK)){struct pollfd p={in[1],POLLOUT,0};poll(&p,1,50);continue;}break;
 }
 memset(cfg,0,sizeof(cfg));close(in[1]);
 fcntl(out[0],F_SETFL,fcntl(out[0],F_GETFL)|O_NONBLOCK);char *body=calloc(REPLY_LIMIT+1,1);size_t size=0;int state=0,status=0;bool eof=false,reaped=false;
 struct timespec begin;clock_gettime(CLOCK_MONOTONIC,&begin);
 for(;;){
  struct timespec now;clock_gettime(CLOCK_MONOTONIC,&now);if((now.tv_sec-begin.tv_sec)*1000+(now.tv_nsec-begin.tv_nsec)/1000000>(j->provider==PROVIDER_GROQ?16500:9500)){state=ST_NETWORK;break;}
  if(cancelled(j)||paused){state=-1;break;}if(!body||sent!=len){state=ST_NETWORK;break;}
  struct pollfd p={out[0],POLLIN|POLLHUP,0};poll(&p,1,50);
  for(;;){char block[2048];ssize_t n=read(out[0],block,sizeof(block));if(n>0){if(size+(size_t)n>REPLY_LIMIT){state=ST_NETWORK;break;}memcpy(body+size,block,n);size+=n;body[size]=0;continue;}if(n==0)eof=true;break;}
  if(state)break;if(!reaped){pid_t r=waitpid(child,&status,WNOHANG);if(r==child)reaped=true;}if(reaped&&eof)break;if(eof&&!reaped){struct timespec wait={0,10000000};nanosleep(&wait,NULL);}
 }
 if(!reaped){kill(child,SIGTERM);for(int i=0;i<10;i++){if(waitpid(child,&status,WNOHANG)==child){reaped=true;break;}struct timespec t={0,10000000};nanosleep(&t,NULL);}if(!reaped){kill(child,SIGKILL);while(waitpid(child,&status,0)<0&&errno==EINTR){}}}
 close(out[0]);if(!state&&(!WIFEXITED(status)||WEXITSTATUS(status)!=0||!size||!eof))state=ST_NETWORK;
 if(!state){char *mark=strrchr(body,'\n');if(mark&&!strncmp(mark,"\nNXT_HTTP_STATUS:",17)){int http=atoi(mark+17);snprintf(j->diagnostic,sizeof(j->diagnostic),"HTTP=%d curl=%d",http,WIFEXITED(status)?WEXITSTATUS(status):-1);*mark=0;if(j->provider==PROVIDER_GROQ)groq_read_headers(j,body);else strip_http_headers(body);if(http==429)state=ST_QUOTA;else if(http==401||http==403)state=j->provider?ST_KEY:ST_QUOTA;else if(http!=200)state=j->provider==PROVIDER_GROQ&&(http==400||http==404||http==422)?ST_SERVICE:ST_NETWORK;}else state=ST_NETWORK;}
 if(state){if(j->provider==PROVIDER_GROQ&&state==ST_QUOTA&&j->retry_after){size_t at=strlen(j->diagnostic);snprintf(j->diagnostic+at,sizeof(j->diagnostic)-at," retry=%ds",j->retry_after);}if(!*j->diagnostic)snprintf(j->diagnostic,sizeof(j->diagnostic),"transport state=%d curl=%d",state,WIFEXITED(status)?WEXITSTATUS(status):-1);free(body);return state;}j->diagnostic[0]=0;*reply=body;return 0;
}
static char *clean(const char *input){
 if(!input||!valid_text(input,LINE_LIMIT-1))return NULL;char *out=calloc(LINE_LIMIT,1);if(!out)return NULL;size_t at=0;
 for(const char *p=input;*p&&at+5<LINE_LIMIT;){
  struct {const char *entity;char c;} map[]={{"&amp;",'&'},{"&quot;",'"'},{"&apos;",'\''},{"&#39;",'\''},{"&lt;",'<'},{"&gt;",'>'}};
  bool matched=false;for(unsigned i=0;i<sizeof(map)/sizeof(*map);i++){size_t n=strlen(map[i].entity);if(!strncmp(p,map[i].entity,n)){out[at++]=map[i].c;p+=n;matched=true;break;}}
  if(matched)continue;if(*p=='\r'||*p=='\n'||*p=='\t'){out[at++]=' ';p++;}else out[at++]=*p++;
 }
 out[at]=0;if(!valid_text(out,LINE_LIMIT-1)||!*out){free(out);return NULL;}return out;
}
static int translate_chunk(Job *j,const char *text,char **result){
 char *reply=NULL;int state=transport_batch(j,text,NULL,1,&reply);if(state)return state;
 if(j->provider==PROVIDER_GROQ){char *parts[1]={0};state=groq_parse(reply,1,parts);free(reply);if(!state)*result=parts[0];return state;}
 JSON_Value *v=json_parse_string(reply);free(reply);JSON_Object *o=json_value_get_object(v);const char *text_out=NULL;
 if(j->provider==0){
  int status=json_object_get_number(o,"responseStatus");
  if(json_object_get_boolean(o,"quotaFinished")==1||status==429)state=ST_QUOTA;
  else if(status==401||status==403)state=ST_QUOTA;
  else if(status!=200)state=ST_NETWORK;
  else text_out=json_object_get_string(json_object_get_object(o,"responseData"),"translatedText");
 }else{
  JSON_Object *error=json_object_get_object(o,"error");int code=json_object_get_number(error,"code");
  if(code==429)state=ST_QUOTA;else if(code==400||code==401||code==403)state=ST_KEY;else if(code)state=ST_NETWORK;
  else text_out=json_object_get_string(json_array_get_object(json_object_get_array(json_object_get_object(o,"data"),"translations"),0),"translatedText");
 }
 if(!state&&text_out&&!strncmp(text_out,"MYMEMORY WARNING",16))state=ST_QUOTA;
 if(!state){*result=clean(text_out);if(!*result)state=ST_NETWORK;}json_value_free(v);return state;
}
static int translate_line(Job *j,int index,char **result){
 const char *p=j->input[index];char *out=calloc(LINE_LIMIT,1);if(!out)return ST_LONG;size_t at=0;
 while(*p){
  if(!wait_visible(j)){free(out);return -1;}size_t n=chunk_size(p);if(!n){free(out);return ST_LONG;}
  char input[CHUNK_LIMIT+1];memcpy(input,p,n);input[n]=0;char *part=NULL;int state=translate_chunk(j,input,&part);
  if(state==-1&&!cancelled(j)){if(!wait_visible(j)){free(out);return -1;}continue;}
  if(state){free(out);return state;}size_t bytes=strlen(part);if(at+bytes+2>=LINE_LIMIT){free(part);free(out);return ST_LONG;}
  if(at)out[at++]=' ';memcpy(out+at,part,bytes+1);at+=bytes;free(part);p+=n;while(*p==' '||*p=='\t')p++;
  if(*p&&!rest(j,j->provider?400:1000)){free(out);return -1;}
 }
 *result=out;return 0;
}
/* Small batches reduce child starts/TLS handshakes. MyMemory has no structured
 * multi-q response: accept it only if all newline boundaries survive, otherwise
 * fall back to individual lines for this job without publishing misaligned text. */
static int translate_group(Job *j,int *indices,int count,char **result){
 const char *items[GROUP_MAX];char joined[GROQ_BYTES+1];size_t used=0;size_t limit=j->provider==PROVIDER_GROQ?GROQ_BYTES:CHUNK_LIMIT;
 for(int k=0;k<count;k++){items[k]=j->input[indices[k]];size_t n=strlen(items[k]);if(used+n+(k>0)>limit)return -2;if(k)joined[used++]='\n';memcpy(joined+used,items[k],n);used+=n;}joined[used]=0;
 char *reply=NULL;int state=transport_batch(j,joined,items,count,&reply);if(state)return state;
 if(j->provider==PROVIDER_GROQ){state=groq_parse(reply,count,result);free(reply);return state;}
 JSON_Value *v=json_parse_string(reply);free(reply);JSON_Object *o=json_value_get_object(v);
 if(j->provider){
  JSON_Object *error=json_object_get_object(o,"error");int code=json_object_get_number(error,"code");
  if(code==429)state=ST_QUOTA;else if(code==400||code==401||code==403)state=ST_KEY;else if(code)state=ST_NETWORK;
  JSON_Array *a=json_object_get_array(json_object_get_object(o,"data"),"translations");
  if(!state&&json_array_get_count(a)!=(size_t)count)state=ST_NETWORK;
  for(int k=0;k<count&&!state;k++){result[k]=clean(json_object_get_string(json_array_get_object(a,k),"translatedText"));if(!result[k])state=ST_NETWORK;}
 }else{
  int code=json_object_get_number(o,"responseStatus");
  if(code==429||code==401||code==403||json_object_get_boolean(o,"quotaFinished")==1)state=ST_QUOTA;else if(code!=200)state=ST_NETWORK;
  const char *value=json_object_get_string(json_object_get_object(o,"responseData"),"translatedText");
  if(!state&&value&&!strncmp(value,"MYMEMORY WARNING",16))state=ST_QUOTA;
  if(!state){
   char *split=value?strdup(value):NULL;int k=0;if(!split)state=ST_NETWORK;
   else{char *p=split;for(;;){char *end=strchr(p,'\n');if(end)*end=0;size_t n=strlen(p);if(n&&p[n-1]=='\r')p[n-1]=0;
     if(k>=count||!*p){state=-2;break;}result[k]=clean(p);if(!result[k++]){state=-2;break;}if(!end)break;p=end+1;}
    if(k!=count)state=-2;free(split);}
  }
 }
 json_value_free(v);if(state)for(int k=0;k<count;k++){free(result[k]);result[k]=NULL;}return state;
}
static bool install_line(Job *j,int i,char *out){
 if(!out||j->output_bytes+strlen(out)+1>OUTPUT_LIMIT){free(out);j->due[i]=(time_t)-1;return false;}
 j->output[i]=out;j->output_bytes+=strlen(out)+1;j->done++;publish(j,i);return true;
}
static void process(Job *j){
 fprintf(stderr,"[NXT translation] start generation=%u lines=%d provider=%d target=%d curl=%d\n",j->generation,j->count,j->provider,j->target,*j->curl_path!=0);
 j->source_auto=j->source==0;j->source=j->source?j->source-1:guess(j);j->state=ST_BUSY;digest(j);read_cache(j);
 if(j->done==j->count){j->state=ST_READY;publish(j,-1);return;}
 if(j->provider!=0&&!key_valid(j->key)){j->state=ST_KEY;publish(j,-1);return;}
 int since_save=0;
 while(!cancelled(j)){
  if(!wait_visible(j))break;
  time_t now=time(NULL),until=atomic_load(&cooldown[j->provider]);
  if(until>now){j->state=atomic_load(&cooldown_state[j->provider]);publish(j,-1);if(!rest(j,1000))break;continue;}
  j->state=ST_BUSY;int selected=-1,start=atomic_load(&focus),remaining=0;if(start<0||start>=j->count)start=0;
  /* Resolve repeats/empty/source==target without any HTTP. */
  for(int step=0;step<j->count;step++){int i=(start+step)%j->count;if(j->output[i])continue;char *out=NULL;
   if(!*j->input[i]||(j->source==j->target&&(!j->provider||!j->source_auto)))out=strdup(j->input[i]);
   else for(int k=0;k<j->count;k++)if(j->output[k]&&!strcmp(j->input[k],j->input[i])){out=strdup(j->output[k]);break;}
   if(out){if(install_line(j,i,out))since_save++;continue;}
   if(j->due[i]!=(time_t)-1){remaining++;if(selected<0&&j->due[i]<=now)selected=i;}
  }
  if(j->done==j->count){j->state=ST_READY;break;}
  if(!remaining){j->state=ST_LONG;break;}
  if(selected<0){j->state=ST_NETWORK;publish(j,-1);if(!rest(j,1000))break;continue;}
  publish(j,-1);int indices[GROUP_MAX]={selected},count=1;size_t bytes=strlen(j->input[selected]);
  size_t limit=j->provider==PROVIDER_GROQ?GROQ_BYTES:CHUNK_LIMIT;int max_group=j->provider==PROVIDER_GROQ?(j->groq_group_limit?j->groq_group_limit:GROUP_MAX):6;
  if(!j->batch_disabled&&bytes<=limit&&!strchr(j->input[selected],'\n')){
   for(int step=1;step<j->count&&count<max_group;step++){int i=(selected+step)%j->count;size_t n=strlen(j->input[i]);
    if(j->output[i]||j->due[i]==(time_t)-1||j->due[i]>now||!n||strchr(j->input[i],'\n')||bytes+n+1>limit)continue;
    bool duplicate=false;for(int k=0;k<count;k++)if(!strcmp(j->input[i],j->input[indices[k]]))duplicate=true;if(duplicate)continue;
    indices[count++]=i;bytes+=n+1;}
  }
  char *outputs[GROUP_MAX]={0};int state=(count>1||j->provider==PROVIDER_GROQ)?translate_group(j,indices,count,outputs):translate_line(j,selected,&outputs[0]);
  if(state==-2){if(j->provider==PROVIDER_GROQ&&count>1)j->groq_group_limit=count/2;else j->batch_disabled=true;continue;}
  if(state==-1){if(!cancelled(j))continue;break;}
  if(state){
   fprintf(stderr,"[NXT translation] error generation=%u state=%d done=%d/%d %s\n",j->generation,state,j->done,j->count,j->diagnostic);
   j->state=state;if(state==ST_NETWORK&&count>1&&j->provider!=PROVIDER_GROQ)j->batch_disabled=true;
   if(state==ST_QUOTA||state==ST_KEY||state==ST_SERVICE){atomic_store(&cooldown_state[j->provider],state);atomic_store(&cooldown[j->provider],time(NULL)+(state==ST_QUOTA&&j->provider==PROVIDER_GROQ&&j->retry_after?j->retry_after:900));}
   else for(int k=0;k<count;k++){int i=indices[k];if(state==ST_LONG)j->due[i]=(time_t)-1;else{if(j->failures[i]<3)j->failures[i]++;j->due[i]=now+(j->failures[i]==1?2:j->failures[i]==2?10:30);}}
   publish(j,-1);if(since_save){save_cache(j);since_save=0;}if(!rest(j,1000))break;continue;
  }
  for(int k=0;k<count;k++)if(install_line(j,indices[k],outputs[k]))since_save++;
  if(since_save>=24){if(!save_cache(j))j->state=ST_SAVE;since_save=0;}
  if(j->done==j->count){j->state=ST_READY;break;}
  if(!rest(j,j->provider==PROVIDER_GROQ?2200:j->provider?400:1000))break;
 }
 fprintf(stderr,"[NXT translation] end generation=%u done=%d/%d cancelled=%d\n",j->generation,j->done,j->count,cancelled(j));
 if(!save_cache(j)&&!cancelled(j))j->state=ST_SAVE;publish(j,-1);
}
static void *worker(void *unused){
 (void)unused;
#ifdef SYS_gettid
 setpriority(PRIO_PROCESS,(id_t)syscall(SYS_gettid),5);
/* Do not use I/O idle class: continuous SD playback can starve a cold helper. */
#endif
 /* Prevent EPIPE on an exiting curl from terminating the app. */
 sigset_t blocked;sigemptyset(&blocked);sigaddset(&blocked,SIGPIPE);pthread_sigmask(SIG_BLOCK,&blocked,NULL);
 for(;;){pthread_mutex_lock(&lock);while(!queued&&!quitting)pthread_cond_wait(&wake,&lock);Job *j=queued;queued=NULL;bool stop=quitting;pthread_mutex_unlock(&lock);if(stop){free_job(j);break;}process(j);free_job(j);}return NULL;
}
static void clear_view(void){
 for(int i=0;i<LYRICS_MAX_LINES;i++){free(shown[i]);shown[i]=NULL;}
 pthread_mutex_lock(&lock);for(int i=0;i<LYRICS_MAX_LINES;i++){free(pending[i]);pending[i]=NULL;}pending_gen=atomic_load(&generation);pending_state=ST_WAIT;pending_done=0;pthread_mutex_unlock(&lock);view_revision++;
}
void NxtTranslate_tick(void){
 bool hidden=NxtSession_isHidden()||PWR_getDisplayOff();
 if(atomic_exchange(&paused,hidden)!=hidden){pthread_mutex_lock(&lock);pthread_cond_broadcast(&wake);pthread_mutex_unlock(&lock);}
 unsigned lyrics=Lyrics_revision();int listener=NxtPersonal_id();
 if(seen_lyrics!=lyrics||seen_mode!=(nxt.translation_mode!=0)||seen_language!=nxt.language||seen_source!=nxt.translation_source||seen_listener!=listener||seen_service!=service_revision){
  load_service();atomic_fetch_add(&generation,1);clear_view();ui_diagnostic[0]=pending_diagnostic[0]=0;seen_lyrics=lyrics;seen_mode=nxt.translation_mode!=0;seen_language=nxt.language;seen_source=nxt.translation_source;seen_listener=listener;seen_service=service_revision;atomic_store(&focus,0);
  ui_done=ui_total=0;ui_state=nxt.translation_mode?ST_WAIT:ST_OFF;
  Job *j=NULL;int count=nxt.translation_mode?Lyrics_lineCount():0;
  if(count>0&&count<=LYRICS_MAX_LINES){j=calloc(1,sizeof(*j));if(j){j->count=count;j->generation=generation;j->target=nxt.language;j->source=nxt.translation_source;j->provider=provider;bool env_ok=snapshot_environment(j);snprintf(j->key,sizeof(j->key),"%s",provider==PROVIDER_GROQ?groq_key:service_key);snprintf(j->model,sizeof(j->model),"%s",groq_models[groq_model]);userdata_snpath("translation-cache",j->cache_dir,sizeof(j->cache_dir));size_t total=0;bool ok=env_ok;for(int i=0;i<count;i++){const char *s=Lyrics_lineText(i);if(!s)s="";total+=strlen(s)+1;if(total>INPUT_LIMIT||!valid_text(s,2047)||!(j->input[i]=strdup(s))){ok=false;break;}}if(!ok||Lyrics_revision()!=lyrics){free_job(j);j=NULL;ui_state=ST_LONG;}else ui_total=count;}}
  pthread_mutex_lock(&lock);pending_state=ui_state;free_job(queued);queued=j;pthread_cond_broadcast(&wake);pthread_mutex_unlock(&lock);
  if(j&&!started){quitting=false;if(pthread_create(&thread,NULL,worker,NULL)==0)started=true;else{pthread_mutex_lock(&lock);free_job(queued);queued=NULL;pending_state=ST_NETWORK;pthread_mutex_unlock(&lock);ui_state=ST_NETWORK;}}
 }
 if(!nxt.translation_mode)return;
 {int i=Lyrics_currentIndex(NxtPlayback_position());atomic_store(&focus,i<0?0:i);}
 if(!atomic_load(&pending_dirty)||pthread_mutex_trylock(&lock))return;
 atomic_store(&pending_dirty,false);
 if(pending_gen==atomic_load(&generation)&&nxt.translation_mode){for(int i=0;i<ui_total;i++)if(pending[i]){free(shown[i]);shown[i]=pending[i];pending[i]=NULL;view_revision++;}if(ui_state!=pending_state||ui_done!=pending_done)view_revision++;ui_state=pending_state;ui_done=pending_done;snprintf(ui_diagnostic,sizeof(ui_diagnostic),"%s",pending_diagnostic);}
 pthread_mutex_unlock(&lock);
}
const char *NxtTranslate_line(int i){
 if(!nxt.translation_mode||i<0||i>=LYRICS_MAX_LINES||seen_lyrics!=Lyrics_revision()||seen_language!=nxt.language||seen_source!=nxt.translation_source||seen_listener!=NxtPersonal_id())return NULL;
 return shown[i];
}
const char *NxtTranslate_display(int i){const char *s=NxtTranslate_line(i);return s&&*s?s:Lyrics_lineText(i);}
unsigned NxtTranslate_revision(void){return view_revision;}
const char *NxtTranslate_modeName(int m){return m==1?Nxt_text("Translation","Bản dịch"):m==2?Nxt_text("Bilingual","Song ngữ"):Nxt_text("Off","Tắt");}
const char *NxtTranslate_status(void){
 if(nxt.translation_mode&&ui_total==0){snprintf(status_text,sizeof(status_text),"%s",Lyrics_status());return status_text;}
 const char *s=ui_state==ST_OFF?Nxt_text("Off · original lyrics","Tắt · lời gốc"):ui_state==ST_WAIT?Nxt_text("Waiting for lyrics","Đang chờ lời bài hát"):ui_state==ST_BUSY?Nxt_text("Translating in background","Đang dịch ở nền"):ui_state==ST_READY?Nxt_text("Translation ready","Bản dịch đã sẵn sàng"):ui_state==ST_QUOTA?Nxt_text("Service limit · original lyrics available","Dịch vụ hết hạn mức · vẫn có lời gốc"):ui_state==ST_KEY?Nxt_text("Check API key / service access","Kiểm tra khóa API / quyền dịch vụ"):ui_state==ST_SERVICE?Nxt_text("Check Groq model / request compatibility","Kiểm tra model Groq / yêu cầu API"):ui_state==ST_LONG?Nxt_text("Lyrics too long · original lyrics available","Lời quá dài · vẫn có lời gốc"):ui_state==ST_SAVE?Nxt_text("Cannot save translation cache","Không lưu được bộ nhớ bản dịch"):Nxt_text("Network error · automatically resuming","Lỗi mạng · đang tự tiếp tục");
 if(paused&&nxt.translation_mode&&ui_done<ui_total)s=Nxt_text("Translation paused · show app / screen","Dịch tạm dừng · mở app / màn hình");
 if(ui_total)snprintf(status_text,sizeof(status_text),"%s · %d/%d%s%s",s,ui_done,ui_total,*ui_diagnostic?" · ":"",ui_diagnostic);else snprintf(status_text,sizeof(status_text),"%s",s);return status_text;
}
static void retry(void){if(!ui_total)Lyrics_retry();atomic_store(&cooldown[provider],0);service_revision++;NxtTranslate_tick();}
void NxtTranslate_quit(void){
 quitting=true;atomic_fetch_add(&generation,1);pthread_mutex_lock(&lock);pthread_cond_broadcast(&wake);pthread_mutex_unlock(&lock);
 if(started){pthread_join(thread,NULL);started=false;}free_job(queued);queued=NULL;clear_view();memset(service_key,0,sizeof(service_key));memset(groq_key,0,sizeof(groq_key));service_loaded=false;seen_mode=-1;quitting=false;
}
bool NxtTranslate_copyGroqKey(char *out,size_t size){load_service();if(!out||size<sizeof(groq_key))return false;snprintf(out,size,"%s",groq_key);return key_valid(groq_key);}
ModuleExitReason NxtTranslate_settings(DisplayContext *d){
 load_service();int selected=0;
 for(;;){
  char mode[180],source[200],service[160],model[160];snprintf(mode,sizeof(mode),"%s\t%s → %s",Nxt_text("Display","Hiển thị"),NxtTranslate_modeName(nxt.translation_mode),NxtLocale_name(nxt.language));
  snprintf(source,sizeof(source),"%s\t%s",Nxt_text("Lyric language","Ngôn ngữ lời"),nxt.translation_source?NxtLocale_name(nxt.translation_source-1):Nxt_text("Automatic","Tự nhận diện"));
  snprintf(service,sizeof(service),"%s\t%s",Nxt_text("Translation service","Dịch vụ dịch"),service_names[provider]);
  snprintf(model,sizeof(model),"%s\t%s",Nxt_text("Groq model","Model Groq"),groq_models[groq_model]);
  const char *rows[]={mode,source,service,Nxt_text("Groq API key\nStored only on this device","Khóa API Groq\nChỉ lưu trên máy"),model,Nxt_text("Google Cloud key\nOptional · stored only on this device","Khóa Google Cloud\nTùy chọn · chỉ lưu trên máy"),Nxt_text("Retry translation","Thử dịch lại"),Nxt_text("Status & usage","Trạng thái & cách dùng"),Nxt_text("Voice-timed karaoke · Groq","Karaoke theo giọng hát · Groq")};
  NxtPick p=Nxt_pickLive(d,Nxt_text("Lyric translation","Dịch lời bài hát"),rows,9,&selected,Nxt_text("A / ←→ Change · B Back","A / ←→ Đổi · B Về"));
  if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
  if(p.action!=1&&p.action!=5&&p.action!=6)continue;int delta=p.action==5?-1:1;
  if(selected==0){nxt.translation_mode=(nxt.translation_mode+3+delta)%3;Nxt_save();NxtTranslate_tick();}
  else if(selected==1){
   const char *langs[15];langs[0]=Nxt_text("Automatic · select manually if needed","Tự nhận diện · chọn lại nếu cần");for(int i=0;i<14;i++)langs[i+1]=NxtLocale_name(i);
   int language=nxt.translation_source;NxtPick q=Nxt_pick(d,Nxt_text("Lyric language","Ngôn ngữ lời"),langs,15,&language,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(q.action==9)return MODULE_EXIT_QUIT;if(q.action==1){nxt.translation_source=language;Nxt_save();NxtTranslate_tick();}
  }else if(selected==2){int old=provider;provider=(provider+PROVIDER_COUNT+delta)%PROVIDER_COUNT;if(save_service())retry();else{provider=old;Toast_show(Nxt_text("Cannot save service","Không lưu được dịch vụ"),2000);}}
  else if(selected==3||selected==5){
   bool is_groq=selected==3;char *stored=is_groq?groq_key:service_key;
   if(is_groq){
    const char *methods[]={Nxt_text("Type on device","Nhập trên máy"),Nxt_text("Import from TXT","Nhập từ TXT")};int method=0;
    NxtPick choice=Nxt_pick(d,Nxt_text("Groq API key","Khóa API Groq"),methods,2,&method,Nxt_text("A Select · B Back","A Chọn · B Về"));
    if(choice.action==9)return MODULE_EXIT_QUIT;if(choice.action!=1)continue;
    if(method==1){
     char key[256],path[1024];int result=NxtKeyFile_readGroq("./groq-api-key.txt",key,sizeof(key));
     if(result==1){const char *sd=getenv("SDCARD_PATH");snprintf(path,sizeof(path),"%s/Music/groq-api-key.txt",sd&&*sd?sd:"/mnt/SDCARD");result=NxtKeyFile_readGroq(path,key,sizeof(key));}
     if(!result){char old[256];snprintf(old,sizeof(old),"%s",groq_key);snprintf(groq_key,sizeof(groq_key),"%s",key);
      if(save_service()){retry();Toast_show(Nxt_text("Groq key imported","Đã nhập khóa Groq"),2000);}
      else{snprintf(groq_key,sizeof(groq_key),"%s",old);Toast_show(Nxt_text("Cannot save key","Không lưu được khóa"),2000);}memset(old,0,sizeof(old));
     }else Toast_show(result==1?Nxt_text("Place groq-api-key.txt in Music or beside launch.sh","Đặt groq-api-key.txt trong Music hoặc cạnh launch.sh"):Nxt_text("Invalid TXT · use one complete Groq key","TXT không hợp lệ · ghi một khóa Groq đầy đủ"),3000);
     memset(key,0,sizeof(key));continue;
    }
   }
   char *key=Keyboard_openLatin(is_groq?Nxt_text("Groq API key · empty removes it","Khóa API Groq · để trống để xóa"):Nxt_text("Google Cloud API key · empty removes it","Khóa API Google Cloud · để trống để xóa"),255);
   if(key){if(!*key||key_valid(key)){char old[256];snprintf(old,sizeof(old),"%s",stored);snprintf(stored,256,"%s",key);if(save_service())retry();else{snprintf(stored,256,"%s",old);Toast_show(Nxt_text("Cannot save key","Không lưu được khóa"),2000);}memset(old,0,sizeof(old));}else Toast_show(Nxt_text("Invalid API key","Khóa API không hợp lệ"),2000);memset(key,0,strlen(key));free(key);}
  }else if(selected==4){int old=groq_model;groq_model=(groq_model+3+delta)%3;if(save_service())retry();else{groq_model=old;Toast_show(Nxt_text("Cannot save model","Không lưu được model"),2000);}}
  else if(selected==6)retry();
  else if(selected==8){if(NxtKaraoke_settings(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
  else{const char *doc[]={NxtTranslate_status(),Nxt_text("Target language follows the app language. Original timestamps and lyric delay remain unchanged.","Dịch sang ngôn ngữ app. Giữ mốc thời gian và độ trễ lời gốc."),
   Nxt_text("Bilingual shows the original and translation together on Now Playing and full-screen lyrics. Missing translations keep the original.","Song ngữ hiện cùng lời gốc và bản dịch ở Đang phát và màn hình lời đầy đủ. Câu chưa dịch vẫn hiện lời gốc."),
   Nxt_text("Groq uses your own key from console.groq.com/keys. Keep the Free plan for free-tier limits. MyMemory needs no key; Google Cloud keeps its separate key and billing.","Groq dùng khóa riêng tại console.groq.com/keys. Giữ gói Free để dùng hạn mức miễn phí. MyMemory không cần khóa; Google Cloud giữ khóa và chi phí riêng."),
   Nxt_text("Groq translates groups with checked line IDs. 429 waits for Retry-After/reset headers, with a safe fallback. Free service is rate-limited; no automatic paid upgrade. Qwen is a preview model.","Groq dịch nhóm và kiểm tra ID từng dòng. Lỗi 429 chờ theo Retry-After/thời gian phục hồi, có thời gian dự phòng. Gói Free có hạn mức; app không tự nâng gói trả phí. Qwen là model preview."),
   Nxt_text("Only lyric text is sent for translation when enabled. Audio, account tokens, track paths and artist metadata are not sent.","Khi bật, chỉ gửi nội dung lời để dịch. Không gửi âm thanh, token tài khoản, đường dẫn bài hoặc thông tin nghệ sĩ."),
   Nxt_text("Translations: up to 256 MiB on SD, no 512-song limit. Offline reuses saved lines; the oldest saved files are removed only at the budget. No 256 MiB RAM allocation.","Lời dịch: tối đa 256 MB trên thẻ, không giới hạn 512 bài. Offline dùng lại câu đã lưu; chỉ dọn bản cũ khi đạt dung lượng. Không nạp 256 MB vào RAM."),
   Nxt_text("Automatic recognises major scripts; Latin/mixed lyrics may need a manual source language. Machine translations can lose poetic meaning.","Tự nhận diện dựa theo chữ; lời Latin/trộn ngôn ngữ có thể cần chọn nguồn. Dịch máy có thể chưa sát ý thơ.")};
   if(Nxt_document(d,Nxt_text("Lyric translation","Dịch lời bài hát"),doc,sizeof(doc)/sizeof(*doc))==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
  }
 }
}

