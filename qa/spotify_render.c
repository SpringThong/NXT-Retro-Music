#include "soundcloud_stub.inc"
#include "nxt_locale.h"
#include "nxt_spotify.c"
#include "nxt_i18n.h"
#include "ui_layers.h"
#include <assert.h>
NxtConfig nxt;
static bool fixture_display_off,fixture_hidden;
static PlayerState fixture_state;
static TrackInfo fixture_info;
static SDL_Surface *fixture_art;
static char fixture_path[512];
static unsigned fixture_generation;
int PWR_getDisplayOff(void){return fixture_display_off;}
bool NxtSession_isHidden(void){return fixture_hidden;}
unsigned Player_getTrackGeneration(void){return fixture_generation;}
const char *Nxt_text(const char *en,const char *vi){
#ifdef NXT_UI_V18
 return nxt.language==1?vi:NxtLocale_text(en);
#else
 return vi;
#endif
}
void ModuleCommon_setAutosleepDisabled(bool v){}
void Player_stop(void){}
void Player_resumeAudio(void){}
void PlayerModule_markStopped(void){}
bool PlayerModule_isActive(void){return false;}
void Resume_updatePosition(int ms){}
static BackgroundPlayerType bg;
BackgroundPlayerType Background_getActive(void){return bg;}
void Background_setActive(BackgroundPlayerType v){bg=v;}
void Background_stopAll(void){bg=BG_NONE;}
void LOG_note(int level,const char *fmt,...){}
static Uint32 ticks=5000;Uint32 SDL_GetTicks(void){return ticks;}
int userdata_snpath(const char *rel,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),rel);}
ToastToken Toast_show(const char *m,uint32_t t){return 0;}
int GetAudioSink(void){return 2;}
bool NxtHardware_hasDAC(void){return true;}
bool NxtHardware_hasJack(void){return false;}
const char *NxtHardware_output(void){return "USB DAC · Black Pearl";}
const char *NxtHardware_pcm(void){return "DAC: 44.1 kHz / 24-bit PCM · 2ch";}
bool Player_isUSBDACActive(void){return true;}
int Nxt_timerRemaining(void){return 0;}
SDL_Color uintToColour(uint32_t c){return (SDL_Color){(c>>24)&255,(c>>16)&255,(c>>8)&255,c&255};}
uint32_t CFG_getColor(int id){return 0x9de4b6ff;}
void GFX_clear(SDL_Surface *s){SDL_FillRect(s,NULL,0);}
void UiLayer_clear(UiLayer l){}
void PLAT_getBatteryStatusFine(int *charging,int *percent){*charging=0;*percent=90;}
bool PLAT_btIsConnected(void){return false;}
bool PLAT_wifiConnected(void){return true;}
static SDL_Surface *drawing;
void UiLayer_blit(SDL_Surface *s,int x,int y,UiLayer layer){if(drawing)SDL_BlitSurface(s,NULL,drawing,&(SDL_Rect){x,y,0,0});}
int Player_getVisBuffer(int16_t *out,int count){for(int i=0;i<count;i+=2){float t=i/2.f;int16_t v=(int16_t)(9000*sin(t*.04)+6000*sin(t*.15)+4000*sin(t*.38)+2000*sin(t*.76));out[i]=out[i+1]=v;}return count;}
int Settings_getInt(const IntSetting *d){return d->fallback;}
void Settings_setInt(const IntSetting *d,int v){}
void Settings_setBool(const BoolSetting *d,bool v){}
bool Settings_getBool(const BoolSetting *d){return true;}
const BoolSetting SETTING_LYRICS_ENABLED={0};
TTF_Font *Fonts_open(int px){return NxtText_open(NxtText_fontPath(nxt.font_index),px);}
SDL_Surface *Nxt_render(TTF_Font *f,const char *t,SDL_Color c){return NxtText_render(f,t,c);}
PlayerState Player_getState(void){return fixture_state;}
int Player_getPosition(void){return 0;}
int Player_getDuration(void){return 0;}
const TrackInfo *Player_getTrackInfo(void){return &fixture_info;}
const char *Player_getCurrentFile(void){return fixture_path;}
SDL_Surface *Player_getAlbumArt(void){return fixture_art;}
SDL_Surface *album_art_get(void){return NULL;}
bool Nxt_isFavorite(const char *path){return false;}
const ResumeState *Resume_getState(void){return NULL;}
SDL_Surface *IMG_Load(const char *path){
 char file[1300];snprintf(file,sizeof(file),"%s.rgba",path);FILE *f=fopen(file,"rb");if(!f)return NULL;
 uint32_t dim[2];if(fread(dim,4,2,f)!=2){fclose(f);return NULL;}SDL_Surface *s=SDL_CreateRGBSurfaceWithFormat(0,dim[0],dim[1],32,SDL_PIXELFORMAT_ARGB8888);if(s)for(int y=0;y<s->h;y++)if(fread((char*)s->pixels+y*s->pitch,4,s->w,f)!=(size_t)s->w){SDL_FreeSurface(s);s=NULL;break;}
 fclose(f);if(s)SDL_SetSurfaceBlendMode(s,SDL_BLENDMODE_BLEND);return s;
}
int main(int argc,char **argv){
 assert(!SDL_Init(0)&&!TTF_Init());nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.output=2;paths();connected=true;child=123;queue=calloc(2,sizeof(*queue));assert(queue);queue_capacity=queue_count=2;
 strcpy(queue[0].title,"Giai điệu bình minh · 한국어 日本語");strcpy(queue[0].artist,"NXT Demo · 歌手");strcpy(queue[0].album,"Những giai điệu trên đường");strcpy(queue[0].uri,"spotify:track:0123456789012345678901");queue[0].duration=226000;
 select_track(0);event("{\"event\":\"playing\",\"position_ms\":37000}");assert(Lyrics_loadTextFile(argv[2],duration_ms));favorite=true;cover=IMG_Load(argv[3]);assert(cover);
 for(int scale=0;scale<2;scale++){SDL_Surface *canvas=SDL_CreateRGBSurfaceWithFormat(0,scale?640:1024,scale?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(canvas);char path[1200];
 drawing=canvas;NxtSkin_playing(canvas,1,2,true,0);Spectrum_init();for(int frame=0;frame<24;frame++){ticks+=16;Spectrum_update();}Spectrum_paint(UI_LAYER_ANIMATION);Spectrum_quit();snprintf(path,sizeof(path),"%s/spotify-player-%d.bmp",argv[1],canvas->w);assert(!SDL_SaveBMP(canvas,path));
 strcpy(track_info.title,"Một giai điệu bình minh dịu dàng · 한국어 日本語 中文 ภาษาไทย");NxtSkin_playing(canvas,1,2,true,0);snprintf(path,sizeof(path),"%s/spotify-long-title-%d.bmp",argv[1],canvas->w);assert(!SDL_SaveBMP(canvas,path));strcpy(track_info.title,current.title);
 NxtSkin_home(canvas,3);snprintf(path,sizeof(path),"%s/spotify-home-%d.bmp",argv[1],canvas->w);assert(!SDL_SaveBMP(canvas,path));SDL_FreeSurface(canvas);}
 NxtSkin_release();SDL_FreeSurface(cover);cover=NULL;TTF_Quit();SDL_Quit();puts("PASS production Spotify selection + native NXT render with album art, common metadata/footer, actual Unicode fallback, mini player at 1024x768 and 640x480; audio/DAC values are fixtures.");return 0;
}