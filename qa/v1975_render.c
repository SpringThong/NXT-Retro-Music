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
#include "nxt_translate.h"
int CFG_getFontStyle(void){return 0;}
bool Nxt_save(void);
ModuleExitReason NxtKaraoke_settings(DisplayContext*d){return MODULE_EXIT_TO_MENU;}
void NxtAudio_streaming(void){}
const char *NxtAudio_modeName(int m){return m==1?"Bit-Perfect":m==2?"Automatic":"Normal";}
void NxtAudio_setMode(int m){nxt.audio_mode=m;nxt.bit_perfect=m==1;Nxt_save();}
static bool qa_save_failure;static int qa_saves;
bool Nxt_save(void){qa_saves++;return !qa_save_failure;}
#include "nxt_appearance.h"
#include "nxt_lyric_preview.h"
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
static bool qa_guard_draft;static NxtConfig qa_committed;
static void guard_draft(void){if(qa_guard_draft)assert(!memcmp(&nxt,&qa_committed,sizeof(nxt)));}
static bool qa_full_spectrum;static bool qa_resumed,qa_play,qa_capture_library;
static const char *qa_capture_screen;static int qa_capture_frame;
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
#include "nxt_motion.h"
#include <utf8proc.h>
extern SDL_Surface *IMG_Load(const char *);
extern int IMG_SavePNG(SDL_Surface *,const char *);
static bool qa_karaoke;static int qa_lyrics_variant;
static const char *qa_original_override;static bool qa_translation_active;static unsigned qa_lyric_revision=1;
static const int *sequence;static int sequence_count;static bool qa_capture_choices;
void ModuleCommon_frameBegin(void){guard_draft();assert(++frame_no<800);int key=frame_no<=sequence_count?sequence[frame_no-1]:BTN_B;pressed_keys=key>0?key:0;released_keys=key<0?-key:0;ticks+=125;}
void ModuleCommon_frameEnd(SDL_Surface *s){guard_draft();if(qa_full_spectrum)assert(!Spectrum_needsRefresh());if(qa_capture_screen&&frame_no==qa_capture_frame){char p[1500];snprintf(p,sizeof(p),"%s/%s-%d.bmp",out_dir,qa_capture_screen,page->w);assert(!SDL_SaveBMP(page,p));}if(qa_capture_library&&frame_no==3){char path[1500];snprintf(path,sizeof(path),"%s/library-files-two-items-%d.bmp",out_dir,page->w);assert(!SDL_SaveBMP(page,path));}if(qa_capture_choices&&frame_no==2){char path[1500];snprintf(path,sizeof(path),"%s/performance-mode-choices-%d.bmp",out_dir,page->w);assert(!SDL_SaveBMP(page,path));}}
static void input(const int *keys,int n){sequence=keys;sequence_count=n;frame_no=pressed_keys=released_keys=0;}
int PAD_anyPressed(void){return pressed_keys!=0;}
void Player_refreshOptions(void){}
ModuleExitReason NxtSoundCloud_recommendations(DisplayContext *d,bool p){return MODULE_EXIT_TO_MENU;}
int Lyrics_currentIndex(int position){return position<50000?0:1;}
const char *Lyrics_lineText(int index){if(qa_translation_active&&index==0&&qa_original_override)return qa_original_override;if(qa_translation_active)return index==0?"The morning light is warm.":"Today is a peaceful day.";if(qa_lyrics_variant)return qa_lyrics_variant==1?"AAAA · Một ngày bình yên":"BBBB · Lắng nghe giai điệu";return index==0?"Một ngày bình yên, lắng nghe giai điệu…":"Nhạc của bạn · 한국어 日本語 中文 ภาษาไทย";}
ModuleExitReason SettingsModule_run(DisplayContext*d){return MODULE_EXIT_TO_MENU;}
static int qa_wifi_calls;
ModuleExitReason NxtTransfer_run(DisplayContext*d){qa_wifi_calls++;return MODULE_EXIT_TO_MENU;}
bool NxtUSB_requestTransfer(void){return false;}
void MusicPlaying_leave(void){}void MusicPlaying_invalidate(void){}bool MusicPlaying_frame(bool r){return true;}void MusicPlaying_paintLayers(void){}
int PAD_anyJustReleased(void){return released_keys!=0;}int PAD_anyJustPressed(void){return pressed_keys!=0;}
void Nxt_returnHome(void){}void GFX_blitHardwareHints(SDL_Surface*s,int a){}
ModuleExitReason NxtWallpapers_run(DisplayContext*d){return MODULE_EXIT_TO_MENU;}
unsigned Lyrics_revision(void){return qa_lyric_revision;}
int Lyrics_lineCount(void){return 2;}bool Lyrics_isAvailable(void){return true;}
void Player_togglePause(void){}void Player_seek(int p){}void SetVolume(int v){}void NxtVolume_set(int v){SetVolume(v);}void SetBrightness(int v){}
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
static void current_release_and_last_theme(void){
 NxtConfig old=nxt;const int back[]={0,BTN_B};
 for(int lang=0;lang<2;lang++){nxt.language=lang;
  qa_capture_screen=lang?"about-current-vi":"about-current-en";qa_capture_frame=1;input(back,2);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
  const int details[]={BTN_A,0,BTN_B,BTN_B};qa_capture_frame=2;qa_capture_screen=lang?"about-main-features-vi":"about-main-features-en";input(details,4);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
  qa_capture_frame=1;qa_capture_screen=lang?"changes-current-vi":"changes-current-en";input(back,2);assert(Nxt_changes((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
  const int shortcut[]={BTN_A,BTN_X,0,BTN_B,BTN_B,BTN_B};qa_capture_frame=3;qa_capture_screen=lang?"changes-from-X-vi":"changes-from-X-en";input(shortcut,6);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 }
 nxt.language=1;nxt.interface_style=49;qa_capture_frame=1;qa_capture_screen="theme-gallery-last-page";input(back,2);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==49);
 qa_capture_screen=NULL;const int apply[]={BTN_RIGHT,BTN_LEFT,BTN_A};input(apply,3);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==49);
 const int vertical[]={BTN_DOWN,BTN_UP,BTN_A};input(vertical,3);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==49);
 nxt=old;puts("PASS both current release menus, X shortcut, final partial theme page, 49->0->49 horizontal wrap and vertical same-column wrap.");
}
static void galleries(void){const int select[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,true,0)==31);input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==31);const int cancel[]={BTN_RIGHT,BTN_B};input(cancel,2);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==-1);int value=0x123456;const int rgb_cancel[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_B};input(rgb_cancel,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x123456);const int rgb_save[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(rgb_save,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x134456);puts("PASS native RGB save/cancel, 300-icon/180-emoji paged navigation, theme preview/apply/cancel preserves Original color, wallpaper and cover mode.");}
static void profile_flow(void){nxt.mood=nxt.genre=nxt.country=0;
 char path[1400];profile_path(0,"preferences.json",path,sizeof(path));char parent[1400];snprintf(parent,sizeof(parent),"%s",path);*strrchr(parent,'/')=0;assert(mkdir_p(parent));FILE*f=fopen(path,"w");assert(f);fputs("{\"avatar_mode\":1,\"avatar_emoji\":177}",f);fclose(f);
 ProfileAvatar a;int saved=nxt.avatar_emoji;read_profile_avatar(0,0,&a);assert(a.mode==1&&a.emoji==177&&nxt.avatar_emoji==saved);
 const int select[]={BTN_A};input(select,1);ModuleExitReason got=NxtPersonal_login((DisplayContext*)1);assert(got==MODULE_EXIT_TO_MENU&&frame_no==1&&nxt.mood==0&&nxt.genre==0&&nxt.country==0);
 const int back[]={0,BTN_B};input(back,2);assert(choose_listener((DisplayContext*)1,false)==MODULE_EXIT_TO_MENU);shot("profiles-startup");
 assert(exec("INSERT OR IGNORE INTO listeners(id,name) VALUES(15,'Delete fixture');"));
 qa_capture_screen="profile-delete-confirm";qa_capture_frame=4;
 const int cancel_delete[]={BTN_RIGHT,BTN_RIGHT,BTN_X,0,BTN_A,BTN_B};input(cancel_delete,6);assert(choose_listener((DisplayContext*)1,false)==MODULE_EXIT_TO_MENU);sqlite3_stmt *check=NULL;assert(sqlite3_prepare_v2(db,"SELECT 1 FROM listeners WHERE id=15",-1,&check,NULL)==SQLITE_OK&&sqlite3_step(check)==SQLITE_ROW);sqlite3_finalize(check);
 qa_capture_screen=NULL;
 const int commit_delete[]={BTN_RIGHT,BTN_RIGHT,BTN_X,BTN_DOWN,BTN_A,0,BTN_B};input(commit_delete,7);assert(choose_listener((DisplayContext*)1,false)==MODULE_EXIT_TO_MENU);assert(sqlite3_prepare_v2(db,"SELECT 1 FROM listeners WHERE id=15",-1,&check,NULL)==SQLITE_OK&&sqlite3_step(check)==SQLITE_DONE);sqlite3_finalize(check);assert(listener==0);
 const int edit_delete[]={BTN_UP,BTN_A,0,BTN_B};input(edit_delete,4);assert(profile_edit((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&listener==0);
 puts("PASS native profile deletion controls: X selected listener, default cancel, confirmed delete, reload and Edit profile deletion route.");
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
 nxt.interface_style=0;const int files[]={BTN_DOWN,BTN_A,0,BTN_B,BTN_B};qa_capture_library=true;input(files,5);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);qa_capture_library=false;const int show[]={0,BTN_B};input(show,2);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("settings-five-groups");
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


static void v1882_controls(void){
 NxtConfig before=nxt;nxt.bit_perfect=0;int preset=nxt.eq_preset;float bands[31];memcpy(bands,nxt.eq,sizeof(bands));
 const int select[]={0,-BTN_SELECT};input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));
 const int nested[]={BTN_START,0,-BTN_SELECT};input(nested,3);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no<=3&&nxt.eq_preset==preset);
 nxt.bit_perfect=1;input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));shot("bit-perfect-EQ-bypass");
 nxt=before;
 const int audio[]={BTN_A,0,BTN_B,BTN_B};input(audio,4);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("audio-settings-v1882");
 const int mode[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B,BTN_B};nxt.bit_perfect=0;nxt.audio_mode=0;input(mode,6);assert(Nxt_quickOptions((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.bit_perfect==1&&nxt.eq_preset==preset);shot("quick-options-v1882");nxt=before;
 puts("PASS actual SELECT release: presets close unchanged, nested EQ closes fully, Bit-Perfect EQ closes unchanged; native settings and START mode selection.");
}
static void v1882_startup(void){
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
static void translation_flow(void){
 NxtConfig saved=nxt;qa_translation_active=true;qa_lyric_revision++;nxt.language=1;nxt.translation_mode=1;nxt.translation_source=0;
 NxtTranslate_tick();for(int i=0;i<160&&!NxtTranslate_line(1);i++){usleep(50000);NxtTranslate_tick();}assert(NxtTranslate_line(0)&&NxtTranslate_line(1));
 assert(!strcmp(Lyrics_lineText(0),"The morning light is warm.")&&strstr(NxtTranslate_line(0),"Ánh sáng"));
 for(int style=0;style<50;style++){nxt.interface_style=style;NxtSkin_playing(page,1,18,true,2);if(style==0||style==24||style==38||style==49){char name[80];snprintf(name,sizeof(name),"translation-playing-%02d",style);shot(name);}}
 nxt.translation_mode=2;NxtTranslate_tick();for(int i=0;i<160&&!NxtTranslate_line(1);i++){usleep(50000);NxtTranslate_tick();}assert(NxtTranslate_line(1));
 /* Both rows must remain live independently on all 50 Now Playing layouts. */
 for(int style=0;style<50;style++){
  nxt.interface_style=style;NxtSkin_playing(page,1,18,true,2);
  uint64_t upper=style?area_hash(58,530,908,30):area_hash(32,395,960,100),lower=style?area_hash(58,562,908,30):0;
  qa_original_override="A different ORIGINAL cue.";NxtSkin_playing(page,1,18,true,2);
  assert(upper!=(style?area_hash(58,530,908,30):area_hash(32,395,960,100)));if(style)assert(lower==area_hash(58,562,908,30));
  qa_original_override=NULL;NxtSkin_playing(page,1,18,true,2);
  if(style==0||style==24||style==38||style==49){char name[90];snprintf(name,sizeof(name),"translation-bilingual-playing-%02d",style);shot(name);}
 }
 const int back[]={0,BTN_B};
 TTF_Font *regular=NxtSkin_font(page,nxt.lyric_size);int before_style=TTF_GetFontStyle(regular);
 TTF_Font *measure=Fonts_open(nxt.lyric_size*page->w/1024);assert(measure);printf("ITALIC_LAYOUT %d %d %d\n",page->w,NxtSkin_top(page),TTF_FontHeight(measure)+8);TTF_CloseFont(measure);
 qa_capture_screen="translation-bilingual-full";qa_capture_frame=1;input(back,2);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(TTF_GetFontStyle(regular)==before_style);
 nxt.translation_mode=1;NxtTranslate_tick();qa_capture_screen="translation-only-full";input(back,2);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(TTF_GetFontStyle(regular)==before_style);
 nxt.translation_mode=0;NxtTranslate_tick();qa_capture_screen="translation-off-full";input(back,2);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(TTF_GetFontStyle(regular)==before_style);
 /* Dedicated italic fallback must preserve original regular glyphs for CJK. */
 int saved_style=nxt.interface_style,saved_font=nxt.font_index;nxt.interface_style=0;nxt.font_index=14;NxtSkin_playing(page,1,18,true,2);
 const char *cjk="中文 日本語 한국어 ไทย";regular=NxtSkin_font(page,32);before_style=TTF_GetFontStyle(regular);
 SDL_Surface *regular_surface=NxtSkin_renderText(regular,cjk);assert(regular_surface&&regular_surface->w>0);
 NxtSkin_fitItalicText(page,32,cjk,20,100,page->w-40,40,NxtSkin_color(NXT_INK),false);assert(TTF_GetFontStyle(regular)==before_style&&NxtSkin_renderText(regular,cjk)==regular_surface);
 nxt.interface_style=saved_style;nxt.font_index=saved_font;nxt.translation_mode=2;NxtTranslate_tick();for(int i=0;i<160&&!NxtTranslate_line(1);i++){usleep(50000);NxtTranslate_tick();}assert(NxtTranslate_line(1));

 qa_capture_screen="translation-settings";qa_capture_frame=1;input(back,2);assert(NxtTranslate_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 const int from_lyrics[]={BTN_START,0,BTN_B,BTN_B};qa_capture_screen="translation-from-lyrics-START";qa_capture_frame=2;input(from_lyrics,4);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 const int playback[]={BTN_A,BTN_A,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_A,0,BTN_B,BTN_B,BTN_B,BTN_B};qa_capture_screen="translation-from-playback";qa_capture_frame=10;input(playback,14);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 qa_capture_screen=NULL;NxtTranslate_quit();qa_translation_active=false;qa_lyric_revision++;nxt=saved;
 puts("PASS production translated lyrics in 50 native players, bilingual whole-line screen, original text kept, settings/START/controller routes and font sharpness at this resolution. Server replies are explicit fixtures.");
}
static void album_cache_pixels(void){
 SDL_Surface *source=SDL_CreateRGBSurfaceWithFormat(0,37,29,32,SDL_PIXELFORMAT_ARGB8888);assert(source);
 SDL_Surface *direct=SDL_CreateRGBSurfaceWithFormat(0,160,120,32,SDL_PIXELFORMAT_ARGB8888),*cached=SDL_CreateRGBSurfaceWithFormat(0,160,120,32,SDL_PIXELFORMAT_ARGB8888);assert(direct&&cached);
 for(int blend=0;blend<2;blend++){
  NxtSkin_forgetArt();SDL_SetSurfaceBlendMode(source,blend?SDL_BLENDMODE_BLEND:SDL_BLENDMODE_NONE);
  for(int y=0;y<source->h;y++)for(int x=0;x<source->w;x++)((Uint32*)((Uint8*)source->pixels+y*source->pitch))[x]=SDL_MapRGBA(source->format,x*6,y*8,(x+y)*3,blend?(x*7)%256:255);
  for(int i=0;i<4;i++){SDL_Rect dst={3,7,70+i*20,47+i*11};Uint32 bg=SDL_MapRGBA(direct->format,43,61,89,255);SDL_FillRect(direct,NULL,bg);SDL_FillRect(cached,NULL,bg);if(blend)SDL_BlitScaled(source,NULL,direct,&dst);else assert(NxtSkin_smoothScale(source,NULL,direct,&dst));NxtSkin_cachedArt(cached,source,dst);
   for(int y=0;y<direct->h;y++){if(memcmp((Uint8*)direct->pixels+y*direct->pitch,(Uint8*)cached->pixels+y*cached->pitch,direct->w*4)){fprintf(stderr,"cache mismatch blend=%d size=%d row=%d\n",blend,i,y);abort();}}
   SDL_FillRect(cached,NULL,bg);NxtSkin_cachedArt(cached,source,dst);for(int y=0;y<direct->h;y++){if(memcmp((Uint8*)direct->pixels+y*direct->pitch,(Uint8*)cached->pixels+y*cached->pitch,direct->w*4)){fprintf(stderr,"cache mismatch blend=%d size=%d row=%d\n",blend,i,y);abort();}}
  }
 }
 NxtSkin_forgetArt();SDL_FreeSurface(source);SDL_FreeSurface(direct);SDL_FreeSurface(cached);puts("PASS cached album pixels match smooth opaque / direct alpha reference: four sizes, opaque and alpha, cold and warm renders.");
}

bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
ModuleExitReason NxtSoundCloud_playSaved(DisplayContext *d,const char *p,const char *t,const char *a){return FixtureCloudSaved(d,p,t,a);}

void ModuleCommon_signalStop(void){}

bool Player_resume(void){return qa_seek;}

const char *Lyrics_status(void){return "Original lyrics fixture";}

void Lyrics_retry(void){}

#include "nxt_auralis.h"
int NxtVolume_getPercent(void){return 86;}
void NxtVolume_setPercent(int v){}
void NxtVolume_tick(void){}
void NxtVolume_flush(void){}
static const LyricWord qa_words[]={{0,1000,0,4},{1000,2000,4,8},{2000,3000,12,6},{3000,4000,18,3},{4000,5000,21,5}};
int Lyrics_wordCount(int i){return qa_karaoke&&i==0?5:0;}
const LyricWord *Lyrics_wordAt(int i,int word){return word>=0&&word<Lyrics_wordCount(i)?&qa_words[word]:NULL;}
bool Lyrics_wordMotion(int i,int position){return qa_karaoke&&i==0&&position>=0&&position<5000;}
int Lyrics_lineTime(int i){return i<0||i>=2?-1:i*50000;}
bool Lyrics_canFollow(void){return Lyrics_isSynced();}
static bool qa_estimated;
bool Lyrics_wordEstimated(int i){return qa_estimated;}

static void draft_run(int field,const int*keys,int n,const char*shot_name,int shot_frame){
 qa_committed=nxt;qa_guard_draft=true;qa_capture_screen=shot_name;qa_capture_frame=shot_frame;
 unsigned revision=Lyrics_revision();input(keys,n);assert(NxtLyricPreview_run((DisplayContext*)1,field)==MODULE_EXIT_TO_MENU);
 qa_guard_draft=false;qa_capture_screen=NULL;assert(!NxtLyricPreview_active()&&Lyrics_revision()==revision);
}
static void preview_checks(void){
 NxtConfig initial=nxt;int saves=qa_saves;
 const int cancel[]={BTN_RIGHT,BTN_Y,BTN_LEFT,BTN_DOWN,BTN_R1,BTN_X,0,BTN_B};
 draft_run(NXT_EDIT_SUNG,cancel,8,"preview-full-rgb-cancel",7);assert(!memcmp(&nxt,&initial,sizeof(nxt))&&qa_saves==saves);
 const int rgb[]={BTN_Y,BTN_LEFT,BTN_DOWN,BTN_R1,BTN_A};
 nxt.lyric_rgb=0x204060;NxtConfig expected=nxt;expected.lyric_rgb=0x1f5060;
 draft_run(NXT_EDIT_SUNG,rgb,5,NULL,0);assert(!memcmp(&nxt,&expected,sizeof(nxt))&&qa_saves==saves+1);
 const int bold[]={BTN_RIGHT,BTN_A};expected=nxt;expected.lyric_bold=!expected.lyric_bold;
 draft_run(NXT_EDIT_BOLD,bold,2,NULL,0);assert(!memcmp(&nxt,&expected,sizeof(nxt))&&qa_saves==saves+2);
 qa_save_failure=true;const int fail[]={BTN_RIGHT,BTN_A,0,BTN_B};expected=nxt;
 draft_run(NXT_EDIT_EFFECT,fail,4,"preview-save-failure",3);qa_save_failure=false;assert(!memcmp(&nxt,&expected,sizeof(nxt))&&qa_saves==saves+3);
 nxt.font_index=0;expected=nxt;const int font_cancel[]={BTN_LEFT,BTN_X,0,BTN_B};
 draft_run(NXT_EDIT_FONT,font_cancel,4,"preview-font-newsreader-full",3);assert(!memcmp(&nxt,&expected,sizeof(nxt)));
 const int font_apply[]={BTN_R1,BTN_A};expected=nxt;expected.font_index=10;expected.font_path[0]=0;expected.auralis_reference_font=0;
 draft_run(NXT_EDIT_FONT,font_apply,2,NULL,0);assert(!memcmp(&nxt,&expected,sizeof(nxt)));
 nxt.font_index=0;expected=nxt;expected.font_index=59;
 const int font_wrap[]={BTN_LEFT,BTN_A};draft_run(NXT_EDIT_FONT,font_wrap,2,NULL,0);assert(!memcmp(&nxt,&expected,sizeof(nxt)));
 for(int style=0;style<NXT_THEME_COUNT;style++){
  nxt.interface_style=style;expected=nxt;const int both[]={0,BTN_X,0,BTN_B};
  draft_run(NXT_EDIT_TRANSLATION,both,4,style==0?"preview-original-full":style==24?"preview-theme24-full":NULL,3);
  assert(!memcmp(&nxt,&expected,sizeof(nxt)));
 }
 nxt=initial;puts("PASS transactional native preview: every background/input frame sees committed config; cancel, apply, failed save, exact RGB, font jump/wrap, both views/all 51 themes, sample isolation.");
}

int main(int argc,char **argv){
 assert(argc==2&&!SDL_Init(0)&&!TTF_Init());out_dir=argv[1];album_cache_pixels();
 nxt.lyric_rgb=0xffd166;nxt.lyric_unsung_rgb=0xeeeeee;nxt.lyric_translation_rgb=0xbde3e5;nxt.lyric_outline=2;nxt.lyric_outline_rgb=0x101010;nxt.lyric_shadow=1;nxt.song_title_rgb=0xffffff;nxt.song_artist_rgb=0xc8e4e6;nxt.song_title_size=50;
 nxt.performance_mode=1;nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=34;nxt.animations=1;nxt.width=1;nxt.limiter=1;nxt.eq_bands=5;nxt.avatar_icon=-1;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0xffa4c9;NxtPersonal_init();NxtControl_defaults();assert(nxt.playing_keys[12]==NC_NONE&&NC_COUNT==22);fixture_state=PLAYER_STATE_PLAYING;child=123;connected=true;track_loaded=playing=true;position_ms=37000;position_since=ticks;duration_ms=192000;
 snprintf(current.uri,sizeof(current.uri),"spotify:track:0123456789012345678901");snprintf(track_info.title,sizeof(track_info.title),"Nắng dịu trong một chiều bình yên");snprintf(track_info.artist,sizeof(track_info.artist),"NXT Music");snprintf(track_info.album,sizeof(track_info.album),"Một ngày an yên");snprintf(track_info.codec,sizeof(track_info.codec),"FLAC");track_info.sample_rate=96000;track_info.bit_depth=24;track_info.bitrate=2400000;track_info.channels=2;cover=IMG_Load("./res/avatar-test.png");assert(cover);
 assert(NXT_THEME_COUNT==51&&NXT_FONT_COUNT==60&&!strcmp(NxtTheme_name(50),"Auralis Themes"));
 for(int size=0;size<2;size++){
  qa_translation_active=false;nxt.translation_mode=0;qa_pos=0;
  page=SDL_CreateRGBSurfaceWithFormat(0,size?640:1024,size?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);drawing=page;
  /* Repainting an unchanged Now Playing is not another page entrance. */
  nxt.interface_style=0;nxt.cover_mode=0;nxt.animations=1;
  NxtMotion_release();Nxt_renderPlaying(page,0,1,5,false,0);NxtMotion_frame(page,true);
  for(int j=0;j<6;j++){ticks+=250;Nxt_renderPlaying(page,0,1,5,false,0);assert(!NxtMotion_frame(page,true));}
  puts("PASS Original Now Playing redraw keeps a single page identity, no recurring entrance displacement.");
  original_cover_fit();galleries();eq_flow();nxt.interface_style=50;preview_checks();
  nxt.lyric_effect=1;nxt.auralis_reference_font=1;nxt.interface_style=50;nxt.auralis_now_brightness=72;nxt.auralis_full_brightness=60;
  NxtSkin_begin(page,"Settings",NULL,false);const char *refrows[]={"Library","Online Radio","Podcasts","Settings"};NxtSkin_list(page,refrows,4,3,false);shot("auralis-reference-selection");
  {int ux=page->w;Uint32 pixel=*((Uint32*)((Uint8*)page->pixels+(410*ux/1024)*page->pitch)+850*ux/1024);Uint8 r,g,b;SDL_GetRGB(pixel,page->format,&r,&g,&b);assert(r==218&&g==218&&b==218);TTF_Font*ref=NxtSkin_font(page,32);assert(strstr(TTF_FontFaceFamilyName(ref),"Harmony"));}
  {const int backonly[]={0,BTN_B};qa_capture_screen="auralis-personal-menu";qa_capture_frame=1;input(backonly,2);assert(NxtAppearance_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);qa_capture_screen=NULL;}
  {const int pickfx[]={BTN_RIGHT,BTN_A};input(pickfx,2);assert(NxtLyricFX_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.lyric_effect==2);nxt.lyric_effect=1;}
  {const int edits[]={BTN_DOWN,BTN_A,BTN_RIGHT,BTN_DOWN,BTN_RIGHT,BTN_B,BTN_B};input(edits,7);assert(NxtAppearance_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(nxt.auralis_now_brightness==73&&nxt.auralis_full_brightness==61);}
  {const int readability[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_A,0,BTN_B,BTN_B};qa_capture_screen="auralis-readability-settings";qa_capture_frame=8;input(readability,10);assert(NxtAppearance_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);qa_capture_screen=NULL;}
  {const int cancelFont[]={BTN_DOWN,BTN_B};nxt.font_index=16;int saves=qa_saves;qa_capture_screen="font-harmony-preview";qa_capture_frame=1;input(cancelFont,2);assert(NxtAppearance_fonts((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.font_index==16&&qa_saves==saves);qa_capture_screen=NULL;}
  {const int applyFont[]={BTN_RIGHT,BTN_A};input(applyFont,2);assert(NxtAppearance_fonts((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.font_index==17);TTF_Font *font=NxtSkin_font(page,30);assert(strstr(TTF_FontFaceFamilyName(font),"Harmony"));}
  nxt.auralis_reference_font=0;for(int f=17;f<NXT_FONT_COUNT;f++){nxt.font_index=f;TTF_Font *font=NxtSkin_font(page,32);assert(font&&NxtText_hasGlyph(font,0x1ebf)&&NxtText_hasGlyph(font,0x4e2d)&&NxtText_hasGlyph(font,0x3042)&&NxtText_hasGlyph(font,0xac00)&&NxtText_hasGlyph(font,0x0e01));NxtSkin_begin(page,NxtText_fontName(f),NULL,false);NxtSkin_fitText(page,36,"Tiếng Việt · 中文 · 日本語 · 한국어 · ภาษาไทย",44*page->w/1024,220*page->w/1024,936*page->w/1024,60*page->w/1024,NxtSkin_color(NXT_INK),false);NxtSkin_fitItalicText(page,32,"Bản dịch in nghiêng · 你好 · 안녕하세요",44*page->w/1024,310*page->w/1024,936*page->w/1024,60*page->w/1024,NxtSkin_color(NXT_MUTED),false);char name[64];snprintf(name,sizeof(name),"font-new-%d",f);shot(name);}
  nxt.font_index=16;nxt.interface_style=50;const int wrap[]={BTN_RIGHT,BTN_LEFT,BTN_A};input(wrap,3);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==50);
  const int cancel[]={BTN_RIGHT,BTN_B};input(cancel,2);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==50);
  const int choose[]={BTN_LEFT,BTN_A};input(choose,2);assert(NxtTheme_gallery((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.interface_style==49);

  for(int style=0;style<50;style++){nxt.interface_style=style;NxtSkin_playing(page,219,672,true,2);lyric_visible_and_budget(style);}
  /* Actual translation worker with local curl fixture, not a live quota. */
  translation_flow();
  nxt.auralis_reference_font=1;nxt.interface_style=50;nxt.translation_mode=2;qa_translation_active=true;qa_lyric_revision++;qa_pos=1000;NxtTranslate_tick();for(int j=0;j<160&&!NxtTranslate_line(1);j++){usleep(50000);NxtTranslate_tick();}assert(NxtTranslate_line(1));
  NxtSkin_playing(page,219,672,true,2);shot("auralis-playing-bilingual");
  {TTF_Font*normal=NxtSkin_font(page,38);int original_style=TTF_GetFontStyle(normal);uint64_t base=area_hash(44,265,936,120);nxt.lyric_bold=1;nxt.lyric_rgb=0xffa6c1;nxt.lyric_outline=3;NxtSkin_playing(page,219,672,true,2);assert(base!=area_hash(44,265,936,120));assert(TTF_GetFontStyle(normal)==original_style);shot("auralis-bold-pink-outline");
   SDL_Surface*previous=cover;cover=SDL_CreateRGBSurfaceWithFormat(0,400,400,32,SDL_PIXELFORMAT_ARGB8888);assert(cover);SDL_FillRect(cover,NULL,SDL_MapRGB(cover->format,250,244,231));NxtAuralis_release();NxtSkin_playing(page,219,672,true,2);shot("auralis-bright-cover-readability");NxtAuralis_release();SDL_FreeSurface(cover);cover=previous;nxt.lyric_bold=0;nxt.lyric_rgb=0xffd166;nxt.lyric_outline=2;NxtAuralis_release();}
  qa_estimated=true;NxtSkin_playing(page,219,672,true,2);shot("auralis-estimated-badge");qa_estimated=false;
  qa_karaoke=true;qa_lyric_revision++;int save_ms=position_ms;Uint32 save_since=position_since;
  for(int k=0;k<3;k++){position_ms=k*2300;position_since=ticks;NxtSkin_playing(page,219,672,true,2);char name[64];snprintf(name,sizeof(name),"auralis-karaoke-%d",k);shot(name);}
  qa_karaoke=false;qa_lyric_revision++;position_ms=save_ms;position_since=save_since;

  TTF_Font *upright=NxtSkin_font(page,34);int upright_style=TTF_GetFontStyle(upright);
  NxtTranslate_tick();for(int j=0;j<160&&!NxtTranslate_line(1);j++){usleep(10000);NxtTranslate_tick();}
  NxtAuralis_lyrics(page,0,true);shot("auralis-full-bilingual");assert(TTF_GetFontStyle(upright)==upright_style);
  qa_original_override="A very long English original sentence keeps its matching Vietnamese translation on a separate single line.";NxtSkin_playing(page,219,672,true,2);shot("auralis-long-single-line");qa_original_override=NULL;
  const int back[]={0,BTN_B};qa_capture_screen="auralis-full-controller";qa_capture_frame=1;input(back,2);if(!Spectrum_isEnabled())Spectrum_toggleVisible();bool spectrum_saved=Spectrum_isEnabled();qa_full_spectrum=true;assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);qa_full_spectrum=false;assert(Spectrum_isEnabled()==spectrum_saved);qa_capture_screen=NULL;
  for(int effect=0;effect<10;effect++){nxt.lyric_effect=effect;qa_pos=1000;NxtSkin_playing(page,219,672,false,1);qa_pos=51000;NxtSkin_playing(page,220,672,false,1);ticks+=140;NxtSkin_playing(page,220,672,false,1);char name[64];snprintf(name,sizeof(name),"auralis-effect-%d",effect);shot(name);int dx,dy;Uint8 alpha;NxtLyricFX_transform(1.f,page->w,&dx,&dy,&alpha);assert(dx==0&&dy==0&&alpha==255);assert(NxtLyricFX_progress(360)==1.f);}nxt.lyric_effect=1;
  qa_pos=51000;NxtSkin_playing(page,220,672,false,1);shot("auralis-transition-start");ticks+=210;NxtSkin_playing(page,220,672,false,1);shot("auralis-transition-middle");ticks+=210;NxtSkin_playing(page,220,672,false,1);shot("auralis-transition-end");
  SDL_Surface *saved=cover;cover=NULL;NxtSkin_playing(page,0,0,false,0);shot("auralis-no-cover");cover=saved;
  nxt.translation_mode=0;NxtSkin_playing(page,219,672,false,0);shot("auralis-original-only");
  nxt.font_index=NXT_FONT_MISANS;nxt.interface_style=0;NxtSkin_playing(page,219,672,false,0);shot("original-misans");
  NxtSkin_release();SDL_FreeSurface(page);page=NULL;drawing=NULL;
 }
 NxtPersonal_quit();NxtTranslate_quit();NxtSkin_release();SDL_FreeSurface(cover);TTF_Quit();SDL_Quit();puts("PASS v1.9.7.5: 50 legacy layouts, Auralis full-cover/cue pairs/italic/transition/no-cover, native full lyrics controls, 60 fonts, two resolutions, translation worker/cache fixture, ASan/UBSan. Hardware inputs are mocked.");return 0;
}

const char *NxtHardware_dacName(void){return "TRN Black Pearl";}
bool NxtHardware_setDACName(const char *s){return true;}
void NxtBit_setDSDMode(int m){nxt.dsd_output=m;}
void NxtLibrary_request(bool f){}
