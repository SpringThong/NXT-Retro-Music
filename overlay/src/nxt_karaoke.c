/* SPDX-License-Identifier: GPL-3.0-only
 * Opt-in local-song word timing. Whisper supplies measured ASR word intervals;
 * partial word alignment preserves matching intervals and labels estimates.
 * Singing recognition is fallible; estimates are visibly identified.
 * One low-priority, generation-cancelled job; 10-minute/20-MB upload bound.
 */
#define _GNU_SOURCE
#include "nxt_karaoke.h"
#include "api.h"
#include "nxt_decoder.h"
#include "nxt_dsd.h"
#include "nxt_playback.h"
#include "nxt_translate.h"
#include "nxt_write.h"
#include "lyrics.h"
#include "nxt_lyric_words.h"
#include "player.h"
#include "file_utils.h"
#include "parson/parson.h"
#include "mbedtls/sha256.h"
#include "toast.h"
#include <utf8proc.h>
#include <stdatomic.h>
#include <pthread.h>
#include <spawn.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_AUDIO_MS 600000
#define MAX_REPLY (512*1024)
#define MAX_ASR_WORDS 4096
#define MAX_NORMAL 32768
#define CACHE_BYTES (16*1024*1024)
enum {K_IDLE,K_READ,K_UPLOAD,K_READY,K_NOMATCH,K_ERROR,K_CANCEL,K_UNSUPPORTED,K_KEY,K_QUOTA,K_CACHE};
typedef struct {int start,end,at,len;} AsrWord;
typedef struct {
 unsigned generation,playback,revision;bool synced,upload,refresh;int count,duration,matched;
 char path[1024],cache[1400],key[256],curl[1400],wav[80];char **env;
 LyricLine *lines;
} KaraokeJob;
static pthread_t worker;static KaraokeJob *job;
static atomic_bool done,paused;
static atomic_uint generation=1;
static atomic_int state=K_IDLE;
static unsigned seen_playback,seen_revision;static bool force,initialized;
static unsigned char last_hash[32];static bool have_hash;
extern char **environ;
static bool cancelled(KaraokeJob*j){return atomic_load(&generation)!=j->generation;}
static bool wait_visible(KaraokeJob*j){while(atomic_load(&paused)&&!cancelled(j))usleep(40000);return !cancelled(j);}
static bool visible(void){return !PWR_getDisplayOff()&&!NxtSession_isHidden();}
/* Semi-global word alignment tolerates skipped ASR tokens and accent marks.
 * Only good anchors are accepted; incomplete lines retain an explicit estimate
 * flag. Work is bounded to 64 x 160 cells per line on the low-priority worker. */
