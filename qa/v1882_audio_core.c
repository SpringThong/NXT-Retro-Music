#define NXT_QA_AUDIO_CORE
#define main NormalBluetooth_fixture_main
#include "v1882_nextui_bluetooth.c"
#undef main
#include "nxt_audio_mode.c"
static char raw_path[512],raw_error[256];static int raw_pos,raw_loads;static PlayerState raw_state;static bool raw_ready;
static int provider_quits;
void NxtSpotify_quit(void){provider_quits++;}void NxtSoundCloud_quit(void){provider_quits++;}
bool NxtDrive_isPath(const char*p){return p&&(!strncmp(p,"nxtdrive://",11)||strstr(p,"/nxt-retro-music/drive/cache/"));}
bool Podcast_isActive(void){return false;}void Podcast_stop(void){}void Podcast_flushProgress(void){}void Radio_stop(void){}
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
SDL_Surface *NxtDecoder_art(const char*p){return NULL;}char *NxtDecoder_lyrics(const char*p){return NULL;}
void SDL_FreeSurface(SDL_Surface *s){assert(!s);}
int NxtBit_init(void){return 0;}void NxtBit_quit(void){}
int NxtBit_load(const char*p,int ms,PlayerState s){snprintf(raw_path,sizeof(raw_path),"%s",p);raw_pos=ms;raw_state=s;raw_ready=true;raw_error[0]=0;raw_loads++;return 0;}
int NxtBit_loadStatus(void){return *raw_error?-1:0;}int NxtBit_waitLoaded(void){return NxtBit_loadStatus();}
void NxtBit_route(int c,bool u){}void NxtBit_play(void){raw_state=PLAYER_STATE_PLAYING;}void NxtBit_pause(void){raw_state=PLAYER_STATE_PAUSED;}
void NxtBit_stop(void){raw_path[0]=0;raw_state=PLAYER_STATE_STOPPED;raw_ready=false;}
void NxtBit_seek(int m){raw_pos=m;}bool NxtBit_seeking(void){return false;}PlayerState NxtBit_state(void){return raw_state;}
int NxtBit_position(void){return raw_pos;}int64_t NxtBit_listened(void){return 0;}bool NxtBit_ready(void){return raw_ready;}
int NxtBit_rate(void){return 44100;}int NxtBit_bits(void){return 24;}int NxtBit_channels(void){return 2;}int NxtBit_vis(int16_t*p,int n){return 0;}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t s){memset(i,0,sizeof(*i));snprintf(i->title,sizeof(i->title),"Source");snprintf(p,s,"%s",raw_path);return 0x40000001u;}
const char *NxtBit_error(void){return raw_error;}const char *NxtBit_status(void){return "explicit raw transport fixture";}float NxtBit_peak(void){return 0;}
int main(void){
 e.running=true;e.loaded=true;e.state=PLAYER_STATE_PLAYING;e.position=81723;e.options=nxt;e.device=1;qa_device=true;
 snprintf(e.visible_path,sizeof(e.visible_path),"%s","song.flac");snprintf(ui_path,sizeof(ui_path),"%s","song.flac");
 usb_card=3;nxt.output=2;NxtAudio_setMode(2);assert(nxt.bit_perfect&&raw_loads==1&&raw_pos==81723&&raw_state==PLAYER_STATE_PLAYING&&!qa_device&&e.device==0);
 Player_update();NxtAudio_tick();assert(nxt.bit_perfect&&raw_loads==1);raw_pos=94567;
 usb_card=-1;nxt.output=0;NxtAudio_routeChanged();NxtAudio_tick();assert(!nxt.bit_perfect&&nxt.audio_mode==2&&e.state==PLAYER_STATE_PLAYING&&e.position==94567&&!strcmp(e.request_path,"song.flac")&&Player_outputRetryPending());
 /* Decoder/output transport remain explicit fixtures: test Play after the
  * normal worker has loaded, not a simulated physical DAC certification. */
 e.loaded=true;e.state=PLAYER_STATE_PAUSED;assert(Player_play()==0&&Player_getState()==PLAYER_STATE_PLAYING);
 usb_card=3;nxt.output=2;NxtAudio_routeChanged();NxtAudio_tick();assert(nxt.bit_perfect&&raw_loads==2);raw_pos=100000;raw_ready=false;raw_state=PLAYER_STATE_PAUSED;strcpy(raw_error,"exact format rejected");NxtAudio_tick();assert(!nxt.bit_perfect&&e.state==PLAYER_STATE_PLAYING&&e.position==100000&&!strcmp(e.request_path,"song.flac"));
 int before=raw_loads;for(int i=0;i<30;i++)NxtAudio_tick();assert(raw_loads==before);
 e.loaded=true;nxt.output=1;NxtAudio_routeChanged();NxtAudio_tick();assert(!nxt.bit_perfect);e.state=PLAYER_STATE_PAUSED;assert(!Player_play());
 nxt.output=2;NxtAudio_setMode(1);assert(nxt.bit_perfect);int quits=provider_quits;NxtAudio_streaming();assert(!nxt.bit_perfect&&provider_quits==quits&&nxt.audio_mode==1&&Player_getPosition()==raw_pos);
 NxtAudio_prepare("nxtdrive://PUBLICID123/audio.flac");assert(!nxt.bit_perfect);NxtAudio_tick();assert(!nxt.bit_perfect);
 NxtAudio_prepare("song.flac");assert(nxt.bit_perfect);e.running=false;
 puts("PASS actual player/controller handoff: shared SDL closed before raw; position/play state survives USB→Normal; Play works after normal load; failed exact PCM retries Normal at same position without reopen loop; Bluetooth policy Normal. Decoder/PCM hardware are fixtures.");
}
