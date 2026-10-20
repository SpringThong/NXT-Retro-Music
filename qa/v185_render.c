#define PlayerModule_isActive Fixture_old_isActive
#define Player_getPosition Fixture_old_position
#define userdata_snpath Fixture_flat_snpath
#define NXT_UI_V18 1
/* Native production rendering and controller-flow checks. Audio and ROM
 * inputs remain explicit fixtures; all UI/DSP/theme code is compiled. */
#define NXT_UI_FIXTURE_MAIN NxtFixture_old_main
#define NxtRecord_hide NxtFixture_record_hide
#define NxtRecord_release NxtFixture_record_release
#define ModuleCommon_frameBegin NxtFixture_frame_begin
#define ModuleCommon_frameEnd NxtFixture_frame_end
#define NxtHardware_output NxtFixture_hardware_output
#define NxtHardware_pcm NxtFixture_hardware_pcm
#define IMG_Load NxtFixture_IMG_Load
#define IMG_SavePNG NxtFixture_IMG_SavePNG
#define Nxt_save NxtFixture_save
#include "v15_ui_render.c"
#undef Nxt_save
#include "nxt_performance.h"
#include "nxt_audio_mode.h"
#include "nxt_write.h"
bool Nxt_save(void);
void NxtAudio_streaming(void){}
const char *NxtAudio_modeName(int m){return m==1?"Bit-Perfect":m==2?"Automatic":"Normal";}
void NxtAudio_setMode(int m){nxt.audio_mode=m;nxt.bit_perfect=m==1;Nxt_save();}
static bool qa_save_failure;static int qa_saves;
bool Nxt_save(void){qa_saves++;return !qa_save_failure;}
#include "nxt_appearance.h"
#undef main
#undef NxtRecord_hide
#undef NxtRecord_release
#undef ModuleCommon_frameBegin
#undef ModuleCommon_frameEnd
#undef IMG_Load
#undef IMG_SavePNG
#undef NxtHardware_output
#undef NxtHardware_pcm
#undef PlayerModule_isActive
#undef Player_getPosition
#undef userdata_snpath
#include "nxt_startup.h"
#include "nxt_bitperfect.h"
static bool qa_resumed,qa_play;
static int qa_pos,qa_saved_position;static bool qa_seek;
bool PlayerModule_isActive(void){return qa_resumed;}
int Player_getPosition(void){return qa_pos;}
int userdata_snpath(const char *rel,char *out,size_t size){int n=NxtPersonal_route(rel,out,size);return n>=0?n:snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
ModuleExitReason PlayerModule_runResumeMode(DisplayContext *d,const ResumeState *r,bool play){qa_resumed=true;qa_play=play;qa_saved_position=r->position_ms;fixture_state=play?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED;return MODULE_EXIT_TO_MENU;}
bool NxtDrive_isStream(const char *p){return false;}
bool NxtBit_ready(void){return false;}
const char *NxtBit_status(void){return "Bit-Perfect requested · paused / unavailable";}
const char *NxtBit_error(void){return "Connect USB DAC";}
static const char *qa_output="USB DAC · Black Pearl",*qa_pcm="DAC: 44.1 kHz / 24-bit PCM · 2ch";
const char *NxtHardware_output(void){return qa_output;}
const char *NxtHardware_pcm(void){return qa_pcm;}
bool NxtStock_requestPanel(void){return true;}
#include "nxt_theme.h"
#include "nxt_custom.h"
#include "nxt_eq.h"
#include "nxt_monitor.h"
#include "nxt_record.h"
#include <utf8proc.h>
extern SDL_Surface *IMG_Load(const char *);
extern int IMG_SavePNG(SDL_Surface *,const char *);
static int qa_lyrics_variant;
static const int *sequence;static int sequence_count;static bool qa_capture_choices;
void ModuleCommon_frameBegin(void){assert(++frame_no<800);int key=frame_no<=sequence_count?sequence[frame_no-1]:BTN_B;pressed_keys=key>0?key:0;released_keys=key<0?-key:0;ticks+=125;}
void ModuleCommon_frameEnd(SDL_Surface *s){if(qa_capture_choices&&frame_no==2){char path[1500];snprintf(path,sizeof(path),"%s/performance-mode-choices-%d.bmp",out_dir,page->w);assert(!SDL_SaveBMP(page,path));}}
static void input(const int *keys,int n){sequence=keys;sequence_count=n;frame_no=pressed_keys=released_keys=0;}
int PAD_anyPressed(void){return pressed_keys!=0;}
void Player_refreshOptions(void){}
ModuleExitReason NxtSoundCloud_recommendations(DisplayContext *d,bool p){return MODULE_EXIT_TO_MENU;}
int Lyrics_currentIndex(int position){return position<50000?0:1;}
const char *Lyrics_lineText(int index){if(qa_lyrics_variant)return qa_lyrics_variant==1?"AAAA · Một ngày bình yên":"BBBB · Lắng nghe giai điệu";return index==0?"Một ngày bình yên, lắng nghe giai điệu…":"Nhạc của bạn · 한국어 日本語 中文 ภาษาไทย";}
ModuleExitReason SettingsModule_run(DisplayContext*d){return MODULE_EXIT_TO_MENU;}
static int qa_wifi_calls;
ModuleExitReason NxtTransfer_run(DisplayContext*d){qa_wifi_calls++;return MODULE_EXIT_TO_MENU;}
bool NxtUSB_requestTransfer(void){return false;}
void MusicPlaying_leave(void){}void MusicPlaying_invalidate(void){}bool MusicPlaying_frame(bool r){return true;}void MusicPlaying_paintLayers(void){}
int PAD_anyJustReleased(void){return released_keys!=0;}int PAD_anyJustPressed(void){return pressed_keys!=0;}
void Nxt_returnHome(void){}void GFX_blitHardwareHints(SDL_Surface*s,int a){}
ModuleExitReason NxtWallpapers_run(DisplayContext*d){return MODULE_EXIT_TO_MENU;}
int Lyrics_lineCount(void){return 2;}bool Lyrics_isAvailable(void){return true;}
void Player_togglePause(void){}void Player_seek(int p){}void SetVolume(int v){}void SetBrightness(int v){}
void Nxt_setShuffle(bool v){nxt.shuffle=v;}void Nxt_setRepeat(int v){nxt.repeat_mode=v;}void Nxt_screenOff(void){}void Nxt_requestScreenOff(void){}void ModuleCommon_requestQuit(void){}
void Nxt_sleepTimer(int m){}bool NxtHardware_select(int o){return true;}const char *NxtHardware_details(void){return "Fixture output";}const char *NxtHardware_score(void){return "Fixture";}const char *NxtHardware_bottomStatus(void){return "Fixture";}int NxtHardware_bottomUSB(bool h){return 0;}
const char *SettingsModule_getScreenOffDisplayStr(void){return "Screen off";}void SettingsModule_cycleScreenOffNext(void){}void SettingsModule_cycleScreenOffPrev(void){}
bool Nxt_chooseFolder(DisplayContext*d){return false;}ModuleExitReason Nxt_library(DisplayContext*d){return MODULE_EXIT_TO_MENU;}bool Settings_toggleBool(const BoolSetting*k){return true;}
int Player_getOutputRate(void){return 48000;}
int Player_getOutputBits(void){return 32;}
int64_t Player_getListeningTime(void){return ticks;}
bool Player_outputReady(void){return true;}
static void shot(const char *name){char p[1500];snprintf(p,sizeof(p),"%s/%s-%d.bmp",out_dir,name,page->w);assert(!SDL_SaveBMP(page,p));}
static void original_preserved(void){nxt.interface_style=0;nxt.theme=2;nxt.accent_color=16;nxt.accent_rgb=0xb0629a;nxt.wallpaper_mode=11;nxt.wallpaper_dim=20;nxt.panel_opacity=35;nxt.cover_mode=0;NxtConfig before=nxt;const int cancel[]={BTN_RIGHT,BTN_Y,BTN_B,BTN_B};input(cancel,4);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(nxt.interface_style==0&&nxt.theme==before.theme&&nxt.accent_color==before.accent_color&&nxt.accent_rgb==before.accent_rgb&&nxt.wallpaper_mode==11&&nxt.cover_mode==0);NxtSkin_playing(page,1,1,false,0);shot("original-square-wallpaper");nxt.cover_mode=1;NxtSkin_playing(page,1,1,false,0);NxtRecord_paint(true);shot("original-vinyl-wallpaper");const int apply[]={BTN_RIGHT,BTN_A};input(apply,2);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==1&&nxt.accent_rgb==before.accent_rgb&&nxt.wallpaper_mode==11);nxt.interface_style=0;nxt.theme=1;nxt.accent_color=0;nxt.wallpaper_mode=0;}
static void original_cover_fit(void){
 SDL_Surface *saved=cover;int mode=nxt.cover_mode,style=nxt.interface_style;nxt.interface_style=0;nxt.cover_mode=0;
 for(int tall=0;tall<2;tall++){cover=SDL_CreateRGBSurfaceWithFormat(0,tall?200:400,tall?400:200,32,SDL_PIXELFORMAT_ARGB8888);assert(cover);SDL_FillRect(cover,NULL,SDL_MapRGB(cover->format,12,34,250));NxtSkin_playing(page,1,18,false,0);
 int x=153*page->w/1024,y=268*page->w/1024;Uint8 red,green,blue;Uint32 pixel=*((Uint32*)((Uint8*)page->pixels+y*page->pitch)+x);SDL_GetRGB(pixel,page->format,&red,&green,&blue);assert(red==12&&green==34&&blue==250);
 x=40*page->w/1024;y=155*page->w/1024;pixel=*((Uint32*)((Uint8*)page->pixels+y*page->pitch)+x);SDL_GetRGB(pixel,page->format,&red,&green,&blue);assert(red!=12||green!=34||blue!=250);shot(tall?"original-cover-portrait-fit":"original-cover-landscape-fit");NxtSkin_forgetArt();SDL_FreeSurface(cover);}
 cover=saved;nxt.cover_mode=mode;nxt.interface_style=style;puts("PASS original player portrait/landscape album images retain aspect ratio and stay centered in the old frame.");
}
static void galleries(void){const int select[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,true,0)==31);input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==31);const int cancel[]={BTN_RIGHT,BTN_B};input(cancel,2);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==-1);int value=0x123456;const int rgb_cancel[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_B};input(rgb_cancel,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x123456);const int rgb_save[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(rgb_save,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x134456);puts("PASS native RGB save/cancel, 300-icon/180-emoji paged navigation, theme preview/apply/cancel preserves Original color, wallpaper and cover mode.");}
static void profile_flow(void){nxt.mood=nxt.genre=nxt.country=0;
 char path[1400];profile_path(0,"preferences.json",path,sizeof(path));char parent[1400];snprintf(parent,sizeof(parent),"%s",path);*strrchr(parent,'/')=0;assert(mkdir_p(parent));FILE*f=fopen(path,"w");assert(f);fputs("{\"avatar_mode\":1,\"avatar_emoji\":177}",f);fclose(f);
 ProfileAvatar a;int saved=nxt.avatar_emoji;read_profile_avatar(0,0,&a);assert(a.mode==1&&a.emoji==177&&nxt.avatar_emoji==saved);
 const int select[]={BTN_A};input(select,1);ModuleExitReason got=NxtPersonal_login((DisplayContext*)1);assert(got==MODULE_EXIT_TO_MENU&&frame_no==1&&nxt.mood==0&&nxt.genre==0&&nxt.country==0);
 const int back[]={0,BTN_B};input(back,2);assert(choose_listener((DisplayContext*)1,false)==MODULE_EXIT_TO_MENU);shot("profiles-startup");
 const int choices[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_DOWN,BTN_A};input(choices,6);assert(NxtPersonal_checkIn((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.mood==1&&nxt.genre==1&&nxt.country==1);
 puts("PASS saved startup avatar, no global-avatar mutation, cold-start profile selection without questionnaire; optional Personal check-in and mood/genre/country commit.");
}
static void eq_flow(void){nxt.eq_preset=0;const int rock[]={BTN_RIGHT,BTN_RIGHT,BTN_A};input(rock,3);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.eq_preset==2&&nxt.eq_bands==5&&nxt.eq[0]==3);nxt.eq_preset=12;const int custom[]={BTN_A,BTN_UP,BTN_RIGHT,BTN_DOWN,BTN_B,BTN_B};input(custom,6);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.eq_preset==12&&nxt.eq_bands==5&&nxt.eq[0]==3.5f&&nxt.eq[1]==.5f);NxtEQ_drawCustom(page,1);shot("eq-custom-five");NxtEQ_drawPresets(page,12);shot("eq-presets");for(int i=0;i<12;i++){NxtEQ_apply(i);assert(nxt.eq_bands==5&&nxt.preamp<=0&&nxt.preamp>=-6);for(int k=0;k<5;k++)assert(nxt.eq[k]>=-12&&nxt.eq[k]<=12);}puts("PASS SELECT preset grid -> apply Rock / Custom five sliders, actual controller gain edits and all 12 DSP presets.");}
static void icons_and_emoji(void){for(int i=0;i<180;i++){char p[128];snprintf(p,sizeof(p),"./res/emoji3d/%03d.png",i);SDL_Surface *im=IMG_Load(p);assert(im&&im->w==160&&im->h==160);SDL_FreeSurface(im);}for(int i=0;i<300;i++){char p[180];snprintf(p,sizeof(p),"./res/icons/%s.png",NxtCustom_iconName(i));SDL_Surface *im=IMG_Load(p);assert(im);SDL_FreeSurface(im);}
 const int last_icons[]={BTN_LEFT,BTN_A};input(last_icons,2);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==299);input(last_icons,2);assert(NxtCustom_gallery((DisplayContext*)1,true,0)==179);