typedef struct {int start,end;char key[128];bool split;} Recognized;
static int near_word(const char*a,const char*b){
 if(!strcmp(a,b))return 0;size_t na=strlen(a),nb=strlen(b);if(na<4||nb<4||na>nb+1||nb>na+1)return 4;
 for(size_t i=0;i<na;i++)if((unsigned char)a[i]>=128)return 4;
 for(size_t i=0;i<nb;i++)if((unsigned char)b[i]>=128)return 4;
 size_t i=0,k=0;int errors=0;while(i<na&&k<nb){if(a[i]==b[k]){i++;k++;continue;}if(++errors>1)return 4;if(na>=nb)i++;if(nb>=na)k++;}errors+=(i<na||k<nb);return errors==1?1:4;
}
static bool align_words(KaraokeJob *j,const char *reply){
 JSON_Value*v=json_parse_string(reply);JSON_Array*a=json_object_get_array(json_value_get_object(v),"words");size_t count=json_array_get_count(a);
 if(!count||count>MAX_ASR_WORDS){json_value_free(v);return false;}
 Recognized *asr=calloc(MAX_ASR_WORDS*2,sizeof(*asr));if(!asr){json_value_free(v);return false;}int n=0,last=-1;
 for(size_t i=0;i<count&&n<MAX_ASR_WORDS*2;i++){
  JSON_Object*o=json_array_get_object(a,i);const char*t=json_object_get_string(o,"word");double start=json_object_get_number(o,"start"),end=json_object_get_number(o,"end");
  if(!t||strlen(t)>2047||!json_object_has_value_of_type(o,"start",JSONNumber)||!json_object_has_value_of_type(o,"end",JSONNumber)||!isfinite(start)||!isfinite(end)||start<0||end<=start||end>(j->duration+3000)/1000.0||start*1000<last)continue;
  NxtWordToken tokens[LYRICS_MAX_WORDS];int c=NxtWords_tokens(t,tokens,LYRICS_MAX_WORDS);if(!c)continue;int begin=llround(start*1000),finish=llround(end*1000);if(finish-begin<c)continue;
  for(int k=0;k<c&&n<MAX_ASR_WORDS*2;k++){Recognized*w=&asr[n++];w->start=begin+(long long)(finish-begin)*k/c;w->end=begin+(long long)(finish-begin)*(k+1)/c;w->split=c>1;snprintf(w->key,sizeof(w->key),"%s",tokens[k].key);}last=begin;
 }
 json_value_free(v);int cursor=0;
 for(int line=0;line<j->count&&!cancelled(j);line++){
  LyricLine*l=&j->lines[line];if(l->word_count&&!l->estimated){j->matched++;continue;}
  NxtWordToken tokens[LYRICS_MAX_WORDS];int m=NxtWords_tokens(l->text,tokens,LYRICS_MAX_WORDS);if(!m)continue;
  int first=cursor,end=line+1<j->count?j->lines[line+1].time_ms:j->duration;
  if(j->synced){while(first<n&&asr[first].end<l->time_ms-1200)first++;}
  int candidates=0;while(first+candidates<n&&candidates<160&&(!j->synced||asr[first+candidates].start<end+400))candidates++;
  if(!candidates)continue;unsigned short dp[LYRICS_MAX_WORDS+1][161];unsigned char way[LYRICS_MAX_WORDS+1][161];
  for(int k=0;k<=candidates;k++){dp[0][k]=0;way[0][k]=2;}
  for(int i=1;i<=m;i++){dp[i][0]=i*2;way[i][0]=1;for(int k=1;k<=candidates;k++){
   int diagonal=dp[i-1][k-1]+near_word(tokens[i-1].key,asr[first+k-1].key),missing=dp[i-1][k]+2,extra=dp[i][k-1]+1;
   dp[i][k]=diagonal;way[i][k]=0;if(missing<dp[i][k]){dp[i][k]=missing;way[i][k]=1;}if(extra<dp[i][k]){dp[i][k]=extra;way[i][k]=2;}
  }}
  int best=1;for(int k=2;k<=candidates;k++)if(dp[m][k]<dp[m][best])best=k;
  int map[LYRICS_MAX_WORDS],i=m,k=best,hits=0,last_anchor=-1;bool estimated=false;for(int q=0;q<m;q++)map[q]=-1;
  while(i>0){int step=way[i][k];if(step==0){int cost=near_word(tokens[i-1].key,asr[first+k-1].key);if(cost<2){map[i-1]=first+k-1;hits++;if(cost||asr[first+k-1].split)estimated=true;}i--;k--;}else if(step==1)i--;else k--;}
  if(hits*100<m*45||(m>2&&hits<2))continue;
  LyricLine candidate=*l;candidate.word_count=0;candidate.estimated=0;
  if(!j->synced){for(int q=0;q<m;q++)if(map[q]>=0){candidate.time_ms=asr[map[q]].start;break;}end=asr[first+best-1].end+300;if(end>j->duration)end=j->duration;}
  if(end<=candidate.time_ms||!NxtWords_estimate(&candidate,end))continue;
  for(int q=0;q<m;q++)if(map[q]>=0){Recognized*w=&asr[map[q]];int begin=w->start<candidate.time_ms?candidate.time_ms:w->start,finish=w->end>end?end:w->end;if(finish<=begin){map[q]=-1;hits--;continue;}candidate.words[q]=(LyricWord){begin,finish,tokens[q].offset,tokens[q].length};last_anchor=map[q];}
  if(hits*100<m*45||(m>2&&hits<2))continue;
  /* Interpolate only holes between accepted measured anchors. Such a row is
   * labelled estimated; recognizer output never replaces the original words. */
  for(int q=0;q<m;){if(map[q]>=0){q++;continue;}int begin=q;while(q<m&&map[q]<0)q++;int a_ms=begin?candidate.words[begin-1].end_ms:candidate.time_ms,b_ms=q<m?candidate.words[q].time_ms:end;
   if(b_ms-a_ms<q-begin){hits=0;break;}for(int z=begin;z<q;z++){candidate.words[z].time_ms=a_ms+(long long)(b_ms-a_ms)*(z-begin)/(q-begin);candidate.words[z].end_ms=a_ms+(long long)(b_ms-a_ms)*(z-begin+1)/(q-begin);}estimated=true;
  }
  bool valid=hits>0;for(int q=0;q<m;q++)if(candidate.words[q].end_ms<=candidate.words[q].time_ms||(q&&candidate.words[q].time_ms<candidate.words[q-1].time_ms))valid=false;
  if(!valid)continue;candidate.estimated=estimated;candidate.aligned=1;*l=candidate;j->matched++;cursor=last_anchor+1;
 }
 /* For plain text only, interpolate unmatched line cues between recognised
  * lines. They remain estimated and the original isSynced flag stays false. */
 if(!j->synced&&j->matched){for(int i=0;i<j->count;){if(j->lines[i].word_count){i++;continue;}int first=i;while(i<j->count&&!j->lines[i].word_count)i++;int start=first?j->lines[first-1].words[j->lines[first-1].word_count-1].end_ms:0;int end=i<j->count?j->lines[i].time_ms:j->duration;
  if(end<start)end=start;for(int z=first;z<i;z++){j->lines[z].time_ms=start+(long long)(end-start)*(z-first)/(i-first);j->lines[z].estimated=1;}}
 }
 free(asr);return !cancelled(j)&&j->matched>0;
}
static char *cache_text(KaraokeJob*j){
 JSON_Value*v=json_value_init_object();JSON_Object*o=json_value_get_object(v);json_object_set_number(o,"format",2);json_object_set_number(o,"matched",j->matched);json_object_set_boolean(o,"synced",j->synced);
 JSON_Value*a=json_value_init_array();JSON_Array*rows=json_value_get_array(a);json_object_set_value(o,"lines",a);
 for(int i=0;i<j->count;i++){LyricLine*l=&j->lines[i];JSON_Value*vline=json_value_init_object();JSON_Object*line=json_value_get_object(vline);json_object_set_string(line,"text",l->text);json_object_set_number(line,"time",l->time_ms);json_object_set_boolean(line,"estimated",l->estimated);json_object_set_boolean(line,"aligned",l->aligned);JSON_Value*w=json_value_init_array();JSON_Array*items=json_value_get_array(w);json_object_set_value(line,"words",w);
  for(int k=0;k<l->word_count;k++){LyricWord*p=&l->words[k];JSON_Value*x=json_value_init_array();JSON_Array*y=json_value_get_array(x);json_array_append_number(y,p->time_ms);json_array_append_number(y,p->end_ms);json_array_append_number(y,p->offset);json_array_append_number(y,p->length);json_array_append_value(items,x);}json_array_append_value(rows,vline);
 }
 char *out=json_serialize_to_string(v);json_value_free(v);return out;
}
static bool read_cache(KaraokeJob*j){
 struct stat st;if(stat(j->cache,&st)||st.st_size<1||st.st_size>MAX_REPLY)return false;
 LyricLine *saved=malloc(j->count*sizeof(*saved));if(!saved)return false;memcpy(saved,j->lines,j->count*sizeof(*saved));
 JSON_Value*v=json_parse_file(j->cache);JSON_Object*o=json_value_get_object(v);JSON_Array*a=json_object_get_array(o,"lines");int format=json_object_get_number(o,"format");bool ok=(format==1||format==2)&&json_array_get_count(a)==(size_t)j->count;int matched=0,last=0;
 for(int i=0;ok&&i<j->count;i++){
  JSON_Object*row=json_array_get_object(a,i);const char*text=json_object_get_string(row,"text");JSON_Array*w=json_object_get_array(row,"words");size_t n=json_array_get_count(w);double time=json_object_get_number(row,"time");LyricLine*l=&j->lines[i];
  if(!text||strcmp(text,l->text)||n>LYRICS_MAX_WORDS||!isfinite(time)||time<last||time>j->duration){ok=false;break;}
  l->time_ms=(int)time;last=l->time_ms;l->word_count=0;l->estimated=json_object_get_boolean(row,"estimated")==1;l->aligned=format==2?json_object_get_boolean(row,"aligned")==1:!(saved[i].word_count&&!saved[i].estimated);
  for(size_t k=0;ok&&k<n;k++){JSON_Array*x=json_array_get_array(w,k);if(json_array_get_count(x)!=4){ok=false;break;}double d[4];for(int m=0;m<4;m++){d[m]=json_array_get_number(x,m);if(!isfinite(d[m])||d[m]<0||d[m]>MAX_AUDIO_MS+3000||floor(d[m])!=d[m])ok=false;}
   if(!ok)break;LyricWord p={(int)d[0],(int)d[1],(int)d[2],(int)d[3]};size_t len=strlen(text);if(p.time_ms<l->time_ms||p.end_ms<=p.time_ms||p.end_ms>j->duration+3000||p.offset+p.length>len||!p.length||(p.offset&&((unsigned char)text[p.offset]&0xc0)==0x80)||(p.offset+p.length<len&&((unsigned char)text[p.offset+p.length]&0xc0)==0x80)||(k&&p.time_ms<l->words[k-1].time_ms)){ok=false;break;}l->words[l->word_count++]=p;
  }if(l->word_count)matched++;
 }json_value_free(v);if(!ok){memcpy(j->lines,saved,j->count*sizeof(*saved));free(saved);return false;}
 /* A timing cache can fill holes, but cannot override current Enhanced LRC. */
 for(int i=0;i<j->count;i++)if(saved[i].word_count&&!saved[i].estimated&&!saved[i].aligned)j->lines[i]=saved[i];
 free(saved);j->matched=matched;return matched>0;
}
static void put16(unsigned char*p,unsigned v){p[0]=v;p[1]=v>>8;}static void put32(unsigned char*p,unsigned v){for(int i=0;i<4;i++)p[i]=v>>(8*i);}
static bool audio_file(KaraokeJob*j){
 snprintf(j->wav,sizeof(j->wav),"/tmp/nxt-karaoke-XXXXXX.wav");int fd=mkstemps(j->wav,4);if(fd<0)return false;fcntl(fd,F_SETFD,FD_CLOEXEC);FILE*f=fdopen(fd,"wb");if(!f){close(fd);return false;}
 unsigned char header[44]={0};memcpy(header,"RIFF",4);memcpy(header+8,"WAVEfmt ",8);put32(header+16,16);put16(header+20,1);put16(header+22,1);put32(header+24,16000);put32(header+28,32000);put16(header+32,2);put16(header+34,16);memcpy(header+36,"data",4);bool ok=fwrite(header,1,44,f)==44;
 NxtDecoder*d=ok?NxtDecoder_open(j->path,16000):NULL;if(!d)ok=false;float pcm[2048*2];unsigned char mono[2048*2];unsigned frames=0;
 while(ok&&wait_visible(j)){
  int n=NxtDecoder_read(d,pcm,2048);if(n<0){ok=false;break;}if(!n)break;if(n>2048||frames+(unsigned)n>16000u*600u){ok=false;break;}
  for(int i=0;i<n;i++){float x=(pcm[i*2]+pcm[i*2+1])*.5f;if(!isfinite(x))x=0;if(x>1)x=1;if(x< -1)x=-1;put16(mono+i*2,(unsigned)(int16_t)lrintf(x*32767));}
  if(fwrite(mono,2,n,f)!=(size_t)n){ok=false;break;}frames+=n;usleep(8000);
 }if(cancelled(j)||frames<160)ok=false;if(d)NxtDecoder_close(d);
 if(ok){put32(header+4,36+frames*2);put32(header+40,frames*2);if(fseek(f,0,SEEK_SET)||fwrite(header,1,44,f)!=44)ok=false;}if(fclose(f))ok=false;return ok;
}
static char *transport(KaraokeJob*j,int *code){
 int in[2],out[2];if(pipe2(in,O_CLOEXEC))return NULL;if(pipe2(out,O_CLOEXEC)){close(in[0]);close(in[1]);return NULL;}
 for(int *p=in;p<in+2;p++)if(*p<3){int n=fcntl(*p,F_DUPFD_CLOEXEC,3);close(*p);*p=n;}
 for(int *p=out;p<out+2;p++)if(*p<3){int n=fcntl(*p,F_DUPFD_CLOEXEC,3);close(*p);*p=n;}
 posix_spawn_file_actions_t act;int error=posix_spawn_file_actions_init(&act);pid_t pid=-1;
 if(!error){error=posix_spawn_file_actions_adddup2(&act,in[0],0);if(!error)error=posix_spawn_file_actions_adddup2(&act,out[1],1);if(!error)error=posix_spawn_file_actions_addopen(&act,2,"/dev/null",O_WRONLY,0);
  DIR*fds=opendir("/proc/self/fd");if(!fds)error=EINVAL;else{struct dirent*e;int own=dirfd(fds);while(!error&&(e=readdir(fds))){char*end;long fd=strtol(e->d_name,&end,10);if(!*end&&fd>=3&&fd!=own)error=posix_spawn_file_actions_addclose(&act,(int)fd);}closedir(fds);}
  char*argv[]={"curl","-sS","--proto","=https","--cacert","./res/cacert.pem","--connect-timeout","5","--max-time","60","--max-redirs","0","-w","\nNXT_STATUS:%{http_code}","-K","-",NULL};
  if(!error)error=posix_spawn(&pid,j->curl,&act,NULL,argv,j->env);posix_spawn_file_actions_destroy(&act);
 }close(in[0]);close(out[1]);if(error){close(in[1]);close(out[0]);return NULL;}
 char cfg[1600];int len=snprintf(cfg,sizeof(cfg),"url = \"https://api.groq.com/openai/v1/audio/transcriptions\"\nheader = \"Authorization: Bearer %s\"\nform = \"file=@%s;filename=audio.wav;type=audio/wav\"\nform = \"model=whisper-large-v3\"\nform = \"response_format=verbose_json\"\nform = \"timestamp_granularities[]=word\"\nform = \"temperature=0\"\n",j->key,j->wav);
 ssize_t sent=write(in[1],cfg,len);memset(cfg,0,sizeof(cfg));close(in[1]);fcntl(out[0],F_SETFL,O_NONBLOCK);char*body=calloc(MAX_REPLY+1,1);size_t used=0;bool eof=false,reaped=false,ok=body&&sent==len;int status=0;struct timespec began;clock_gettime(CLOCK_MONOTONIC,&began);
 while(ok&&(!eof||!reaped)){
  struct timespec now;clock_gettime(CLOCK_MONOTONIC,&now);if(cancelled(j)||now.tv_sec-began.tv_sec>62){ok=false;break;}
  struct pollfd p={out[0],POLLIN|POLLHUP,0};poll(&p,1,40);
  for(;;){char block[4096];ssize_t n=read(out[0],block,sizeof(block));if(n==0){eof=true;break;}if(n<0){if(errno!=EAGAIN&&errno!=EINTR)ok=false;break;}if(used+(size_t)n>MAX_REPLY){ok=false;break;}memcpy(body+used,block,n);used+=n;body[used]=0;}
  if(!reaped){pid_t w=waitpid(pid,&status,WNOHANG);if(w==pid)reaped=true;else if(w<0&&errno!=EINTR){reaped=true;ok=false;}}
 }close(out[0]);if(!reaped){kill(pid,SIGKILL);while(waitpid(pid,&status,0)<0&&errno==EINTR){}}
 if(!ok||!WIFEXITED(status)||WEXITSTATUS(status)){free(body);return NULL;}char*mark=strrchr(body,'\n');if(!mark||strncmp(mark,"\nNXT_STATUS:",12)){free(body);return NULL;}*code=atoi(mark+12);*mark=0;return body;
}
static void prune(const char*cache,size_t incoming){
 char dir[1400];snprintf(dir,sizeof(dir),"%s",cache);char*slash=strrchr(dir,'/');if(!slash)return;*slash=0;DIR*d=opendir(dir);if(!d)return;
 struct {char name[80];off_t bytes;time_t modified;} entries[257];int n=0;off_t bytes=0;struct dirent*e;
 while((e=readdir(d))&&n<257){if(strlen(e->d_name)!=69||strcmp(e->d_name+64,".json"))continue;char path[1500];snprintf(path,sizeof(path),"%s/%s",dir,e->d_name);struct stat st;if(lstat(path,&st)||!S_ISREG(st.st_mode))continue;snprintf(entries[n].name,sizeof(entries[n].name),"%s",e->d_name);entries[n].bytes=st.st_size;entries[n].modified=st.st_mtime;bytes+=st.st_size;n++;}closedir(d);
 while(n>0&&(n>=256||bytes+(off_t)incoming>CACHE_BYTES)){int oldest=0;for(int i=1;i<n;i++)if(entries[i].modified<entries[oldest].modified)oldest=i;char path[1500];snprintf(path,sizeof(path),"%s/%s",dir,entries[oldest].name);unlink(path);bytes-=entries[oldest].bytes;entries[oldest]=entries[--n];}
}
static void *run(void*ptr){
 KaraokeJob*j=ptr;setpriority(PRIO_PROCESS,(id_t)syscall(SYS_gettid),19);int result=K_IDLE;
 if(!wait_visible(j))result=K_CANCEL;
 else if(!j->refresh&&read_cache(j))result=K_CACHE;
 else if(j->upload){
  atomic_store(&state,K_READ);
  if(!audio_file(j))result=cancelled(j)?K_CANCEL:K_ERROR;
  else if(wait_visible(j)){atomic_store(&state,K_UPLOAD);int code=0;char*reply=transport(j,&code);
   if(cancelled(j))result=K_CANCEL;else if(code==429)result=K_QUOTA;else if(code==401||code==403)result=K_KEY;else if(reply&&code==200&&align_words(j,reply)){
    if(!cancelled(j)){char*text=cache_text(j);if(text){prune(j->cache,strlen(text));char tmp[1450];snprintf(tmp,sizeof(tmp),"%s.part",j->cache);int fd=open(tmp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);if(fd>=0){size_t n=strlen(text),at=0;while(at<n){ssize_t w=write(fd,text+at,n-at);if(w<=0)break;at+=w;}close(fd);if(at==n&&!cancelled(j))rename(tmp,j->cache);unlink(tmp);}json_free_serialized_string(text);}result=K_READY;}
   }else result=reply&&code==200?K_NOMATCH:K_ERROR;free(reply);
  }else result=K_CANCEL;
 }if(j->wav[0])unlink(j->wav);memset(j->key,0,sizeof(j->key));if(cancelled(j))result=K_CANCEL;atomic_store(&state,result);atomic_store(&done,true);return NULL;
}
static void free_job(KaraokeJob*j){if(!j)return;free(j->lines);if(j->env){for(int i=0;j->env[i];i++)free(j->env[i]);free(j->env);}memset(j->key,0,sizeof(j->key));free(j);}
static bool snapshot(KaraokeJob*j){
 int n=0;size_t size=0;while(environ[n]){size+=strlen(environ[n])+1;if(n>=2048||size>256*1024)return false;n++;}j->env=calloc(n+1,sizeof(char*));if(!j->env)return false;for(int i=0;i<n;i++)if(!(j->env[i]=strdup(environ[i])))return false;
 char *paths=strdup(getenv("PATH")?getenv("PATH"):"/usr/bin:/bin");if(!paths)return false;char *save=NULL;for(char*p=strtok_r(paths,":",&save);p;p=strtok_r(NULL,":",&save)){char path[1400];snprintf(path,sizeof(path),"%s/curl",p);if(!access(path,X_OK)){char*abs=realpath(path,NULL);if(abs){snprintf(j->curl,sizeof(j->curl),"%s",abs);free(abs);break;}}}free(paths);return j->curl[0]!=0;
}
void NxtKaraoke_tick(void){
 if(!initialized){initialized=true;signal(SIGPIPE,SIG_IGN);}atomic_store(&paused,!visible());unsigned play=NxtPlayback_generation(),rev=Lyrics_revision();
 if(play!=seen_playback){atomic_fetch_add(&generation,1);have_hash=false;seen_revision=0;seen_playback=play;force=false;}
 if(job){if(!atomic_load(&done))return;pthread_join(worker,NULL);int result=atomic_load(&state);
  bool applied=false;if((result==K_READY||result==K_CACHE)&&job->playback==play&&job->revision==rev&&!cancelled(job))applied=Lyrics_applyWordTimings(job->revision,job->lines,job->count,job->synced);
  unsigned attempted=job->revision;free_job(job);job=NULL;seen_revision=applied?Lyrics_revision():attempted==rev?rev:0;return;
 }
 if(!visible()||(!force&&rev==seen_revision))return;seen_revision=rev;int count=Lyrics_lineCount();if(count<1||count>LYRICS_MAX_LINES)return;
 const char*path=NxtPlayback_path();struct stat st;if(!path||NxtDSD_isPath(path)||stat(path,&st)||!S_ISREG(st.st_mode)){atomic_store(&state,K_UNSUPPORTED);force=false;return;}
 int already=0;for(int i=0;i<count;i++){LyricLine line;if(Lyrics_copyLine(i,&line)&&line.word_count&&!line.estimated)already++;}if(already==count&&!force){atomic_store(&state,K_READY);force=false;return;}
 int duration=NxtPlayback_duration();if(duration<10||duration>MAX_AUDIO_MS){atomic_store(&state,K_UNSUPPORTED);force=false;return;}
 mbedtls_sha256_context ctx;mbedtls_sha256_init(&ctx);mbedtls_sha256_starts(&ctx,0);mbedtls_sha256_update(&ctx,(const unsigned char*)path,strlen(path));mbedtls_sha256_update(&ctx,(const unsigned char*)&st.st_size,sizeof(st.st_size));mbedtls_sha256_update(&ctx,(const unsigned char*)&st.st_mtime,sizeof(st.st_mtime));
 for(int i=0;i<count;i++){const char*t=Lyrics_lineText(i);if(t)mbedtls_sha256_update(&ctx,(const unsigned char*)t,strlen(t)+1);}unsigned char hash[32];mbedtls_sha256_finish(&ctx,hash);mbedtls_sha256_free(&ctx);if(have_hash&&!memcmp(hash,last_hash,32)&&!force)return;memcpy(last_hash,hash,32);have_hash=true;
 KaraokeJob*j=calloc(1,sizeof(*j));if(!j){force=false;return;}j->lines=calloc(count,sizeof(LyricLine));if(!j->lines){free_job(j);force=false;return;}
 j->generation=atomic_load(&generation);j->revision=rev;j->playback=play;j->count=count;j->duration=duration;j->synced=Lyrics_isSynced();j->upload=force||nxt.karaoke_auto;j->refresh=force;force=false;snprintf(j->path,sizeof(j->path),"%s",path);
 for(int i=0;i<count;i++){Lyrics_copyLine(i,&j->lines[i]);if(j->refresh&&j->lines[i].aligned){j->lines[i].word_count=0;j->lines[i].estimated=0;}}
 if(j->revision!=Lyrics_revision()){free_job(j);seen_revision=0;return;}
 char dir[1300],name[65];userdata_snpath("karaoke-cache",dir,sizeof(dir));mkdir(dir,0700);for(int i=0;i<32;i++)snprintf(name+i*2,3,"%02x",hash[i]);snprintf(j->cache,sizeof(j->cache),"%s/%s.json",dir,name);
 if(j->upload&&(!NxtTranslate_copyGroqKey(j->key,sizeof(j->key))||!snapshot(j))){free_job(j);atomic_store(&state,K_KEY);return;}
 atomic_store(&done,false);atomic_store(&state,K_IDLE);job=j;if(pthread_create(&worker,NULL,run,j)){job=NULL;free_job(j);atomic_store(&state,K_ERROR);}
}
void NxtKaraoke_quit(void){atomic_fetch_add(&generation,1);atomic_store(&paused,false);if(job){pthread_join(worker,NULL);free_job(job);job=NULL;}force=false;have_hash=false;seen_revision=0;initialized=false;}
const char *NxtKaraoke_status(void){
 switch(atomic_load(&state)){case K_READ:return Nxt_text("Preparing audio quietly…","Đang chuẩn bị âm thanh ở ưu tiên thấp…");case K_UPLOAD:return Nxt_text("Groq is finding word times…","Groq đang nhận diện mốc từng chữ…");case K_READY:case K_CACHE:return Nxt_text("AI word times ready · check singing alignment","Có mốc chữ AI · kiểm tra độ khớp giọng hát");case K_NOMATCH:return Nxt_text("Voice text did not match · original lyrics retained","Nhận diện không khớp lời · giữ lời gốc");case K_ERROR:return Nxt_text("Could not analyse audio · retry manually","Chưa phân tích được âm thanh · thử lại thủ công");case K_CANCEL:return Nxt_text("Cancelled; original lyrics retained","Đã hủy; giữ lời gốc");case K_UNSUPPORTED:return Nxt_text("AI timing: local audio up to 10 minutes only","Mốc AI: chỉ tệp nhạc trên thẻ tối đa 10 phút");case K_KEY:return Nxt_text("Check your Groq key and curl connection","Kiểm tra khóa Groq và kết nối curl");case K_QUOTA:return Nxt_text("Groq quota reached · retry later","Groq hết hạn mức · thử lại sau");default:return Nxt_text("Original word timings when available","Dùng mốc từng chữ gốc nếu có");}
}
static bool consent(DisplayContext*d){
 int selected=0;const char*rows[]={Nxt_text("Cancel · keep local lyrics","Hủy · giữ lời hiện có"),Nxt_text("Agree · send audio to Groq","Đồng ý · gửi âm thanh tới Groq")};
 const char *doc[]={Nxt_text("This optional feature uploads this song's audio to Groq using your key and Whisper Large v3. It uses Groq audio quota, separate from text translation. Account billing depends on your Groq plan.","Tính năng tùy chọn gửi âm thanh bài hát tới Groq bằng khóa của bạn và Whisper Large v3. Dùng hạn mức âm thanh riêng với dịch chữ; chi phí phụ thuộc gói Groq của bạn."),Nxt_text("AI recognition can miss singing, accents or backing vocals. Only matching words get timings; unmatched lyrics remain. No guarantee for every song. Results are cached for offline use. Local files only, up to 10 minutes. Protected online streams are not recorded.","AI có thể nhận sai tiếng hát, phát âm hoặc bè. Chỉ ghép mốc khi khớp chữ; giữ phần lời chưa khớp. Không đảm bảo mọi bài. Lưu mốc để dùng offline. Chỉ tệp trên thẻ, tối đa 10 phút; không ghi lại luồng nhạc online được bảo vệ.")};
 if(Nxt_document(d,Nxt_text("Before enabling AI karaoke","Trước khi bật karaoke AI"),doc,2)==MODULE_EXIT_QUIT)return false;
 NxtPick p=Nxt_pick(d,Nxt_text("Send audio to Groq?","Gửi âm thanh tới Groq?"),rows,2,&selected,Nxt_text("A Select · B Cancel","A Chọn · B Hủy"));return p.action==1&&selected==1;
}
ModuleExitReason NxtKaraoke_settings(DisplayContext*d){
 int selected=0;for(;;){char status[260],mode[160];snprintf(status,sizeof(status),"%s",NxtKaraoke_status());snprintf(mode,sizeof(mode),"%s\t%s",Nxt_text("Automatic local-song analysis","Tự phân tích bài trên thẻ"),nxt.karaoke_auto?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));
 char fallback[180];snprintf(fallback,sizeof(fallback),"%s\t%s",Nxt_text("Estimate missing word times offline","Ước lượng mốc chữ còn thiếu offline"),nxt.karaoke_estimate?Nxt_text("On · labelled estimated","Bật · có nhãn ước lượng"):Nxt_text("Off","Tắt"));
 const char*rows[]={status,Nxt_text("Create word timing for this song","Tạo mốc từng chữ cho bài này"),mode,Nxt_text("Cancel current analysis","Hủy phân tích đang chạy"),Nxt_text("Use word-timed LRC offline","Dùng LRC có mốc chữ offline"),fallback};
 NxtPick p=Nxt_pickLive(d,Nxt_text("Voice-timed karaoke · Groq","Karaoke theo giọng hát · Groq"),rows,6,&selected,Nxt_text("A Select · B Back","A Chọn · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
 if(selected==1){if(consent(d)){force=true;seen_revision=0;NxtKaraoke_tick();}}
 else if(selected==2){if(nxt.karaoke_auto)nxt.karaoke_auto=0;else if(consent(d)){nxt.karaoke_auto=1;have_hash=false;seen_revision=0;}Nxt_save();}
 else if(selected==3){atomic_fetch_add(&generation,1);force=false;}
 else if(selected==5){nxt.karaoke_estimate=!nxt.karaoke_estimate;Nxt_save();}
 else if(selected==4){const char*doc[]={Nxt_text("A same-name UTF-8 .lrc beside the song can contain real word timestamps: [00:10.000]<00:10.000>Hello <00:10.600>world<00:11.400>. This works offline without sending audio. Times must match the exact recording. Plain line timestamps cannot reveal when each word was sung.","Đặt .lrc UTF-8 cùng tên cạnh bài nhạc, có mốc chữ thật: [00:10.000]<00:10.000>Hello <00:10.600>world<00:11.400>. Dùng offline, không gửi âm thanh. Mốc phải khớp đúng bản thu. Chỉ mốc theo câu không cho biết thời điểm từng chữ.")};if(Nxt_document(d,Nxt_text("Word timing","Mốc từng chữ"),doc,1)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
 }
}
