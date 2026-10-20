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
#include "v15_ui_render.c"
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
static const int *sequence;static int sequence_count;
void ModuleCommon_frameBegin(void){assert(++frame_no<800);pressed_keys=frame_no<=sequence_count?sequence[frame_no-1]:BTN_B;released_keys=0;ticks+=125;}
void ModuleCommon_frameEnd(SDL_Surface *s){}
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
static void galleries(void){const int select[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,true,0)==31);input(select,4);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==31);const int cancel[]={BTN_RIGHT,BTN_B};input(cancel,2);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==-1);int value=0x123456;const int rgb_cancel[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_B};input(rgb_cancel,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x123456);const int rgb_save[]={BTN_RIGHT,BTN_DOWN,BTN_R1,BTN_A};input(rgb_save,4);assert(NxtCustom_rgb((DisplayContext*)1,"RGB",&value)==MODULE_EXIT_TO_MENU&&value==0x134456);puts("PASS native RGB save/cancel, 300-icon/180-emoji paged navigation, theme preview/apply/cancel preserves Original color, wallpaper and cover mode.");}
static void profile_flow(void){nxt.mood=nxt.genre=nxt.country=0;
 char path[1400];profile_path(0,"preferences.json",path,sizeof(path));char parent[1400];snprintf(parent,sizeof(parent),"%s",path);*strrchr(parent,'/')=0;assert(mkdir_p(parent));FILE*f=fopen(path,"w");assert(f);fputs("{\"avatar_mode\":1,\"avatar_emoji\":177}",f);fclose(f);
 ProfileAvatar a;int saved=nxt.avatar_emoji;read_profile_avatar(0,0,&a);assert(a.mode==1&&a.emoji==177&&nxt.avatar_emoji==saved);
 const int select[]={BTN_A,BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B};input(select,8);ModuleExitReason got=NxtPersonal_login((DisplayContext*)1);assert(got==MODULE_EXIT_TO_MENU&&nxt.mood==1&&nxt.genre==1&&nxt.country==1);
 const int back[]={0,BTN_B};input(back,2);assert(choose_listener((DisplayContext*)1,false)==MODULE_EXIT_TO_MENU);shot("profiles-startup");
 const int choices[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_DOWN,BTN_A};input(choices,6);assert(NxtPersonal_checkIn((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.mood==2&&nxt.genre==2&&nxt.country==2);
 puts("PASS saved startup avatar, no global-avatar mutation, fresh entry check-in and mood/genre/country commit.");
}
static void eq_flow(void){nxt.eq_preset=0;const int rock[]={BTN_RIGHT,BTN_RIGHT,BTN_A};input(rock,3);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.eq_preset==2&&nxt.eq_bands==5&&nxt.eq[0]==3);nxt.eq_preset=12;const int custom[]={BTN_A,BTN_UP,BTN_RIGHT,BTN_DOWN,BTN_B,BTN_B};input(custom,6);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.eq_preset==12&&nxt.eq_bands==5&&nxt.eq[0]==3.5f&&nxt.eq[1]==.5f);NxtEQ_drawCustom(page,1);shot("eq-custom-five");NxtEQ_drawPresets(page,12);shot("eq-presets");for(int i=0;i<12;i++){NxtEQ_apply(i);assert(nxt.eq_bands==5&&nxt.preamp<=0&&nxt.preamp>=-6);for(int k=0;k<5;k++)assert(nxt.eq[k]>=-12&&nxt.eq[k]<=12);}puts("PASS SELECT preset grid -> apply Rock / Custom five sliders, actual controller gain edits and all 12 DSP presets.");}
static void icons_and_emoji(void){for(int i=0;i<180;i++){char p[128];snprintf(p,sizeof(p),"./res/emoji3d/%03d.png",i);SDL_Surface *im=IMG_Load(p);assert(im&&im->w==160&&im->h==160);SDL_FreeSurface(im);}for(int i=0;i<300;i++){char p[180];snprintf(p,sizeof(p),"./res/icons/%s.png",NxtCustom_iconName(i));SDL_Surface *im=IMG_Load(p);assert(im);SDL_FreeSurface(im);}
 const int last_icons[]={BTN_LEFT,BTN_A};input(last_icons,2);assert(NxtCustom_gallery((DisplayContext*)1,false,0)==299);input(last_icons,2);assert(NxtCustom_gallery((DisplayContext*)1,true,0)==179);
SDL_FillRect(page,NULL,SDL_MapRGB(page->format,20,25,28));for(int i=0;i<64;i++){int x=(22+i%8*125)*page->w/1024,y=(15+i/8*89)*page->w/1024;char name[32];snprintf(name,sizeof(name),"emoji-%d",i);NxtSkin_icon(page,name,x,y,55*page->w/1024,NxtSkin_color(NXT_INK));SDL_Surface *glyph=NxtSkin_renderText(NxtSkin_font(page,56),NxtCustom_emoji(i));assert(glyph&&glyph->w>0);utf8proc_int32_t cp;assert(utf8proc_iterate((const utf8proc_uint8_t*)NxtCustom_emoji(i),-1,&cp)>0&&NxtText_hasGlyph(NxtSkin_font(page,56),cp));NxtSkin_text(page,15,NxtCustom_emoji(i),x,y+58*page->w/1024,110*page->w/1024,NxtSkin_color(NXT_MUTED));}shot("emoji-64");SDL_FillRect(page,NULL,SDL_MapRGB(page->format,20,25,28));for(int i=0;i<64;i++){int x=(22+i%8*125)*page->w/1024,y=(15+i/8*89)*page->w/1024;char path[220];snprintf(path,sizeof(path),"./res/icons/%s.png",NxtCustom_iconName(i));SDL_Surface *icon=IMG_Load(path);assert(icon);SDL_FreeSurface(icon);NxtSkin_icon(page,NxtCustom_iconName(i),x,y,55*page->w/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(page,14,NxtCustom_iconLabel(i),x,y+58*page->w/1024,115*page->w/1024,NxtSkin_color(NXT_MUTED));}shot("icons-64");nxt.avatar_mode=1;nxt.avatar_emoji=12;for(int i=0;i<7;i++)nxt.menu_icons[i]=301+i;NxtSkin_home(page,4);shot("home-personal-emoji");nxt.avatar_mode=0;nxt.avatar_icon=23;NxtSkin_home(page,4);shot("home-personal-migrated-icon");nxt.avatar_mode=2;snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"NXT");NxtSkin_home(page,4);shot("home-personal-text");nxt.avatar_mode=3;snprintf(nxt.avatar_path,sizeof(nxt.avatar_path),"./res/avatar-test.png");NxtSkin_home(page,4);shot("home-personal-photo");nxt.avatar_mode=0;memset(nxt.menu_icons,0,sizeof(nxt.menu_icons));}

