/* Actual karaoke worker/mapping/cache/transport, fake PCM and HTTP endpoint. */
#define NXT_PLAYBACK_H
unsigned NxtPlayback_generation(void);int NxtPlayback_duration(void);const char*NxtPlayback_path(void);
#include "../upstream/src/nxt_karaoke.c"
#include <assert.h>
NxtConfig nxt;static unsigned playback=1,revision=1;static int applied;static char audio_path[1024];static bool has_key=true,online;static int samples;static bool allow_audio;static int pick_step;
Uint32 SDL_GetTicks(void){return 1000;}
int PWR_getDisplayOff(void){return 0;}bool NxtSession_isHidden(void){return false;}
unsigned NxtPlayback_generation(void){return playback;}int NxtPlayback_duration(void){return 5000;}
const char*Player_getCurrentFile(void){return audio_path;}const char*NxtPlayback_path(void){return online?"spotify:track:fixture":audio_path;}
bool Lyrics_copyLine(int i,LyricLine*out){memset(out,0,sizeof(*out));out->time_ms=1000;strcpy(out->text,"Hello world");return true;}
unsigned Lyrics_revision(void){return revision;}int Lyrics_lineCount(void){return 1;}bool Lyrics_isSynced(void){return true;}const char*Lyrics_lineText(int i){return "Hello world";}int Lyrics_wordCount(int i){return 0;}int Lyrics_lineTime(int i){return 1000;}const LyricWord*Lyrics_wordAt(int i,int k){return NULL;}
bool Lyrics_applyWordTimings(unsigned rev,const LyricLine*lines,int count,bool sync){assert(rev==revision&&count==1&&sync&&lines[0].word_count==2);applied++;revision++;return true;}
bool NxtDSD_isPath(const char*path){return false;}
bool NxtTranslate_copyGroqKey(char*out,size_t size){snprintf(out,size,"TEST_ONLY_KEY");return has_key;}
const char*Nxt_text(const char*a,const char*b){return a;}
int userdata_snpath(const char*rel,char*out,size_t size){return snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
struct NxtDecoder{int unused;};static struct NxtDecoder decoder;
NxtDecoder*NxtDecoder_open(const char*path,int rate){assert(rate==16000);samples=0;return &decoder;}
int NxtDecoder_read(NxtDecoder*d,float*out,int frames){if(samples>=16000)return 0;int n=16000-samples;if(n>frames)n=frames;for(int i=0;i<n*2;i++)out[i]=.25f;samples+=n;return n;}
void NxtDecoder_close(NxtDecoder*d){assert(d==&decoder);}
NxtPick Nxt_pick(DisplayContext*d,const char*title,const char**rows,int n,int*selected,const char*hint){assert(n==2&&*selected==0);*selected=allow_audio?1:0;return(NxtPick){1,*selected};}
NxtPick Nxt_pickLive(DisplayContext*d,const char*title,const char**rows,int n,int*selected,const char*hint){assert(n==6);*selected=2;return(NxtPick){pick_step++?2:1,*selected};}
ModuleExitReason Nxt_document(DisplayContext*d,const char*title,const char*const*rows,int n){assert(n==2&&strstr(rows[0],"uploads")&&strstr(rows[0],"Groq"));return MODULE_EXIT_TO_MENU;}
bool Nxt_save(void){return true;}
static const char *reply="{\"words\":[{\"word\":\"Hello\",\"start\":1,\"end\":1.4},{\"word\":\" world\",\"start\":1.5,\"end\":2}]}";
static void wait_job(void){for(int i=0;job&&i<1000;i++){usleep(10000);NxtKaraoke_tick();}assert(!job);}
static void mapping(void){
 KaraokeJob j={.count=1,.duration=5000,.synced=true,.generation=1};j.lines=calloc(1,sizeof(LyricLine));j.lines[0].time_ms=1000;strcpy(j.lines[0].text,"Hello, world!");
 assert(align_words(&j,reply)&&j.matched==1&&j.lines[0].word_count==2);assert(j.lines[0].words[0].offset==0&&j.lines[0].words[0].length==5&&j.lines[0].words[1].offset==7&&j.lines[0].words[1].end_ms==2000);
 char path[1400];snprintf(path,sizeof(path),"%s/test.json",getenv("QA_DATA"));snprintf(j.cache,sizeof(j.cache),"%s",path);char*json=cache_text(&j);FILE*f=fopen(path,"w");assert(f);fputs(json,f);fclose(f);json_free_serialized_string(json);memset(j.lines[0].words,0,sizeof(j.lines[0].words));j.lines[0].word_count=0;assert(read_cache(&j)&&j.lines[0].word_count==2);
 /* A valid AI cache must not replace real Enhanced-LRC intervals. */
 j.lines[0].aligned=0;j.lines[0].words[0].end_ms=1450;LyricLine original=j.lines[0];assert(read_cache(&j)&&!memcmp(&original,j.lines,sizeof(original)));
 LyricLine saved=j.lines[0];f=fopen(path,"w");fputs("{\"format\":1,\"lines\":[{\"text\":\"Hello, world!\",\"time\":4500,\"words\":[[4500,4800,999,20]]}]}",f);fclose(f);assert(!read_cache(&j)&&!memcmp(&saved,j.lines,sizeof(saved)));unlink(path);
 j.lines[0].word_count=0;j.matched=0;strcpy(j.lines[0].text,"Different words");assert(!align_words(&j,reply)&&!j.lines[0].word_count);
 strcpy(j.lines[0].text,"Hello world");assert(!align_words(&j,"{\"words\":[{\"word\":\"Hello\",\"end\":1}]}"));assert(!align_words(&j,"{\"words\":[{\"word\":\"Hello\",\"start\":4,\"end\":3}]}"));
 j.synced=false;j.lines[0].time_ms=0;j.matched=0;assert(align_words(&j,reply)&&j.lines[0].time_ms==1000);
 memset(j.lines,0,sizeof(*j.lines));strcpy(j.lines[0].text,"你好，世界");j.matched=0;assert(align_words(&j,"{\"words\":[{\"word\":\"你好\",\"start\":1,\"end\":2},{\"word\":\"世界\",\"start\":2,\"end\":3}]}"));assert(j.lines[0].word_count==4&&j.lines[0].words[2].offset==strlen("你好，")&&j.lines[0].words[2].length==strlen("世")&&j.lines[0].estimated);
 memset(j.lines,0,sizeof(*j.lines));j.synced=true;j.matched=0;j.lines[0].time_ms=1000;strcpy(j.lines[0].text,"Đường về nhà hôm nay");
 assert(align_words(&j,"{\"words\":[{\"word\":\"noise\",\"start\":0,\"end\":0},{\"word\":\"duong\",\"start\":1,\"end\":1.5},{\"word\":\"ve\",\"start\":1.5,\"end\":2},{\"word\":\"nha\",\"start\":2,\"end\":2.5},{\"word\":\"nay\",\"start\":3,\"end\":3.5}]}")&&j.lines[0].word_count==5&&j.lines[0].estimated);
 assert(j.lines[0].words[0].length==strlen("Đường")&&j.lines[0].words[3].time_ms>=2500&&j.lines[0].words[3].end_ms<=3000);
 free(j.lines);
}
int main(int argc,char**argv){assert(argc==2);snprintf(audio_path,sizeof(audio_path),"%s/song.wav",argv[1]);FILE*f=fopen(audio_path,"w");assert(f);fputs("fixture",f);fclose(f);mapping();
 allow_audio=false;pick_step=0;assert(NxtKaraoke_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.karaoke_auto==0);
 allow_audio=true;pick_step=0;assert(NxtKaraoke_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.karaoke_auto==1);nxt.karaoke_auto=0;
 KaraokeJob a={.generation=1};assert(audio_file(&a));f=fopen(a.wav,"rb");unsigned char h[44];assert(fread(h,1,44,f)==44);fclose(f);assert(!memcmp(h,"RIFF",4)&&!memcmp(h+8,"WAVE",4)&&h[22]==1&&h[24]==0x80&&h[25]==0x3e&&h[40]==0&&h[41]==0x7d);unlink(a.wav);
 nxt.karaoke_auto=0;NxtKaraoke_tick();wait_job();assert(applied==0); /* Off = cache lookup only, no upload. */
 force=true;NxtKaraoke_tick();wait_job();assert(applied==1&&atomic_load(&state)==K_READY);
 playback++;has_key=false;NxtKaraoke_tick();wait_job();assert(applied==2&&atomic_load(&state)==K_CACHE); /* Offline timing load requires no API key. */
 setenv("QA_MODE","quota",1);has_key=true;force=true;NxtKaraoke_tick();wait_job();assert(atomic_load(&state)==K_QUOTA&&applied==2);setenv("QA_MODE","ok",1); /* manual refresh bypasses a valid cache */
 playback++;force=true;has_key=true;NxtKaraoke_tick();if(job){atomic_fetch_add(&generation,1);wait_job();}assert(applied==2);NxtKaraoke_quit();
 /* Distinct content/stat fingerprint: force a cache miss for HTTP failures. */
 f=fopen(audio_path,"a");assert(f);fputs("-changed",f);fclose(f);NxtKaraoke_tick();wait_job();
 setenv("QA_MODE","quota",1);force=true;NxtKaraoke_tick();wait_job();assert(atomic_load(&state)==K_QUOTA&&applied==2);
 for(int i=0;i<20;i++){NxtKaraoke_tick();assert(!job);} /* no retry storm */
 setenv("QA_MODE","oversize",1);force=true;NxtKaraoke_tick();wait_job();assert(atomic_load(&state)==K_ERROR&&applied==2);
 setenv("QA_MODE","slow",1);force=true;NxtKaraoke_tick();for(int i=0;i<300&&atomic_load(&state)!=K_UPLOAD;i++)usleep(10000);assert(job&&atomic_load(&state)==K_UPLOAD);atomic_fetch_add(&generation,1);wait_job();assert(atomic_load(&state)==K_CANCEL&&applied==2);NxtKaraoke_quit();
 online=true;playback++;revision++;NxtKaraoke_tick();assert(!job&&atomic_load(&state)==K_UNSUPPORTED&&applied==2);NxtKaraoke_quit();
 int status;assert(waitpid(-1,&status,WNOHANG)==-1&&errno==ECHILD);
 puts("PASS actual karaoke: partial normalized word alignment, CJK byte offsets, mismatch rejection, malformed cache rollback, mono 16k WAV, opt-in-only HTTP, worker publish, key-free offline cache and generation cancellation/reaping. PCM/HTTP are fixtures; no singing accuracy claim.");
}
