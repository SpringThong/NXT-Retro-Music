#include "nxt.h"
#include "nxt_audio_mode.h"
#include "nxt_bitperfect.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
NxtConfig nxt;
static int card=-1,switches,load_status=0;static bool ready;
static PlayerState state=PLAYER_STATE_PLAYING;
static char current[512]="song.flac",raw[512]="song.flac",error[256];
const char *Nxt_text(const char*a,const char*b){return a;}
void Toast_show(const char*s,int ms){}
bool Nxt_save(void){return true;}
int NxtHardware_dacCard(void){return card;}
const char *Player_getCurrentFile(void){return current;}
PlayerState Player_getState(void){return state;}
PlayerState NxtBit_state(void){return state;}
bool NxtBit_ready(void){return ready;}
int NxtBit_loadStatus(void){return load_status;}
const char *NxtBit_error(void){return error;}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t n){memset(i,0,sizeof(*i));snprintf(p,n,"%s",raw);return 1;}
void Player_switchBitMode(bool bit,bool reload,int override){if(nxt.bit_perfect!=(int)bit)switches++;nxt.bit_perfect=bit;if(override>=0)state=override;}
int main(void){
 NxtAudio_setMode(2);assert(!nxt.bit_perfect);card=3;nxt.output=2;NxtAudio_routeChanged();NxtAudio_tick();assert(nxt.bit_perfect&&state==PLAYER_STATE_PLAYING);
 ready=true;current[0]=0;NxtAudio_tick();assert(nxt.bit_perfect);strcpy(current,"song.flac");
 ready=false;load_status=-1;strcpy(error,"unsupported exact PCM");NxtAudio_tick();assert(!nxt.bit_perfect&&state==PLAYER_STATE_PLAYING);
 int before=switches;for(int i=0;i<100;i++)NxtAudio_tick();assert(switches==before);
 load_status=0;error[0]=0;NxtAudio_prepare("other.flac");strcpy(raw,"other.flac");assert(nxt.bit_perfect);ready=true;strcpy(current,"other.flac");NxtAudio_tick();
 card=-1;nxt.output=0;NxtAudio_routeChanged();NxtAudio_tick();assert(!nxt.bit_perfect&&nxt.audio_mode==2&&state==PLAYER_STATE_PLAYING);
 card=4;nxt.output=2;NxtAudio_routeChanged();NxtAudio_tick();assert(nxt.bit_perfect);
 NxtAudio_streaming();assert(!nxt.bit_perfect);NxtAudio_tick();assert(!nxt.bit_perfect); /* Never re-open raw for an online provider. */
 NxtAudio_prepare("other.flac");assert(nxt.bit_perfect);nxt.output=1;NxtAudio_routeChanged();NxtAudio_tick();assert(!nxt.bit_perfect);
 nxt.output=2;NxtAudio_setMode(0);assert(!nxt.bit_perfect);
 NxtAudio_setMode(1);assert(nxt.bit_perfect);ready=false;load_status=-1;strcpy(error,"DAC busy");NxtAudio_tick();assert(nxt.bit_perfect); /* Manual never claims a fallback is exact. */
 card=-1;nxt.output=0;NxtAudio_routeChanged();NxtAudio_tick();assert(!nxt.bit_perfect);
 puts("PASS production audio policy: Normal/Manual/Auto, USB attach/detach, raw-ready-before-UI, exact-PCM rejection falls back once, different source/card retry, speaker/jack/Bluetooth Normal, online providers Normal, manual raw errors retained. Transport/state are fixtures.");
}
