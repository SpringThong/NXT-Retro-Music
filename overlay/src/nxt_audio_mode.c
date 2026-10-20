/* SPDX-License-Identifier: GPL-3.0-only
 * Saved selection and effective transport are separate. AUTO tries exact
 * integer PCM once per source/DAC, then visibly uses Normal on incompatibility.
 * No failed source may repeatedly reopen the DAC in a UI polling loop. */
#include "nxt_audio_mode.h"
#include "nxt.h"
#include "nxt_bitperfect.h"
#include "player.h"
#include "nxt_drive.h"
#include "nxt_volume.h"
#include "toast.h"
#include "nxt_dsd.h"
#include <string.h>
#include <stdio.h>
extern void Player_switchBitMode(bool bit,bool reload,int state);
static char attempted[512],rejected[512];static int rejected_card=-1,last_raw_state;
static bool streaming,loading,reentrant,software_volume;
static bool online_path(const char *p){return p&&(NxtDrive_isPath(p)||!strncmp(p,"https://",8)||!strncmp(p,"http://",7));}
const char *NxtAudio_modeName(int n){return n==1?"Bit-Perfect":n==2?Nxt_text("Automatic","Tự động"):Nxt_text("Normal","Thường");}
static bool usb(void){return nxt.output==2&&NxtHardware_dacCard()>=0;}
static bool raw_dsd(const char*p){
 return !online_path(p)&&NxtDSD_isPath(p);
}
static bool rejected_source(const char *path){return rejected_card==NxtHardware_dacCard()&&path&&*path&&!strcmp(rejected,path);}
void NxtAudio_routeChanged(void){rejected[0]=0;rejected_card=-1;software_volume=false;NxtVolume_routeChanged();}
void NxtAudio_setMode(int mode){if(mode<0||mode>2)return;nxt.audio_mode=mode;NxtAudio_routeChanged();NxtAudio_tick();Nxt_save();}
void NxtAudio_prepare(const char *path){
 streaming=online_path(path);snprintf(attempted,sizeof(attempted),"%s",path?path:"");loading=true;last_raw_state=PLAYER_STATE_PAUSED;
 bool bit=raw_dsd(path)||(!streaming&&!software_volume&&usb()&&(nxt.audio_mode==NXT_AUDIO_BIT||(nxt.audio_mode==NXT_AUDIO_AUTO&&!rejected_source(path))));
 Player_switchBitMode(bit,false,-1);
}
void NxtAudio_streaming(void){streaming=true;loading=false;attempted[0]=0;if(nxt.bit_perfect)Player_switchBitMode(false,false,-1);}
void NxtAudio_volumeFallback(void){if(raw_dsd(*attempted?attempted:Player_getCurrentFile()))return;software_volume=true;if(nxt.bit_perfect)Player_switchBitMode(false,true,-1);}
void NxtAudio_tick(void){
 if(reentrant)return;reentrant=true;
 char active[512]="";
 if(nxt.bit_perfect){
  PlayerState state=NxtBit_state();if(NxtBit_ready()){TrackInfo info;NxtBit_snapshot(&info,active,sizeof(active));last_raw_state=state;}
  if(nxt.audio_mode==NXT_AUDIO_AUTO&&!raw_dsd(*active?active:attempted)&&NxtBit_loadStatus()<0&&*NxtBit_error()){
   TrackInfo info;char path[512];NxtBit_snapshot(&info,path,sizeof(path));if(!*path)snprintf(path,sizeof(path),"%s",attempted);
   if(*path){snprintf(rejected,sizeof(rejected),"%s",path);rejected_card=NxtHardware_dacCard();snprintf(attempted,sizeof(attempted),"%s",path);loading=true;
    Toast_show(Nxt_text("Automatic: using Normal; exact USB PCM unavailable","Tự động: dùng Thường; PCM USB nguyên bản chưa khả dụng"),2500);
    Player_switchBitMode(false,true,last_raw_state==PLAYER_STATE_PLAYING?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED);
   }
  }
 }
 const char *current=Player_getCurrentFile();if(!nxt.bit_perfect&&loading&&current&&!strcmp(current,attempted))loading=false;
 const char *path=*active?active:loading?attempted:current;
 if(!path)path="";
 bool source=*path&&!streaming&&!online_path(path)&&!software_volume;
 bool wanted=(!streaming&&raw_dsd(path))||(usb()&&source&&(nxt.audio_mode==NXT_AUDIO_BIT||(nxt.audio_mode==NXT_AUDIO_AUTO&&!rejected_source(path))));
 if(wanted!=(nxt.bit_perfect!=0)){
  if(wanted)snprintf(attempted,sizeof(attempted),"%s",path);
  Player_switchBitMode(wanted,true,-1);loading=wanted;last_raw_state=Player_getState();
 }
 reentrant=false;
}
