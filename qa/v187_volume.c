/* Real volume controller; ALSA card/mixer and ROM settings are fixtures. */
#include "nxt.h"
#include "nxt_volume.h"
#include "nxt_audio_mode.h"
#include <alsa/asoundlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
struct _snd_mixer {int live;};struct _snd_mixer_elem {int index;};
static struct _snd_mixer mixer_object;static struct _snd_mixer_elem elems[3]={{0},{1},{2}};
static int card=3,sink=2,level=8,opens,closes,writes,failures,hardware=1,chosen=-1,fallbacks;
static long last_level;static int switch_state=-1;
NxtConfig nxt;
int GetAudioSink(void){return sink;}void SetVolume(int v){level=v;}int NxtHardware_dacCard(void){return card;}
void NxtAudio_volumeFallback(void){fallbacks++;nxt.bit_perfect=0;}
int snd_mixer_open(snd_mixer_t **m,int flags){opens++;*m=&mixer_object;return 0;}
int snd_mixer_close(snd_mixer_t *m){assert(m==&mixer_object);closes++;return 0;}
int snd_mixer_attach(snd_mixer_t*m,const char*n){assert(!strcmp(n,"hw:3")||!strcmp(n,"hw:4"));return 0;}
int snd_mixer_selem_register(snd_mixer_t*m,struct snd_mixer_selem_regopt*o,snd_mixer_class_t**c){return 0;}
int snd_mixer_load(snd_mixer_t*m){return hardware?0:-1;}
snd_mixer_elem_t *snd_mixer_first_elem(snd_mixer_t*m){return &elems[0];}
snd_mixer_elem_t *snd_mixer_elem_next(snd_mixer_elem_t*e){return e->index==2?NULL:&elems[e->index+1];}
int snd_mixer_selem_is_active(snd_mixer_elem_t*e){return 1;}
int snd_mixer_selem_has_playback_volume(snd_mixer_elem_t*e){return 1;}
const char *snd_mixer_selem_get_name(snd_mixer_elem_t*e){const char*n[]={"Mic Playback Volume","Master","PCM Playback Volume"};return n[e->index];}
int snd_mixer_selem_get_playback_volume_range(snd_mixer_elem_t*e,long*a,long*b){*a=-120;*b=0;return 0;}
int snd_mixer_selem_set_playback_volume_all(snd_mixer_elem_t*e,long v){writes++;chosen=e->index;last_level=v;return failures?-1:0;}
int snd_mixer_selem_has_playback_switch(snd_mixer_elem_t*e){return 1;}
int snd_mixer_selem_set_playback_switch_all(snd_mixer_elem_t*e,int v){switch_state=v;return 0;}
int main(void){
 nxt.output=2;nxt.bit_perfect=1;NxtVolume_set(10);assert(nxt.bit_perfect&&level==10&&chosen==2&&last_level==-60&&switch_state==1&&opens==1&&!fallbacks);
 NxtVolume_set(0);assert(nxt.bit_perfect&&last_level==-120&&switch_state==0&&opens==1);NxtVolume_set(22);assert(level==20&&last_level==0&&switch_state==1);
 nxt.bit_perfect=0;NxtVolume_routeChanged();assert(NxtVolume_hardware());NxtVolume_set(8);assert(NxtVolume_hardware()&&last_level==-72&&opens==1);nxt.bit_perfect=1;
 card=4;NxtVolume_set(5);assert(opens==2&&closes==1&&level==5&&last_level==-90);
 failures=1;NxtVolume_set(12);assert(!nxt.bit_perfect&&fallbacks==1&&level==12&&closes==2);
#ifdef NXT_ROM_STOCK
 assert(NxtVolume_gain()==1.f); /* Stock's pre-existing software fader applies gain. */
#else
 assert(NxtVolume_gain()>.599f&&NxtVolume_gain()<.601f);
#endif
 int old_writes=writes;NxtVolume_set(8);assert(writes==old_writes&&fallbacks==1&&level==8);
#ifndef NXT_ROM_STOCK
 assert(NxtVolume_gain()>.399f&&NxtVolume_gain()<.401f);
#endif
 sink=0;NxtVolume_set(6);assert(NxtVolume_gain()==1.f&&writes==old_writes);sink=2;card=3;failures=0;hardware=0;nxt.bit_perfect=1;NxtVolume_set(11);assert(!nxt.bit_perfect&&fallbacks==2&&level==11);
 NxtVolume_release();assert(NxtVolume_gain()==1.f);puts("PASS real volume controller: hardware PCM control selected, all channels/mute, clamping, cached mixer/card change; write/missing control falls back to Normal with live software gain; no forced-DAC warning. ALSA and ROM are fixtures.");
}
