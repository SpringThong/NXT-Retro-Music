/* SPDX-License-Identifier: GPL-3.0-only
 * User volume controls stay usable in native USB mode. Hardware mixer changes
 * leave the transmitted PCM untouched; unsupported controls use honest Normal
 * playback. No shell command, sample scaling or mixer probe in a UI tick. */
#include "nxt_volume.h"
#include "nxt_audio_mode.h"
#include "nxt_bitperfect.h"
#include "nxt.h"
#include "msettings.h"
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string.h>
static snd_mixer_t *mixer;
static snd_mixer_elem_t *volume;
static int current_card=-1;
static int software_card=-1,software_percent=100,hardware_gain;
static int observed[3]={-1,-1,-1},external[3]={-1,-1,-1};static Uint32 external_at[3],command_at;static bool command_guard;static bool fine_dirty;static Uint32 fine_changed;
bool NxtVolume_hardware(void){return __atomic_load_n(&hardware_gain,__ATOMIC_RELAXED)!=0;}
void NxtVolume_release(void){if(mixer)snd_mixer_close(mixer);mixer=NULL;volume=NULL;current_card=software_card=-1;__atomic_store_n(&software_percent,100,__ATOMIC_RELAXED);__atomic_store_n(&hardware_gain,0,__ATOMIC_RELAXED);}
void NxtVolume_routeChanged(void){if(nxt.output!=2||current_card<0||current_card!=NxtHardware_dacCard())NxtVolume_release();}
float NxtVolume_gain(void){
#ifdef NXT_ROM_STOCK
 return 1.f; /* Stock already applies its session DAC software fader. */
#else
 return __atomic_load_n(&software_percent,__ATOMIC_RELAXED)/100.f;
#endif
}
static bool hardware_volume(int card,int value){
 if(card<0)return false;
 if(card!=current_card)NxtVolume_release();
 if(!mixer){
  char device[32];snprintf(device,sizeof(device),"hw:%d",card);
  if(snd_mixer_open(&mixer,0)<0){mixer=NULL;return false;}
  if(snd_mixer_attach(mixer,device)<0||snd_mixer_selem_register(mixer,NULL,NULL)<0||snd_mixer_load(mixer)<0){NxtVolume_release();return false;}
  current_card=card;int best=0;
  for(snd_mixer_elem_t *e=snd_mixer_first_elem(mixer);e;e=snd_mixer_elem_next(e)){
   if(!snd_mixer_selem_is_active(e)||!snd_mixer_selem_has_playback_volume(e))continue;
   const char *name=snd_mixer_selem_get_name(e);if(!name||strstr(name,"Capture")||strstr(name,"Mic"))continue;
   int rank=strstr(name,"PCM")?5:strstr(name,"Playback")?4:strstr(name,"Headphone")?3:strstr(name,"Master")?2:1;
   if(rank>best){volume=e;best=rank;}
  }
 }
 if(!volume){NxtVolume_release();return false;}
 long min,max;if(snd_mixer_selem_get_playback_volume_range(volume,&min,&max)<0||max<=min){NxtVolume_release();return false;}
 long level=min+(long long)value*(max-min)/100;
 if(snd_mixer_selem_set_playback_volume_all(volume,level)<0||(snd_mixer_selem_has_playback_switch(volume)&&snd_mixer_selem_set_playback_switch_all(volume,value>0)<0)){NxtVolume_release();return false;}
 return true;
}
/* The ROM ABI remains 0..20. Raw output is genuinely 0..100, so a fine
 * adjustment is applied after the compatible shared level. Never fake 1% OSD
 * while leaving the actual mixer at 5% increments. No PCM math in raw DSD. */
static int active_sink(void){int s=GetAudioSink();return s>=0&&s<3?s:0;}
int NxtVolume_getPercent(void){
 int s=active_sink(),p=nxt.volume_percent[s];return p<0?0:p>100?100:p;
}
void NxtVolume_setPercent(int value){
 if(value<0)value=0;if(value>100)value=100;int s=active_sink(),coarse=(value+4)/5;
 /* A nonzero fine level must not be mistaken for mute by a ROM daemon. */
 if(GetVolume()!=coarse)SetVolume(coarse);
 SetRawVolume(value);
 observed[s]=GetVolume();nxt.volume[s]=observed[s];nxt.volume_percent[s]=value;
 fine_dirty=true;fine_changed=SDL_GetTicks();command_at=fine_changed;command_guard=true;external[s]=-1;
 if(s!=2){NxtVolume_release();return;}
 int card=NxtHardware_dacCard();
 if(!nxt.bit_perfect&&!NxtVolume_hardware()){if(card>=0&&card==software_card)__atomic_store_n(&software_percent,value,__ATOMIC_RELAXED);else NxtVolume_release();return;}
 if(hardware_volume(card,value))__atomic_store_n(&hardware_gain,1,__ATOMIC_RELAXED);
 else{__atomic_store_n(&hardware_gain,0,__ATOMIC_RELAXED);if(nxt.bit_perfect&&NxtBit_isDSD())return;software_card=card;__atomic_store_n(&software_percent,value,__ATOMIC_RELAXED);NxtAudio_volumeFallback();}
}
void NxtVolume_set(int value){if(value<0)value=0;if(value>20)value=20;NxtVolume_setPercent(value*5);}
void NxtVolume_restore(void){int s=active_sink();int value=nxt.volume_percent[s];observed[s]=GetVolume();NxtVolume_setPercent(value);}
void NxtVolume_flush(void){if(fine_dirty){fine_dirty=false;Nxt_save();}}
void NxtVolume_tick(void){
 int s=active_sink(),v=GetVolume();if(v<0)v=0;if(v>20)v=20;Uint32 now=SDL_GetTicks();
 if(command_guard&&(Uint32)(now-command_at)>=700)command_guard=false;
 if(observed[s]<0)observed[s]=v;
 if(v!=observed[s]){
  if(command_guard){int expected=(NxtVolume_getPercent()+4)/5;SetVolume(expected);SetRawVolume(NxtVolume_getPercent());observed[s]=GetVolume();external[s]=-1;}
  else if(external[s]!=v){external[s]=v;external_at[s]=now;}
  else if((Uint32)(now-external_at[s])>=180){observed[s]=v;external[s]=-1;nxt.volume[s]=v;nxt.volume_percent[s]=v*5;fine_dirty=true;fine_changed=now;}
 }else external[s]=-1;
 if(fine_dirty&&(Uint32)(now-fine_changed)>=600)NxtVolume_flush();
}
