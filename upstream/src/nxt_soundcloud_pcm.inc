#include "nxt_online_pcm_queue.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * PCM epoch, sample count, SOURCE rate, flags (4 x LE u32); float32 stereo.
 * Only this nonblocking audio callback consumes PCM. Pause retains the queue;
 * the helper applies backpressure without blocking its command reader. */
#include <pthread.h>
#include <stdint.h>
#include <math.h>
#include <samplerate.h>
static pthread_mutex_t pcm_lock=PTHREAD_MUTEX_INITIALIZER;
static int pcm_fd=-1,pcm_count,pcm_used,pcm_rate,pcm_source_rate;
static uint32_t pcm_epoch;
static bool pcm_enabled,pcm_eof;
static SRC_STATE *pcm_src;
static NxtPCMQueue *pcm_queue;
static float pcm_input[2048];
static unsigned pcm_packets;
static double pcm_position;
static uint32_t pcm_u32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void pcm_clear_locked(void){pcm_count=pcm_used=0;pcm_eof=false;pcm_source_rate=0;pcm_position=0;if(pcm_src)src_reset(pcm_src);}
static void pcm_close(void){pthread_mutex_lock(&pcm_lock);NxtPCMQueue_close(pcm_queue);pcm_queue=NULL;if(pcm_fd>=0)close(pcm_fd);pcm_fd=-1;pcm_enabled=false;pcm_clear_locked();if(pcm_src)pcm_src=src_delete(pcm_src);pthread_mutex_unlock(&pcm_lock);}
static bool pcm_open(int fd){pthread_mutex_lock(&pcm_lock);int err=0;pcm_src=src_new(SRC_LINEAR,2,&err);pcm_fd=fd;pcm_queue=NxtPCMQueue_open(fd,16);pcm_enabled=false;pcm_rate=0;pcm_packets=0;pcm_clear_locked();pthread_mutex_unlock(&pcm_lock);return pcm_src!=NULL&&pcm_queue!=NULL;}
static void pcm_begin(void){pthread_mutex_lock(&pcm_lock);pcm_epoch++;if(!pcm_epoch)pcm_epoch++;NxtPCMQueue_reset(pcm_queue,pcm_epoch);pcm_clear_locked();pcm_packets=0;pcm_enabled=true;pthread_mutex_unlock(&pcm_lock);}
static void pcm_enable(bool enable){pthread_mutex_lock(&pcm_lock);pcm_enabled=enable;pthread_mutex_unlock(&pcm_lock);}
int NxtSoundCloud_audioSamples(float *out,int frames,int rate){
 int done=0;unsigned char packet[8208];pthread_mutex_lock(&pcm_lock);
 if(pcm_fd<0||!pcm_src||!pcm_enabled||rate<8000||rate>192000){pthread_mutex_unlock(&pcm_lock);return 0;}
 if(rate!=pcm_rate){pcm_rate=rate;if(pcm_src)src_reset(pcm_src);}
 for(int i=0;i<48&&done<frames;i++){
  if(pcm_used>=pcm_count){
   ssize_t bytes=NxtPCMQueue_pop(pcm_queue,packet,sizeof(packet));if(bytes<=0)break;
   uint32_t epoch=bytes>=16?pcm_u32(packet):0,ns=bytes>=16?pcm_u32(packet+4):0;
   uint32_t sr=bytes>=16?pcm_u32(packet+8):0,flags=bytes>=16?pcm_u32(packet+12):0;
   if(epoch!=pcm_epoch)continue;
   if(bytes==16&&flags==1&&ns==0){pcm_eof=true;break;}
   if(flags||sr<8000||sr>192000||ns<2||ns>2048||(ns&1)||bytes!=16+(ssize_t)ns*4)continue;
   if(pcm_source_rate!=(int)sr){pcm_source_rate=sr;src_reset(pcm_src);}
   pcm_count=ns/2;pcm_used=0;pcm_packets++;
   for(uint32_t k=0;k<ns;k++){uint32_t bits=pcm_u32(packet+16+k*4);float v;memcpy(&v,&bits,4);pcm_input[k]=isfinite(v)?fmaxf(-1,fminf(1,v)):0;}
  }
  SRC_DATA data={.data_in=pcm_input+pcm_used*2,.data_out=out+done*2,.input_frames=pcm_count-pcm_used,.output_frames=frames-done,.src_ratio=(double)rate/pcm_source_rate};
  if(src_process(pcm_src,&data)){pcm_count=pcm_used=0;break;}
  pcm_used+=data.input_frames_used;done+=data.output_frames_gen;
  pcm_position+=data.output_frames_gen*1000.0/rate;
  if(!data.input_frames_used&&!data.output_frames_gen)break;
 }
 pthread_mutex_unlock(&pcm_lock);return done;
}
