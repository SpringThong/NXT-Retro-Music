#include "nxt.h"
#include "nxt_audio_mode.h"
#include "nxt_bitperfect.h"
#include "player.h"
#include "nxt_vocal.h"
#include "toast.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
NxtConfig nxt;
static int card=-1,switches,load_status=0;static bool ready;
static PlayerState state=PLAYER_STATE_PLAYING;
static char current[512]="song.flac",raw[512]="song.flac",error[256];
const char *Nxt_text(const char*a,const char*b){return a;}
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
bool Nxt_save(void){return true;}
void NxtVolume_release(void){}void NxtVolume_routeChanged(void){}bool NxtVolume_hardware(void){return false;}
bool NxtDrive_isPath(const char *p){return p&&(!strncmp(p,"nxtdrive://",11)||strstr(p,"/nxt-retro-music/drive/cache/"));}
int NxtHardware_dacCard(void){return card;}
const char *Player_getCurrentFile(void){return current;}
PlayerState Player_getState(void){return state;}
PlayerState NxtBit_state(void){return state;}
bool NxtBit_ready(void){return ready;}
int NxtBit_loadStatus(void){return load_status;}
const char *NxtBit_error(void){return error;}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t n){memset(i,0,sizeof(*i));snprintf(p,n,"%s",raw);return 1;}
void Player_switchBitMode(bool bit,bool reload,int override){if(nxt.bit_perfect!=(int)bit)switches++;nxt.bit_perfect=bit;if(override>=0)state=override;}

static int online_kind;
bool Radio_isActive(void){return online_kind==3;}bool Podcast_isActive(void){return online_kind==4;}
bool NxtStartup_preview(void){return false;}const char *NxtStartup_path(void){return current;}
bool NxtSoundCloud_hasMetadata(void){return online_kind==2;}const char *NxtSoundCloud_uri(void){return "https://soundcloud.invalid/audio";}
bool NxtSpotify_hasMetadata(void){return online_kind==1;}const char *NxtSpotify_uri(void){return "spotify:track:fixture";}
int main(void){
 assert(!NxtVocal_enabled());nxt.output=2;card=3;ready=true;load_status=0;state=PLAYER_STATE_PLAYING;
 NxtAudio_setMode(1);assert(nxt.bit_perfect);int before=switches;
 assert(NxtVocal_toggle()&&NxtVocal_enabled()&&!nxt.bit_perfect&&nxt.audio_mode==1&&state==PLAYER_STATE_PLAYING);assert(switches==before+1);
 for(int i=0;i<100;i++)NxtAudio_tick();assert(switches==before+1);assert(NxtVocal_toggle()&&!NxtVocal_enabled()&&nxt.bit_perfect&&nxt.audio_mode==1&&state==PLAYER_STATE_PLAYING);assert(switches==before+2);
 state=PLAYER_STATE_PAUSED;NxtAudio_setMode(2);before=switches;assert(NxtVocal_toggle()&&!nxt.bit_perfect&&nxt.audio_mode==2&&state==PLAYER_STATE_PAUSED);assert(NxtVocal_toggle()&&nxt.bit_perfect&&state==PLAYER_STATE_PAUSED);assert(switches==before+2);
 NxtAudio_setMode(0);before=switches;assert(NxtVocal_toggle()&&NxtVocal_enabled());assert(NxtVocal_toggle()&&!NxtVocal_enabled());assert(switches==before); /* Normal playback does not reopen/reload. */
 assert(NxtVocal_toggle());strcpy(current,"song.dff");strcpy(raw,current);NxtAudio_prepare(current);NxtAudio_tick();assert(nxt.bit_perfect&&NxtVocal_enabled()&&strstr(NxtVocal_status(),"Bypassed"));assert(NxtVocal_toggle()&&!NxtVocal_enabled()&&nxt.bit_perfect);assert(!NxtVocal_toggle()&&!NxtVocal_enabled()&&nxt.bit_perfect);
 for(int provider=1;provider<=4;provider++){online_kind=provider;NxtAudio_streaming();NxtAudio_setMode(1);assert(!nxt.bit_perfect);before=switches;assert(NxtVocal_toggle());NxtAudio_tick();assert(!nxt.bit_perfect&&nxt.audio_mode==1);assert(NxtVocal_toggle());assert(switches==before);}online_kind=0;
 strcpy(current,"track.flac");strcpy(raw,current);for(int output=0;output<2;output++){nxt.output=output;NxtAudio_prepare(current);assert(!nxt.bit_perfect);before=switches;assert(NxtVocal_toggle());assert(NxtVocal_toggle());assert(switches==before);}
 puts("PASS production vocal controller/audio policy: Manual/Auto PCM temporarily Normal then restored; play/pause state retained by route calls; no route reopen in Normal/online/speaker/BT; raw DFF bypass and enable rejection; stale local DSD does not reject active online metadata.");
}
