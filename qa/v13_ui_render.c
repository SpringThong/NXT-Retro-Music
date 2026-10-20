#define main unused_spotify_render_main
#include "spotify_render.c"
#undef main
#include "nxt_controls.h"
#include "nxt_led.h"
static SDL_Surface *page;
static int pressed_keys,released_keys,frame_no,device_exports;
static const char *out_dir,*screen_name;
int PAD_isPressed(int b){return pressed_keys&b;}
int PAD_justPressed(int b){return pressed_keys&b;}
int PAD_justRepeated(int b){return pressed_keys&b;}
int PAD_justReleased(int b){return released_keys&b;}
int PWR_displayInputConsumed(void){return false;}
void ModuleCommon_frameBegin(void){assert(++frame_no<12);pressed_keys=frame_no==3?BTN_B:frame_no==2&&screen_name&&!strcmp(screen_name,"about")?BTN_Y:0;released_keys=0;ticks+=16;}
void Background_tick(void){}
bool ModuleCommon_stopSignalled(void){return false;}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface *s,int *setting,HelpId help){return(GlobalInputResult){0};}
SDL_Surface *DisplayHelper_getSurface(const DisplayContext *d){return page;}
void ModuleCommon_frameEnd(SDL_Surface *s){if(frame_no==1){char path[1400];snprintf(path,sizeof(path),"%s/%s-%d.bmp",out_dir,screen_name,page->w);assert(!SDL_SaveBMP(s,path));}}
void ModuleCommon_markSurfaceDrawn(void){}
void ModuleCommon_PWR_update(int *dirty,int *show){}
bool Nxt_save(void){return true;}
bool NxtHardware_writeDiagnostic(void){device_exports++;return true;}
void Lyrics_clear(void){}
bool Lyrics_loadTextFile(const char *p,int ms){return true;}
bool Lyrics_isSynced(void){return true;}
void NxtRecord_hide(void){}
void NxtRecord_release(void){}
bool NxtDrive_isPath(const char *p){return false;}
const char *NxtHardware_bottomUSBStatus(void){return "Stock USB";}
int GetVolume(void){return 10;}
int GetBrightness(void){return 5;}

static void catalog_page(int page,int count,bool more,bool duplicate){
 char json[18000];size_t n=snprintf(json,sizeof(json),"{\"event\":\"results\",\"kind\":\"search\",\"offset\":%d,\"next_offset\":%d,\"has_next\":%s,\"items\":[",page,page+count,more?"true":"false");
 for(int i=0;i<count;i++){int id=page+i-(duplicate?1:0);n+=snprintf(json+n,sizeof(json)-n,"%s{\"title\":\"Track %d\",\"uri\":\"catalog:track:%d\",\"id\":\"%d\"}",i?",":"",id,id,id);assert(n<sizeof(json)-3);}
 strcpy(json+n,"]}");event(json);
}
static void catalog_regression(void){
 queue_request=false;result_count=0;for(int page=0;page<80;page+=20){catalog_page(page,20,true,false);assert(result_count==page+20);}
 capture_queue("search");assert(queue_count==80&&queue_more&&queue_offset==80);
 nxt.repeat_mode=2;queue_index=79;assert(adjacent(1)==-1);
 queue_request=true;catalog_page(80,2,false,true);assert(queue_count==81&&!queue_more&&adjacent(1)==80);
 queue_index=80;assert(adjacent(1)==0);nxt.repeat_mode=0;
 free(queue);queue=NULL;queue_count=queue_capacity=0;free(results);results=NULL;result_count=result_capacity=0;queue_more=has_next=false;
 puts("PASS append 80 catalog results, listening queue grows to 81 with deduplication, no early repeat while more pages remain, final repeat-all wraps only at actual catalog end.");
}


static void pairing_retry(void){
 pairing=pair_finishing=true;pair_done=pair_failed=false;connected=false;auth_required=false;
 event("{\"event\":\"logged_in\",\"display_name\":\"QA\",\"playback_ready\":true,\"library_ready\":false,\"library_warning\":\"rate_limited:2\"}");
 assert(connected&&!login_retry_at&&pair_done&&!pairing&&!pair_finishing&&strstr(status,"Thư viện"));
 pairing=pair_finishing=true;pair_failed=false;
 event("{\"event\":\"error\",\"operation\":\"login\",\"reason\":\"playback_auth_rejected\"}");
 assert(!connected&&!auth_required&&!pairing&&!pair_finishing&&pair_failed&&!login_retry_at);
 puts("PASS QR playback success is independent of library limits; real playback rejection exits pairing without an endless login retry.");
}

static void controls(void){
 for(int a=1;a<NC_COUNT;a++)for(int i=0;i<16;i++){
  memset(nxt.playing_keys,0,sizeof(nxt.playing_keys));nxt.playing_keys[i]=a;
  pressed_keys=Nxt_keyIds[i];released_keys=0;
  assert(NxtControl_held(a));
  if(i==14){assert(!NxtControl_pressed(a));pressed_keys=0;released_keys=BTN_SELECT;assert(NxtControl_pressed(a));}
  else assert(NxtControl_pressed(a)&&NxtControl_repeated(a));
  assert(!NxtControl_pressed(a==1?2:1));
 }
 NxtControl_defaults();assert(nxt.playing_keys[4]==NC_PLAY&&nxt.playing_keys[14]==NC_EQ&&nxt.playing_keys[15]==NC_QUICK);
}
static void show(ModuleExitReason (*fn)(DisplayContext *),const char *name){frame_no=pressed_keys=released_keys=0;screen_name=name;assert(fn((DisplayContext*)1)==MODULE_EXIT_TO_MENU);}
int main(int argc,char **argv){
 assert(argc==2&&!SDL_Init(0)&&!TTF_Init());out_dir=argv[1];nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;
 catalog_regression();pairing_retry();controls();nxt.led=0;nxt.led_mode=1;nxt.led_color=16;nxt.led_rgb=0x123456;nxt.led_strength=20;
 for(int i=0;i<2;i++){
  page=SDL_CreateRGBSurfaceWithFormat(0,i?640:1024,i?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);
  show(Nxt_help,"usage");show(Nxt_changes,"changes");show(Nxt_about,"about");show(NxtControl_settings,"controls");show(NxtLED_palette,"led");
  assert(nxt.led==0&&nxt.led_mode==1&&nxt.led_rgb==0x123456&&nxt.led_strength==20);
  SDL_FreeSurface(page);
 }
 assert(device_exports==2);puts("PASS About Y exports device diagnostics at both screen sizes.");
 NxtSkin_release();TTF_Quit();SDL_Quit();puts("PASS production v1.3 guide/change log/About/control/RGB screens at 640 and 1024, all 21 actions on 16 physical buttons, reset defaults, canceled LED preview restoration, ASan/UBSan.");return 0;
}
