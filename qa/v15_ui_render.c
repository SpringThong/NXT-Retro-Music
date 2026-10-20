#define main unused_spotify_render_main
#include "spotify_render.c"
#undef main
#include "nxt_controls.h"
#include "nxt_led.h"
ModuleExitReason FixtureSpotifySaved(DisplayContext *,const char *,const char *,const char *);
ModuleExitReason FixtureCloudSaved(DisplayContext *,const char *,const char *,const char *);
#define NxtSpotify_playSaved FixtureSpotifySaved
#define NxtSoundCloud_playSaved FixtureCloudSaved
#ifdef NXT_UI_V18
#undef ModuleCommon_frameBegin
#undef ModuleCommon_frameEnd
void ModuleCommon_frameBegin(void);void ModuleCommon_frameEnd(SDL_Surface*);
#include "nxt_personal.c"
#define ModuleCommon_frameBegin NxtFixture_frame_begin
#define ModuleCommon_frameEnd NxtFixture_frame_end
#else
#include "nxt_personal.c"
#endif
#undef NxtSpotify_playSaved
#undef NxtSoundCloud_playSaved
#include "nxt_appearance.h"
#include "ui_theme.h"

ModuleExitReason NxtOTA_run(DisplayContext *d){return MODULE_EXIT_TO_MENU;}
ModuleExitReason NxtOTA_guide(DisplayContext *d){return MODULE_EXIT_TO_MENU;}
void NxtLog_step(const char *c,const char *step,bool ok,const char *fmt,...){}
bool NxtLog_export(void){return true;}
void Nxt_loadPreferences(const char *path){}
bool mkdir_p(const char *path){char text[1500];snprintf(text,sizeof(text),"%s",path);for(char *p=text+1;*p;p++)if(*p=='/'){*p=0;mkdir(text,0700);*p='/';}return !mkdir(text,0700)||errno==EEXIST;}
bool userdata_mkdir(const char *rel){char path[1500];userdata_snpath(rel,path,sizeof(path));return mkdir_p(path);}
ModuleExitReason FixtureSpotifySaved(DisplayContext *d,const char *p,const char *t,const char *a){return MODULE_EXIT_TO_MENU;}
ModuleExitReason FixtureCloudSaved(DisplayContext *d,const char *p,const char *t,const char *a){return MODULE_EXIT_TO_MENU;}
bool Nxt_takeReturnHome(void){return false;}
void Nxt_stop(void){NxtPersonal_flush();fixture_state=PLAYER_STATE_STOPPED;}
void PlayerModule_quit(void){}
void Nxt_libraryQuit(void){}
void Fonts_unload(void){}void Fonts_load(void){}void Resume_init(void){}
bool Resume_isAvailable(void){return false;}
AudioFormat Player_detectFormat(const char *p){return AUDIO_FORMAT_MP3;}
ModuleExitReason PlayerModule_runResume(DisplayContext *d,const ResumeState *r){return MODULE_EXIT_TO_MENU;}
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext *d,PlaylistTrack *t,int n,int s){return MODULE_EXIT_TO_MENU;}
void PlayerModule_setResumePlaylistPath(const char *p){}
void Nxt_favorite(const char *p){}
static const char *typed_name;
char *Keyboard_open(const char *p,size_t n){return typed_name?strdup(typed_name):NULL;}
char *Keyboard_openLatin(const char *p,size_t n){return NULL;}
int IMG_SavePNG(SDL_Surface *s,const char *p){return -1;}
int NxtSort_compare(const char *a,const char *b){return strcmp(a,b);}
static SDL_Surface *page;
static int pressed_keys,released_keys,frame_no,device_exports;
static const char *out_dir,*screen_name;
int PAD_isPressed(int b){return pressed_keys&b;}
int PAD_justPressed(int b){return pressed_keys&b;}
int PAD_justRepeated(int b){return pressed_keys&b;}
int PAD_justReleased(int b){return released_keys&b;}
int PWR_displayInputConsumed(void){return false;}
void ModuleCommon_frameBegin(void){assert(++frame_no<12);int exit=screen_name&&!strcmp(screen_name,"personal-history")?4:3;pressed_keys=frame_no==exit?BTN_B:0;
 if(frame_no==2&&screen_name){if(!strcmp(screen_name,"about"))pressed_keys=BTN_Y;else if(!strcmp(screen_name,"personal-statistics")||!strcmp(screen_name,"personal-history"))pressed_keys=BTN_R1;else if(!strcmp(screen_name,"personal-profile"))pressed_keys=BTN_L1;else if(!strcmp(screen_name,"led-save"))pressed_keys=BTN_A;}
 if(frame_no==3&&screen_name&&!strcmp(screen_name,"personal-history"))pressed_keys=BTN_R1;released_keys=0;ticks+=16;
}
void Background_tick(void){}
bool ModuleCommon_stopSignalled(void){return false;}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface *s,int *setting,HelpId help){return(GlobalInputResult){0};}
SDL_Surface *DisplayHelper_getSurface(const DisplayContext *d){return page;}
void ModuleCommon_frameEnd(SDL_Surface *s){if(frame_no<=3){char path[1400];snprintf(path,sizeof(path),"%s/%s-%d.bmp",out_dir,screen_name,page->w);assert(!SDL_SaveBMP(s,path));}}
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
static void render_efficiency_checks(void){
 uint32_t saved=ticks;bool off=fixture_display_off,hidden=fixture_hidden;fixture_display_off=fixture_hidden=false;ticks=5000;
 NxtSkin_begin(page,"OTA Stock",NULL,false);assert(NxtSkin_shouldDraw(0,false));assert(!NxtSkin_shouldDraw(ticks,false));assert(NxtSkin_shouldDraw(ticks,true));
 int static_paints=0;uint32_t last=0;for(int i=0;i<625;i++){ticks=5000+i*16;if(NxtSkin_shouldDraw(last,false)){last=ticks;static_paints++;}}assert(static_paints>=9&&static_paints<=11);
 NxtSkin_text(page,22,"A title that must scroll",32,150,1,NxtSkin_color(NXT_INK));int scroll_paints=0;last=0;for(int i=0;i<625;i++){ticks=5000+i*16;if(NxtSkin_shouldDraw(last,false)){last=ticks;scroll_paints++;}}assert(scroll_paints>=85&&scroll_paints<=101);
 fixture_display_off=true;assert(!NxtSkin_shouldDraw(0,true));fixture_display_off=false;fixture_hidden=true;assert(!NxtSkin_shouldDraw(0,true));fixture_hidden=false;assert(NxtSkin_shouldDraw(last,true));
 NxtSkin_begin(page,"OTA Stock",NULL,false);ticks=949;assert(NxtSkin_shouldDraw(UINT32_MAX-50,false));ticks=49;assert(!NxtSkin_shouldDraw(UINT32_MAX-50,false));
 TTF_Font *font=NxtSkin_font(page,22);SDL_Surface *text=NxtSkin_renderText(font,"Telex tiếng Việt");assert(text&&text==NxtSkin_renderText(font,"Telex tiếng Việt"));
 for(int i=0;i<500;i++){char line[100];snprintf(line,sizeof(line),"Cache eviction %d · tiếng Việt",i);assert(NxtSkin_renderText(font,line));}assert(NxtSkin_renderText(font,"Telex tiếng Việt"));NxtSkin_forgetFont(font);assert(NxtSkin_renderText(font,"Telex tiếng Việt"));
 printf("PASS actual idle redraws %d vs marquee %d / 625 polls; input immediate; hidden/off/wake/tick-wrap; cache reuse/eviction/font invalidation.\n",static_paints,scroll_paints);
 ticks=saved;fixture_display_off=off;fixture_hidden=hidden;
}
static void show(ModuleExitReason (*fn)(DisplayContext *),const char *name){frame_no=pressed_keys=released_keys=0;screen_name=name;assert(fn((DisplayContext*)1)==MODULE_EXIT_TO_MENU);}
#ifndef NXT_UI_FIXTURE_MAIN
#define NXT_UI_FIXTURE_MAIN main
#endif
int NXT_UI_FIXTURE_MAIN(int argc,char **argv){
 assert(argc==2&&!SDL_Init(0)&&!TTF_Init());out_dir=argv[1];nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.wallpaper_dim=20;nxt.panel_opacity=35;
 NxtPersonal_init();assert(db);typed_name="  spring  ";assert(create_listener()&&listener==0&&!strcmp(nickname,"spring"));typed_name="Second listener";assert(create_listener()&&listener==2);assert(switch_listener(0)&&!strcmp(nickname,"spring"));typed_name=NULL;assert(exec("INSERT INTO daily VALUES(0,date('now','localtime'),'Local',5400000,17),(0,date('now','localtime'),'Spotify',7200000,31),(0,date('now','localtime'),'SoundCloud',3600000,22),(0,date('now','localtime'),'Drive',1800000,8);"));
 for(int day=1;day<=6;day++){char sql[350];snprintf(sql,sizeof(sql),"INSERT INTO daily VALUES(0,date('now','localtime','-%d days'),'Local',%d,%d);",day,day*900000,day*3);assert(exec(sql));}
 assert(exec("INSERT INTO history(listener,path,title,artist,source,started,last,ms,qualified,position) VALUES(0,'/Music/demo.mp3','Bình yên ở đây','NXT Demo','Local',strftime('%s','now'),strftime('%s','now'),5400000,1,30000),(0,'spotify:track:0123456789012345678901','Giai điệu bình minh','Demo Artist','Spotify',strftime('%s','now'),strftime('%s','now'),7200000,1,22000),(0,'https://soundcloud.com/fixture/music','Một ngày nhẹ nhàng','NXT Demo','SoundCloud',strftime('%s','now'),strftime('%s','now'),3600000,1,10000),(0,'nxtdrive://1QueueFixture02/audio.wav','Chuyến đi cuối tuần','Demo Artist','Drive',strftime('%s','now'),strftime('%s','now'),1800000,1,15000);"));
 stats_load(0);sqlite3_int64 before_stats=stats.ms;assert(exec("UPDATE daily SET ms=ms+60000 WHERE listener=0 AND source=\"Local\" AND day=date(\"now\",\"localtime\");"));stats_load(0);assert(stats.ms==before_stats+60000);assert(exec("UPDATE daily SET ms=ms-60000 WHERE listener=0 AND source=\"Local\" AND day=date(\"now\",\"localtime\");"));stats_load(0);assert(stats.ms==before_stats);puts("PASS statistics cache refreshes after real SQLite writes and retains exact totals.");
 int colors=0;for(int mode=0;mode<3;mode++)for(int c=0;c<23;c++){nxt.theme=mode;nxt.accent_color=c<16?c:16;unsigned extreme[]={0x000000,0xffffff,0xff0000,0x00ff00,0x0000ff,0xffff00,0x123456};if(c>=16)nxt.accent_rgb=extreme[c-16];SDL_Color bg=NxtSkin_color(NXT_BG),panel=NxtSkin_color(NXT_PANEL),ink=NxtSkin_color(NXT_INK),muted=NxtSkin_color(NXT_MUTED),accent=NxtSkin_color(NXT_ACCENT),focus=NxtSkin_color(NXT_ACCENT_INK);assert(Theme_contrast(bg,ink)>=4.5&&Theme_contrast(panel,ink)>=4.5&&Theme_contrast(bg,muted)>=4.5&&Theme_contrast(panel,muted)>=4.5&&Theme_contrast(accent,bg)>=4.5&&Theme_contrast(accent,panel)>=4.5&&Theme_contrast(focus,accent)>=4.5);assert(!memcmp(&ink,&(SDL_Color){Theme_getColor(THEME_ROLE_PRIMARY,false).r,Theme_getColor(THEME_ROLE_PRIMARY,false).g,Theme_getColor(THEME_ROLE_PRIMARY,false).b,255},sizeof(ink)));colors++;}printf("PASS %d actual theme/RGB palettes: text, hints, accent and selection contrast >= 4.5\n",colors);
 nxt.theme=1;catalog_regression();pairing_retry();controls();nxt.led=0;nxt.led_mode=1;nxt.led_color=16;nxt.led_rgb=0x123456;nxt.led_strength=20;
 for(int i=0;i<2;i++){
  page=SDL_CreateRGBSurfaceWithFormat(0,i?640:1024,i?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);render_efficiency_checks();
  for(int mode=1;mode<=2;mode++){nxt.theme=mode;nxt.accent_color=2;NxtSkin_home(page,4);char image[1500];snprintf(image,sizeof(image),"%s/home-%s-%d.bmp",out_dir,mode==1?"dark":"light",page->w);assert(!SDL_SaveBMP(page,image));nxt.wallpaper_mode=2;NxtSkin_home(page,4);snprintf(image,sizeof(image),"%s/home-wallpaper-%s-%d.bmp",out_dir,mode==1?"dark":"light",page->w);assert(!SDL_SaveBMP(page,image));nxt.wallpaper_mode=0;show(NxtPersonal_statistics,mode==1?"statistics-dark":"statistics-light");}nxt.theme=1;nxt.wallpaper_mode=2;
  show(NxtPersonal_run,"personal-for-you");show(NxtPersonal_run,"personal-statistics");show(NxtPersonal_run,"personal-history");show(NxtPersonal_run,"personal-profile");
  frame_no=pressed_keys=released_keys=0;screen_name="listener-login";assert(NxtPersonal_login((DisplayContext*)1)==MODULE_EXIT_QUIT);
  frame_no=pressed_keys=released_keys=0;screen_name="avatar-gallery";assert(avatar_gallery((DisplayContext*)1)==-1);
  frame_no=pressed_keys=released_keys=0;screen_name="wallpaper-gallery";assert(NxtAppearance_wallpapers((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
  nxt.wallpaper_mode=0;show(Nxt_help,"usage");show(Nxt_changes,"changes");show(Nxt_about,"about");show(NxtControl_settings,"controls");show(NxtLED_palette,"led");
  assert(nxt.led==0&&nxt.led_mode==1&&nxt.led_rgb==0x123456&&nxt.led_strength==20);
  SDL_FreeSurface(page);
 }
 page=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);show(NxtLED_palette,"led-save");assert(nxt.led==1&&nxt.led_mode==1&&nxt.led_rgb==0x123456&&nxt.led_strength==20);SDL_FreeSurface(page);
 assert(device_exports==2);puts("PASS About Y exports device diagnostics at both screen sizes.");
 NxtPersonal_quit();NxtSkin_release();TTF_Quit();SDL_Quit();puts("PASS production v1.5 guide/change log/About/control/RGB screens at 640 and 1024, all 21 actions on 16 physical buttons, reset defaults, canceled LED preview restoration, ASan/UBSan.");return 0;
}