#include <time.h>
static uint64_t area_hash(int xx,int yy,int ww,int hh){int x=xx*page->w/1024,y=yy*page->w/1024,w=ww*page->w/1024,h=hh*page->w/1024;uint64_t hash=1469598103934665603ULL;for(int j=y;j<y+h;j++){Uint32 *p=(Uint32*)((Uint8*)page->pixels+j*page->pitch);for(int i=x;i<x+w;i++)hash=(hash^p[i])*1099511628211ULL;}return hash;}
static uint64_t lyric_hash(int style){return area_hash(43,555,938,42);}
static void audio_info_flow(void){
 NxtSkin_playing(page,1,18,true,2);uint64_t src=area_hash(43,730,938,16),out=area_hash(43,750,938,16);TrackInfo saved=track_info;
 snprintf(track_info.codec,sizeof(track_info.codec),"FLAC");track_info.sample_rate=96000;track_info.bit_depth=24;track_info.bitrate=2400000;track_info.channels=2;NxtSkin_playing(page,1,18,true,2);assert(src!=area_hash(43,730,938,16)&&out==area_hash(43,750,938,16));src=area_hash(43,730,938,16);qa_output="DAC: Fixture DAC 2";qa_pcm="DAC: 48 kHz / 32-bit PCM · 2ch";NxtSkin_playing(page,1,18,true,2);assert(src==area_hash(43,730,938,16)&&out!=area_hash(43,750,938,16));shot("actual-source-output-separate");track_info=saved;qa_output="USB DAC · Black Pearl";qa_pcm="DAC: 44.1 kHz / 24-bit PCM · 2ch";
 puts("PASS native audio info lanes: source codec/rate/bitrate/channels and actual DAC/PCM change independently; hardware inputs are fixtures.");
}
static void new_menu_flows(void){
 nxt.interface_style=0;const int show[]={0,BTN_B};input(show,2);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("settings-five-groups");
 input(show,2);assert(NxtAppearance_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("personal-appearance-four-groups");
 input(show,2);assert(NxtAppearance_original((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("fine-tuning-original");
 input(show,2);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("about-update-second");
 input(show,2);assert(profile_edit((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("edit-profile-clean");
 int mode=nxt.cover_mode;const int cancel[]={0,BTN_RIGHT,0,BTN_B};input(cancel,4);assert(NxtAppearance_album((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.cover_mode==mode);shot("album-preview-cancel");
 const int apply[]={0,BTN_RIGHT,0,BTN_A};input(apply,4);assert(NxtAppearance_album((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.cover_mode!=mode);shot("album-preview-apply");nxt.cover_mode=mode;
 const int font[]={BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_DOWN,BTN_RIGHT,BTN_B};int size=nxt.font_size;input(font,6);assert(NxtAppearance_original((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.font_size==size+5);nxt.font_size=size;
 input(show,2);assert(Nxt_lyricsScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("full-lyrics-one-line");
 const int updates[]={BTN_DOWN,BTN_A,0,BTN_B,BTN_B};input(updates,5);assert(Nxt_about((DisplayContext*)1)==MODULE_EXIT_TO_MENU);
 const int wifi[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B,BTN_B};input(wifi,6);int calls=qa_wifi_calls;assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_wifi_calls==calls+1);
 audio_info_flow();
 puts("PASS native v1.8.1 settings, personal-appearance hub, clean profile editor, merged update route, Original font edit, live album preview apply/cancel and full lyric screen.");
}
static void lyric_visible_and_budget(int style){int animations=nxt.animations;nxt.animations=0;qa_lyrics_variant=1;NxtSkin_playing(page,1,18,true,2);uint64_t first=lyric_hash(style);qa_lyrics_variant=2;NxtSkin_playing(page,1,18,true,2);assert(first!=lyric_hash(style));uint64_t stable=lyric_hash(style);ticks+=15000;NxtSkin_playing(page,1,18,true,2);assert(stable==lyric_hash(style));qa_lyrics_variant=0;nxt.animations=animations;clock_t start=clock();for(int n=0;n<8;n++)NxtSkin_playing(page,1,18,true,2);printf("RENDER_BUDGET %d %d %.3f ms per cached paint (host)\n",style,page->w,1000.*(clock()-start)/CLOCKS_PER_SEC/8);}

int main(int argc,char **argv){assert(argc==2&&!SDL_Init(0)&&!TTF_Init());out_dir=argv[1];nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.animations=1;nxt.cover_mode=1;nxt.width=1;nxt.limiter=1;nxt.eq_bands=10;nxt.avatar_icon=-1;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0xffa4c9;NxtPersonal_init();NxtControl_defaults();fixture_state=PLAYER_STATE_PLAYING;child=123;connected=true;track_loaded=playing=true;position_ms=37000;position_since=ticks;duration_ms=192000;snprintf(current.uri,sizeof(current.uri),"spotify:track:0123456789012345678901");snprintf(track_info.title,sizeof(track_info.title),"Nắng dịu trong một chiều bình yên");snprintf(track_info.artist,sizeof(track_info.artist),"NXT Music · Nghệ sĩ thử nghiệm");snprintf(track_info.album,sizeof(track_info.album),"Album · Việt 한국어 日本語");snprintf(track_info.codec,sizeof(track_info.codec),"Ogg Vorbis");track_info.sample_rate=44100;track_info.bit_depth=16;track_info.bitrate=320000;track_info.duration_ms=192000;cover=IMG_Load("./res/avatar-test.png");assert(cover);
 for(int size=0;size<2;size++){page=SDL_CreateRGBSurfaceWithFormat(0,size?640:1024,size?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);drawing=page;original_preserved();galleries();profile_flow();eq_flow();icons_and_emoji();new_menu_flows();for(int i=0;i<36;i++){nxt.interface_style=i;char name[90];snprintf(name,sizeof(name),"theme-%02d-home",i);NxtSkin_home(page,3);shot(name);snprintf(name,sizeof(name),"theme-%02d-playing",i);NxtSkin_playing(page,1,18,true,2);Spectrum_init();for(int n=0;n<3;n++){ticks+=50;Spectrum_update();}Spectrum_paint(UI_LAYER_ANIMATION);NxtRecord_paint(true);shot(name);Spectrum_quit();lyric_visible_and_budget(i);SDL_Color bg=NxtSkin_color(NXT_BG),ink=NxtSkin_color(NXT_INK);assert(Theme_contrast(bg,ink)>=4.5);}nxt.interface_style=0;nxt.eq_preset=0;for(int color=0;color<21;color++)for(int mode=0;mode<5;mode++){nxt.spectrum_palette=color;nxt.spectrum_mode=mode;Spectrum_init();NxtSkin_playing(page,1,1,false,0);for(int n=0;n<8;n++){ticks+=50;Spectrum_update();}Spectrum_paint(UI_LAYER_ANIMATION);if(mode==1||color==20){char name[80];snprintf(name,sizeof(name),"spectrum-%02d-%d",color,mode);shot(name);}Spectrum_quit();}personal_header(page,0,true);personal_cards(page,0,4);shot("for-you-online");nxt.interface_style=24;for(int lang=0;lang<14;lang++){nxt.language=lang;NxtSkin_home(page,3);char name[80];snprintf(name,sizeof(name),"locale-%02d-home",lang);shot(name);}nxt.language=1;nxt.interface_style=0;input(NULL,0);assert(NxtMonitor_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);SDL_FreeSurface(page);page=NULL;drawing=NULL;}
 for(int effect=0;effect<6;effect++){nxt.led_effect=effect;nxt.led_color=16;nxt.led_rgb=0xff0000;nxt.led_rgb2=0x0000ff;assert(NxtLED_groupRGB(0,1000)<=0xffffff);if(effect==1)assert(NxtLED_groupRGB(1,0)==0xff0000&&NxtLED_groupRGB(4,0)==0x0000ff);if(effect==2)assert(NxtLED_groupRGB(1,1000)!=NxtLED_groupRGB(1,2000));}
 fixture_hidden=true;assert(!NxtSkin_shouldDraw(0,true));fixture_hidden=false;fixture_display_off=true;assert(!NxtSkin_shouldDraw(0,true));fixture_display_off=false;NxtPersonal_quit();NxtSkin_release();SDL_FreeSurface(cover);cover=NULL;TTF_Quit();SDL_Quit();puts("PASS static full-sentence lyrics in all 36 device layouts at both resolutions; 36 native home/player interfaces, reserved icon/quality/output lanes, 300 icons + 180 3D emoji, 105 spectrum combinations, 6 LED color functions, 640/1024 native surfaces under ASan/UBSan. Audio, account and physical-device inputs are fixtures.");return 0;}
