/* Actual production worker/curl/cache, with explicit lyrics/ROM fixtures. */
#include "../upstream/src/nxt_translate.c"
#include <assert.h>
#include <stdarg.h>
NxtConfig nxt;
static unsigned lyric_rev=1;
static int lyric_count=3,listener;
static bool hidden,screen_off;
static char text[LYRICS_MAX_LINES][2048];
unsigned Lyrics_revision(void){return lyric_rev;}
int Lyrics_lineCount(void){return lyric_count;}
const char *Lyrics_lineText(int i){return i>=0&&i<lyric_count?text[i]:NULL;}
int Lyrics_currentIndex(int ms){return 0;}
int Player_getPosition(void){return 500;}
bool NxtBit_ready(void){return false;}
bool NxtSpotify_hasMetadata(void){return false;}
bool NxtSoundCloud_hasMetadata(void){return false;}
int NxtSpotify_position(void){return 500;}
int NxtSoundCloud_position(void){return 500;}
int NxtPersonal_id(void){return listener;}
bool NxtSession_isHidden(void){return hidden;}
int PWR_getDisplayOff(void){return screen_off;}
int userdata_snpath(const char *rel,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),rel);}
bool Nxt_save(void){return true;}
const char *Nxt_text(const char *en,const char *vi){return nxt.language==1?vi:en;}
const char *NxtLocale_name(int i){return language_code(i);}
NxtPick Nxt_pickLive(DisplayContext*d,const char*a,const char**b,int n,int*s,const char*h){return(NxtPick){2,0};}
NxtPick Nxt_pick(DisplayContext*d,const char*a,const char**b,int n,int*s,const char*h){return(NxtPick){2,0};}
ModuleExitReason Nxt_document(DisplayContext*d,const char*t,const char*const*p,int n){return MODULE_EXIT_TO_MENU;}
char *Keyboard_open(const char*p,size_t n){return NULL;}
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
bool NxtStartup_preview(void){return false;}
int NxtStartup_position(void){return 500;}
static double clock_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000.+t.tv_nsec/1000000.;}
static void pump(int ms){double end=clock_ms()+ms;while(clock_ms()<end){NxtTranslate_tick();struct timespec t={0,10000000};nanosleep(&t,NULL);}}
static void control(const char *name){FILE*f=fopen(getenv("QA_CONTROL"),"w");assert(f);fprintf(f,"%s",name);assert(!fclose(f));}
static int requests(void){FILE*f=fopen(getenv("QA_REQUESTS"),"r");if(!f)return 0;int count=0,c;while((c=fgetc(f))!=EOF)if(c=='\n')count++;fclose(f);return count;}
static void ready(void){double end=clock_ms()+15000;while(ui_state!=ST_READY&&clock_ms()<end)pump(20);if(ui_state!=ST_READY)fprintf(stderr,"unexpected translation state=%d progress=%d/%d\n",ui_state,ui_done,ui_total);assert(ui_state==ST_READY);}
static void error_state(int state){double end=clock_ms()+5000;while(ui_state!=state&&clock_ms()<end)pump(20);assert(ui_state==state);}
static void original(const char *s){lyric_count=1;snprintf(text[0],sizeof(text[0]),"%s",s);lyric_rev++;}
int main(void){
 close(STDIN_FILENO); /* GUI launchers may start with stdin already closed. */
 nxt.language=1;nxt.translation_mode=0;
 snprintf(text[0],sizeof(text[0]),"The morning light is warm.");
 snprintf(text[1],sizeof(text[1]),"Today is a peaceful day.");snprintf(text[2],sizeof(text[2]),"%s",text[0]);
 control("ok");pump(80);assert(!started&&requests()==0&&!NxtTranslate_line(0));
 nxt.translation_mode=1;NxtTranslate_tick();ready();assert(requests()==2&&ui_done==3);assert(strstr(NxtTranslate_line(0),"vi:"));assert(!strcmp(NxtTranslate_line(0),NxtTranslate_line(2)));assert(!strcmp(Lyrics_lineText(0),"The morning light is warm."));
 unsigned rev=NxtTranslate_revision();pump(200);assert(rev==NxtTranslate_revision()&&requests()==2);
 /* Same source sequence reuses cache after quit/reopen, even with network down. */
 NxtTranslate_quit();control("network");lyric_rev++;NxtTranslate_tick();ready();assert(requests()==2);
 /* All 14 targets; exact source scripts for requested English/CJK languages. */
 control("ok");for(int lang=0;lang<14;lang++){
  nxt.language=lang;original("A new gentle melody.");NxtTranslate_tick();ready();const char*s=NxtTranslate_line(0);assert(s);
  if(lang==0)assert(!strcmp(s,text[0]));else assert(strstr(s,language_code(lang)));
 }
 nxt.language=1;const char *sources[]={"明天会更好。","오늘은 평화로운 날입니다.","明日は良い日です。"};
 const int expected[]={7,4,5};
 for(int i=0;i<3;i++){original(sources[i]);NxtTranslate_tick();ready();Job j={.count=1};j.input[0]=text[0];assert(guess(&j)==expected[i]);assert(strstr(NxtTranslate_line(0),"vi:"));}
 /* Explicit Latin source override. */
 nxt.translation_source=3;original("Une journée calme.");NxtTranslate_tick();ready();assert(strstr(NxtTranslate_line(0),"vi:"));nxt.translation_source=0;
 /* Split >480 UTF-8 bytes without chopping codepoints; dedup all repetitions. */
 char long_line[2048]="";for(int i=0;i<200;i++)strcat(long_line,"音楽");int before=requests();original(long_line);NxtTranslate_tick();ready();assert(requests()-before==3&&valid_text(NxtTranslate_line(0),4095));
 /* Network/quota cooldown survives track changes. Cached responses are unaffected. */
 control("quota");original("Quota sample one.");NxtTranslate_tick();error_state(ST_QUOTA);before=requests();assert(!NxtTranslate_line(0));
 original("Quota sample two.");NxtTranslate_tick();error_state(ST_QUOTA);pump(150);assert(requests()==before);
 control("ok");retry();ready();assert(requests()==before+1);
 control("malformed");original("Malformed reply.");NxtTranslate_tick();error_state(ST_NETWORK);assert(!strcmp(NxtTranslate_display(0),text[0]));
 control("ok");retry();ready();
 control("huge");original("Huge reply sample.");NxtTranslate_tick();error_state(ST_NETWORK);assert(!NxtTranslate_line(0));
 control("ok");retry();ready();
 /* Source replacement invalidates a slow old request. */
 control("slow");original("Old request that must not publish.");NxtTranslate_tick();before=requests();while(requests()==before)pump(20);
 control("ok");original("Replacement request.");assert(!NxtTranslate_line(0));NxtTranslate_tick();ready();assert(strstr(NxtTranslate_line(0),"Replacement request")&&!strstr(NxtTranslate_line(0),"Old request"));
 /* Hidden/screen-off means no new request, then resumes. */
 hidden=true;original("Hidden sample.");before=requests();pump(150);assert(requests()==before);hidden=false;NxtTranslate_tick();ready();
 screen_off=true;original("Screen-off sample.");before=requests();pump(150);assert(requests()==before);screen_off=false;NxtTranslate_tick();ready();
 /* Official Google protocol/key is on stdin, never in process argv. */
 provider=1;snprintf(service_key,sizeof(service_key),"FAKE_TEST_KEY_188");assert(save_service());service_revision++;
 original("Cloud sample & quote.");NxtTranslate_tick();ready();assert(strstr(NxtTranslate_line(0),"vi:"));
 char keypath[1400];service_path(keypath,sizeof(keypath));struct stat st;assert(!stat(keypath,&st)&&(st.st_mode&0777)==0600);
 control("googleerror");original("Cloud denied sample.");NxtTranslate_tick();error_state(ST_KEY);assert(!NxtTranslate_line(0));
 /* Quit cancels/reaps the active HTTP child quickly, preserving originals. */
 control("slow");atomic_store(&cooldown[1],0);original("Quit sample.");service_revision++;NxtTranslate_tick();before=requests();while(requests()==before)pump(20);
 double begin=clock_ms();NxtTranslate_quit();assert(clock_ms()-begin<700);
 assert(waitpid(-1,NULL,WNOHANG)==-1&&errno==ECHILD);
 puts("PASS production translation: default-off zero work, repeated-line dedup, cache/offline reuse, 14 targets, CJK script detection, source override, UTF-8 chunks, HTTP/JSON/oversize/quota errors, cooldown/manual retry, stale-result rejection, hidden/screen-off pause, Google protocol/private key and bounded quit/reap. Lyrics/audio inputs are fixtures.");
 return 0;
}

