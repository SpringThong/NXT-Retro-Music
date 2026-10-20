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
static Uint32 clock_ms;static int raw_percent,saves;
int GetVolume(void){return level;}
void SetRawVolume(int v){raw_percent=v;}
Uint32 SDL_GetTicks(void){return clock_ms;}
bool Nxt_save(void){saves++;return true;}
bool NxtBit_isDSD(void){return false;}
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
 NxtVolume_release();assert(NxtVolume_gain()==1.f);
 sink=0;nxt.volume_percent[0]=85;NxtVolume_restore();assert(raw_percent==85&&level==17&&NxtVolume_getPercent()==85);
 NxtVolume_setPercent(NxtVolume_getPercent()+1);assert(raw_percent==86&&level==18&&NxtVolume_getPercent()==86);
 NxtVolume_setPercent(NxtVolume_getPercent()+1);assert(raw_percent==87&&level==18&&NxtVolume_getPercent()==87);
 NxtVolume_setPercent(NxtVolume_getPercent()-1);assert(raw_percent==86&&NxtVolume_getPercent()==86);
 sink=1;nxt.volume_percent[1]=43;NxtVolume_restore();assert(raw_percent==43&&NxtVolume_getPercent()==43);sink=0;NxtVolume_restore();assert(raw_percent==86&&NxtVolume_getPercent()==86);
 NxtVolume_flush();int saved=saves;NxtVolume_setPercent(87);clock_ms+=599;NxtVolume_tick();assert(saves==saved);clock_ms++;NxtVolume_tick();assert(saves==saved+1);
 level=8;assert(NxtVolume_getPercent()==40);NxtVolume_setPercent(-1);assert(raw_percent==0&&level==0);NxtVolume_setPercent(1);assert(raw_percent==1&&level==1);NxtVolume_setPercent(101);assert(raw_percent==100&&level==20);
 puts("PASS 85->86->87 fine raw output, independent sinks, external ROM reconciliation, clamping/mute and debounced save.");puts("PASS real volume controller: hardware PCM control selected, all channels/mute, clamping, cached mixer/card change; write/missing control falls back to Normal with live software gain; no forced-DAC warning. ALSA and ROM are fixtures.");
}