SDL_FillRect(page,NULL,SDL_MapRGB(page->format,20,25,28));for(int i=0;i<64;i++){int x=(22+i%8*125)*page->w/1024,y=(15+i/8*89)*page->w/1024;char name[32];snprintf(name,sizeof(name),"emoji-%d",i);NxtSkin_icon(page,name,x,y,55*page->w/1024,NxtSkin_color(NXT_INK));SDL_Surface *glyph=NxtSkin_renderText(NxtSkin_font(page,56),NxtCustom_emoji(i));assert(glyph&&glyph->w>0);utf8proc_int32_t cp;assert(utf8proc_iterate((const utf8proc_uint8_t*)NxtCustom_emoji(i),-1,&cp)>0&&NxtText_hasGlyph(NxtSkin_font(page,56),cp));NxtSkin_text(page,15,NxtCustom_emoji(i),x,y+58*page->w/1024,110*page->w/1024,NxtSkin_color(NXT_MUTED));}shot("emoji-64");SDL_FillRect(page,NULL,SDL_MapRGB(page->format,20,25,28));for(int i=0;i<64;i++){int x=(22+i%8*125)*page->w/1024,y=(15+i/8*89)*page->w/1024;char path[220];snprintf(path,sizeof(path),"./res/icons/%s.png",NxtCustom_iconName(i));SDL_Surface *icon=IMG_Load(path);assert(icon);SDL_FreeSurface(icon);NxtSkin_icon(page,NxtCustom_iconName(i),x,y,55*page->w/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(page,14,NxtCustom_iconLabel(i),x,y+58*page->w/1024,115*page->w/1024,NxtSkin_color(NXT_MUTED));}shot("icons-64");nxt.avatar_mode=1;nxt.avatar_emoji=12;for(int i=0;i<7;i++)nxt.menu_icons[i]=301+i;NxtSkin_home(page,4);shot("home-personal-emoji");nxt.avatar_mode=0;nxt.avatar_icon=23;NxtSkin_home(page,4);shot("home-personal-migrated-icon");nxt.avatar_mode=2;snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"NXT");NxtSkin_home(page,4);shot("home-personal-text");nxt.avatar_mode=3;snprintf(nxt.avatar_path,sizeof(nxt.avatar_path),"./res/avatar-test.png");NxtSkin_home(page,4);shot("home-personal-photo");nxt.avatar_mode=0;memset(nxt.menu_icons,0,sizeof(nxt.menu_icons));}

#include <time.h>
static uint64_t area_hash(int xx,int yy,int ww,int hh){int x=xx*page->w/1024,y=yy*page->w/1024,w=ww*page->w/1024,h=hh*page->w/1024;uint64_t hash=1469598103934665603ULL;for(int j=y;j<y+h;j++){Uint32 *p=(Uint32*)((Uint8*)page->pixels+j*page->pitch);for(int i=x;i<x+w;i++)hash=(hash^p[i])*1099511628211ULL;}return hash;}
static uint64_t lyric_hash(int style){return style?area_hash(43,555,938,42):area_hash(32,395,960,100);}
static void audio_info_flow(void){
 int style=nxt.interface_style;nxt.interface_style=1;
 NxtSkin_playing(page,1,18,true,2);uint64_t src=area_hash(43,730,938,16),out=area_hash(43,750,938,16);TrackInfo saved=track_info;
 snprintf(track_info.codec,sizeof(track_info.codec),"FLAC");track_info.sample_rate=96000;track_info.bit_depth=24;track_info.bitrate=2400000;track_info.channels=2;NxtSkin_playing(page,1,18,true,2);assert(src!=area_hash(43,730,938,16)&&out==area_hash(43,750,938,16));src=area_hash(43,730,938,16);qa_output="DAC: Fixture DAC 2";qa_pcm="DAC: 48 kHz / 32-bit PCM · 2ch";NxtSkin_playing(page,1,18,true,2);assert(src==area_hash(43,730,938,16)&&out!=area_hash(43,750,938,16));shot("actual-source-output-separate");track_info=saved;nxt.interface_style=style;qa_output="USB DAC · Black Pearl";qa_pcm="DAC: 44.1 kHz / 24-bit PCM · 2ch";
 puts("PASS native audio info lanes: source codec/rate/bitrate/channels and actual DAC/PCM change independently; hardware inputs are fixtures.");
}
static void performance_flow(void){
 nxt.performance_mode=NXT_PERFORMANCE_BALANCED;int saves=qa_saves;
 const int battery[]={BTN_A,BTN_UP,BTN_A,0,BTN_B};input(battery,5);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.performance_mode==NXT_PERFORMANCE_BATTERY&&qa_saves==saves+1);shot("performance-battery");
 const int fast[]={BTN_A,BTN_DOWN,BTN_DOWN,BTN_A,0,BTN_B};input(fast,6);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.performance_mode==NXT_PERFORMANCE_FAST);shot("performance-fast");
 const int cancel[]={BTN_A,BTN_UP,BTN_B,BTN_B};saves=qa_saves;input(cancel,4);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.performance_mode==NXT_PERFORMANCE_FAST&&qa_saves==saves);
 qa_save_failure=true;const int failure[]={BTN_A,BTN_UP,BTN_A,BTN_B};input(failure,4);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.performance_mode==NXT_PERFORMANCE_FAST);qa_save_failure=false;
 const int balanced[]={BTN_A,BTN_UP,BTN_A,0,BTN_B};input(balanced,5);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.performance_mode==NXT_PERFORMANCE_BALANCED);shot("performance-balanced");
 const int choices[]={BTN_A,0,BTN_B,BTN_B};qa_capture_choices=true;input(choices,4);assert(NxtPerformance_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);qa_capture_choices=false;
 const int route[]={BTN_DOWN,BTN_DOWN,BTN_A,0,BTN_B,BTN_B};input(route,6);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 puts("PASS three performance modes: immediate save/apply, cancel, failed-save rollback, settings route and Balanced default.");
}
static void new_menu_flows(void){
 nxt.interface_style=0;const int show[]={0,BTN_B};input(show,2);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("settings-five-groups");
 input(show,2);assert(NxtAppearance_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("personal-appearance-four-groups");
 input(show,2);assert(NxtAppearance_original((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("original-theme-options");
 input(show,2);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("about-update-second");
 input(show,2);assert(profile_edit((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("edit-profile-clean");
 int mode=nxt.cover_mode;const int cancel[]={0,BTN_RIGHT,0,BTN_B};input(cancel,4);assert(NxtAppearance_album((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.cover_mode==mode);shot("album-preview-cancel");
 const int apply[]={0,BTN_RIGHT,0,BTN_A};input(apply,4);assert(NxtAppearance_album((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.cover_mode!=mode);shot("album-preview-apply");nxt.cover_mode=mode;
 const int font[]={BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_RIGHT,BTN_B};int size=nxt.font_size;input(font,6);assert(NxtAppearance_original((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.font_size==size+5);nxt.font_size=size;
 input(show,2);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("full-lyrics-one-line");
 const int updates[]={BTN_DOWN,BTN_A,0,BTN_B,BTN_B};input(updates,5);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 const int wifi[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B,BTN_B};input(wifi,6);int calls=qa_wifi_calls;assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_wifi_calls==calls+1);
 performance_flow();audio_info_flow();
 puts("PASS native v1.8.2 settings, personal-appearance hub, clean profile editor, merged update route, Original font edit, live album preview apply/cancel and full lyric screen.");
}
static void lyric_visible_and_budget(int style){int animations=nxt.animations;nxt.animations=0;qa_lyrics_variant=1;NxtSkin_playing(page,1,18,true,2);uint64_t first=lyric_hash(style);qa_lyrics_variant=2;NxtSkin_playing(page,1,18,true,2);assert(first!=lyric_hash(style));uint64_t stable=lyric_hash(style);ticks+=15000;NxtSkin_playing(page,1,18,true,2);assert(stable==lyric_hash(style));qa_lyrics_variant=0;nxt.animations=animations;clock_t start=clock();for(int n=0;n<8;n++)NxtSkin_playing(page,1,18,true,2);printf("RENDER_BUDGET %d %d %.3f ms per cached paint (host)\n",style,page->w,1000.*(clock()-start)/CLOCKS_PER_SEC/8);}


static void v185_controls(void){
 NxtConfig before=nxt;nxt.bit_perfect=0;int preset=nxt.eq_preset;float bands[31];memcpy(bands,nxt.eq,sizeof(bands));
 const int select[]={0,-BTN_SELECT};input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));
 const int nested[]={BTN_START,0,-BTN_SELECT};input(nested,3);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no<=3&&nxt.eq_preset==preset);
 nxt.bit_perfect=1;input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));shot("bit-perfect-EQ-bypass");
 nxt=before;
 const int audio[]={BTN_A,0,BTN_B,BTN_B};input(audio,4);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("audio-settings-v185");
 const int mode[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B,BTN_B};nxt.bit_perfect=0;nxt.audio_mode=0;input(mode,6);assert(Nxt_quickOptions((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.bit_perfect==1&&nxt.eq_preset==preset);shot("quick-options-v185");nxt=before;
 puts("PASS actual SELECT release: presets close unchanged, nested EQ closes fully, Bit-Perfect EQ closes unchanged; native settings and START mode selection.");
}
static void v185_startup(void){
 int old_child=child;bool old_track=track_loaded,old_playing=playing;PlayerState old_state=fixture_state;
 child=-1;track_loaded=playing=false;fixture_state=PLAYER_STATE_STOPPED;fixture_path[0]=0;qa_resumed=false;nxt.bit_perfect=0;nxt.auto_play=0;
 char file[1600];userdata_snpath("last-playback.json",file,sizeof(file));unlink(file);int mood=nxt.mood,genre=nxt.genre,country=nxt.country;
 const int back[]={0,BTN_B};input(back,2);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&!NxtStartup_preview()&&frame_no==2);shot("startup-empty-now-playing");assert(nxt.mood==mood&&nxt.genre==genre&&nxt.country==country);
 FILE *f=fopen(file,"w");assert(f);fputs("{\"path\":\"/missing/music.flac\",\"title\":\"Last song · Nhạc lần trước\",\"artist\":\"Saved artist\",\"source\":\"Local\",\"position_ms\":81723,\"duration_ms\":192000,\"sample_rate\":44100,\"bit_depth\":24,\"channels\":2}",f);fclose(f);
 input(back,2);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&!qa_resumed&&!NxtStartup_preview());shot("startup-missing-saved-track");JSON_Value *v=json_parse_file(file);assert(json_object_get_number(json_value_get_object(v),"position_ms")==81723);json_value_free(v);
 char song[1600];snprintf(song,sizeof(song),"%s/saved.flac",getenv("QA_DATA"));f=fopen(song,"w");assert(f);fputs("explicit audio fixture",f);fclose(f);
 v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"path",song);json_object_set_string(o,"title","Saved song");json_object_set_string(o,"source","Local");json_object_set_number(o,"position_ms",81723);assert(json_serialize_to_file(v,file)==JSONSuccess);json_value_free(v);
 input(NULL,0);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_resumed&&!qa_play&&qa_saved_position==81723&&!NxtStartup_preview());
 qa_resumed=false;nxt.auto_play=1;input(NULL,0);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_resumed&&qa_play&&qa_saved_position==81723);nxt.auto_play=0;qa_resumed=false;
 snprintf(fixture_path,sizeof(fixture_path),"%s",song);fixture_info=(TrackInfo){.duration_ms=192000,.sample_rate=44100,.channels=2,.bit_depth=24};snprintf(fixture_info.title,sizeof(fixture_info.title),"Saved native song");qa_pos=94567;fixture_state=PLAYER_STATE_PAUSED;NxtStartup_save();NxtWrite_flush();v=json_parse_file(file);assert(json_object_get_number(json_value_get_object(v),"position_ms")==94567);json_value_free(v);
 qa_pos=0;qa_seek=true;NxtStartup_save();NxtWrite_flush();v=json_parse_file(file);assert(json_object_get_number(json_value_get_object(v),"position_ms")==94567);json_value_free(v);qa_seek=false;
 int old_listener=listener;listener=1;char profile_file[1600];userdata_mkdir("listeners/1");userdata_snpath("last-playback.json",profile_file,sizeof(profile_file));assert(strstr(profile_file,"listeners/1/"));qa_pos=128000;NxtStartup_save();NxtWrite_flush();v=json_parse_file(profile_file);assert(v&&json_object_get_number(json_value_get_object(v),"position_ms")==128000);json_value_free(v);v=json_parse_file(file);assert(v&&json_object_get_number(json_value_get_object(v),"position_ms")==94567);json_value_free(v);listener=old_listener;
 NxtStartup_quit();child=old_child;track_loaded=old_track;playing=old_playing;fixture_state=old_state;
 puts("PASS real startup card: empty Now Playing, no questionnaire, missing track kept, paused restore, optional auto-play, atomic position save and profile isolation. Audio playback is a fixture.");
}
static void album_cache_pixels(void){
 SDL_Surface *source=SDL_CreateRGBSurfaceWithFormat(0,37,29,32,SDL_PIXELFORMAT_ARGB8888);assert(source);
 SDL_Surface *direct=SDL_CreateRGBSurfaceWithFormat(0,160,120,32,SDL_PIXELFORMAT_ARGB8888),*cached=SDL_CreateRGBSurfaceWithFormat(0,160,120,32,SDL_PIXELFORMAT_ARGB8888);assert(direct&&cached);
 for(int blend=0;blend<2;blend++){
  NxtSkin_forgetArt();SDL_SetSurfaceBlendMode(source,blend?SDL_BLENDMODE_BLEND:SDL_BLENDMODE_NONE);
  for(int y=0;y<source->h;y++)for(int x=0;x<source->w;x++)((Uint32*)((Uint8*)source->pixels+y*source->pitch))[x]=SDL_MapRGBA(source->format,x*6,y*8,(x+y)*3,blend?(x*7)%256:255);
  for(int i=0;i<4;i++){SDL_Rect dst={3,7,70+i*20,47+i*11};Uint32 bg=SDL_MapRGBA(direct->format,43,61,89,255);SDL_FillRect(direct,NULL,bg);SDL_FillRect(cached,NULL,bg);SDL_BlitScaled(source,NULL,direct,&dst);NxtSkin_cachedArt(cached,source,dst);
   for(int y=0;y<direct->h;y++){if(memcmp((Uint8*)direct->pixels+y*direct->pitch,(Uint8*)cached->pixels+y*cached->pitch,direct->w*4)){fprintf(stderr,"cache mismatch blend=%d size=%d row=%d\n",blend,i,y);abort();}}
   SDL_FillRect(cached,NULL,bg);NxtSkin_cachedArt(cached,source,dst);for(int y=0;y<direct->h;y++){if(memcmp((Uint8*)direct->pixels+y*direct->pitch,(Uint8*)cached->pixels+y*cached->pitch,direct->w*4)){fprintf(stderr,"cache mismatch blend=%d size=%d row=%d\n",blend,i,y);abort();}}
  }
 }
 NxtSkin_forgetArt();SDL_FreeSurface(source);SDL_FreeSurface(direct);SDL_FreeSurface(cached);puts("PASS cached album pixels exactly match direct SDL scaling: four sizes, opaque and alpha, cold and warm renders.");
}
int main(int argc,char **argv){assert(argc==2&&!SDL_Init(0)&&!TTF_Init());album_cache_pixels();out_dir=argv[1];nxt.performance_mode=NXT_PERFORMANCE_BALANCED;nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.animations=1;nxt.cover_mode=1;nxt.width=1;nxt.limiter=1;nxt.eq_bands=10;nxt.avatar_icon=-1;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0xffa4c9;NxtPersonal_init();NxtControl_defaults();fixture_state=PLAYER_STATE_PLAYING;child=123;connected=true;track_loaded=playing=true;position_ms=37000;position_since=ticks;duration_ms=192000;snprintf(current.uri,sizeof(current.uri),"spotify:track:0123456789012345678901");snprintf(track_info.title,sizeof(track_info.title),"Nắng dịu trong một chiều bình yên");snprintf(track_info.artist,sizeof(track_info.artist),"NXT Music · Nghệ sĩ thử nghiệm");snprintf(track_info.album,sizeof(track_info.album),"Album · Việt 한국어 日本語");snprintf(track_info.codec,sizeof(track_info.codec),"Ogg Vorbis");track_info.sample_rate=44100;track_info.bit_depth=16;track_info.bitrate=320000;track_info.duration_ms=192000;cover=IMG_Load("./res/avatar-test.png");assert(cover);
 for(int size=0;size<2;size++){page=SDL_CreateRGBSurfaceWithFormat(0,size?640:1024,size?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);drawing=page;original_preserved();original_cover_fit();galleries();profile_flow();eq_flow();icons_and_emoji();new_menu_flows();v185_controls();for(int i=0;i<36;i++){nxt.interface_style=i;char name[90];snprintf(name,sizeof(name),"theme-%02d-home",i);NxtSkin_home(page,3);shot(name);snprintf(name,sizeof(name),"theme-%02d-playing",i);NxtSkin_playing(page,1,18,true,2);Spectrum_init();for(int n=0;n<3;n++){ticks+=50;Spectrum_update();}Spectrum_paint(UI_LAYER_ANIMATION);NxtRecord_paint(true);shot(name);Spectrum_quit();lyric_visible_and_budget(i);SDL_Color bg=NxtSkin_color(NXT_BG),ink=NxtSkin_color(NXT_INK);assert(Theme_contrast(bg,ink)>=4.5);}nxt.interface_style=0;nxt.eq_preset=0;for(int color=0;color<21;color++)for(int mode=0;mode<5;mode++){nxt.spectrum_palette=color;nxt.spectrum_mode=mode;Spectrum_init();NxtSkin_playing(page,1,1,false,0);for(int n=0;n<8;n++){ticks+=50;Spectrum_update();}Spectrum_paint(UI_LAYER_ANIMATION);if(mode==1||color==20){char name[80];snprintf(name,sizeof(name),"spectrum-%02d-%d",color,mode);shot(name);}Spectrum_quit();}personal_header(page,0,true);personal_cards(page,0,4);shot("for-you-online");nxt.interface_style=24;for(int lang=0;lang<14;lang++){nxt.language=lang;NxtSkin_home(page,3);char name[80];snprintf(name,sizeof(name),"locale-%02d-home",lang);shot(name);}nxt.language=1;nxt.interface_style=0;input(NULL,0);assert(NxtMonitor_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);v185_startup();SDL_FreeSurface(page);page=NULL;drawing=NULL;}
 for(int effect=0;effect<6;effect++){nxt.led_effect=effect;nxt.led_color=16;nxt.led_rgb=0xff0000;nxt.led_rgb2=0x0000ff;assert(NxtLED_groupRGB(0,1000)<=0xffffff);if(effect==1)assert(NxtLED_groupRGB(1,0)==0xff0000&&NxtLED_groupRGB(4,0)==0x0000ff);if(effect==2)assert(NxtLED_groupRGB(1,1000)!=NxtLED_groupRGB(1,2000));}
 fixture_hidden=true;assert(!NxtSkin_shouldDraw(0,true));fixture_hidden=false;fixture_display_off=true;assert(!NxtSkin_shouldDraw(0,true));fixture_display_off=false;NxtPersonal_quit();NxtSkin_release();SDL_FreeSurface(cover);cover=NULL;TTF_Quit();SDL_Quit();puts("PASS static full-sentence lyrics in all 36 device layouts at both resolutions; 36 native home/player interfaces, reserved icon/quality/output lanes, 300 icons + 180 3D emoji, 105 spectrum combinations, 6 LED color functions, 640/1024 native surfaces under ASan/UBSan. Audio, account and physical-device inputs are fixtures.");return 0;}

bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
ModuleExitReason NxtSoundCloud_playSaved(DisplayContext *d,const char *p,const char *t,const char *a){return FixtureCloudSaved(d,p,t,a);}

void ModuleCommon_signalStop(void){}

bool Player_resume(void){return qa_seek;}
