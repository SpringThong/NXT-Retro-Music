/* The production worker, with an explicit ALSA hardware/decoder simulator.
 * PCM equality is also tested independently using the real ARM64 decoder. */
#include "nxt_bitperfect.h"
#include "nxt_decoder.h"
#include "nxt_stream.h"
#include "nxt.h"
#include <alsa/asoundlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
struct _snd_pcm {unsigned rate,ch;snd_pcm_format_t format;};
struct _snd_pcm_hw_params {unsigned rate,ch;snd_pcm_format_t format;};
struct _snd_pcm_sw_params {int unused;};
struct NxtDecoder {int frame;TrackInfo info;};
static pthread_mutex_t mock_lock=PTHREAD_MUTEX_INITIALIZER;
static int mock_rate=44100,mock_bits=24,mock_ch=2,mock_busy,mock_bad_rate,mock_opens,mock_closes,mock_writes;
static snd_pcm_format_t mock_format=SND_PCM_FORMAT_S24_3LE;
static unsigned char bytes[65536];static size_t byte_count;
static unsigned epoch=1;
NxtConfig nxt;
const char *Nxt_text(const char *a,const char *b){return a;}
void NxtLog_step(const char *a,const char *b,bool ok,const char *fmt,...){}
unsigned NxtStream_epoch(void){return __atomic_load_n(&epoch,__ATOMIC_RELAXED);}
void NxtDecoder_cancelStreams(void){__atomic_add_fetch(&epoch,1,__ATOMIC_RELAXED);}
NxtDecoder *NxtDecoder_openEpoch(const char *path,int rate,unsigned e){assert(rate==0);if(!strcmp(path,"lossy.mp3"))return NULL;NxtDecoder *d=calloc(1,sizeof(*d));d->info.sample_rate=mock_rate;d->info.bit_depth=mock_bits;d->info.channels=mock_ch;d->info.duration_ms=3000*1000/mock_rate;strcpy(d->info.title,path);return d;}
void NxtDecoder_close(NxtDecoder *d){free(d);}
const TrackInfo *NxtDecoder_info(NxtDecoder *d){return &d->info;}
static int32_t sample(int index,int bits){static const int32_t v[]={INT32_MIN,INT32_MAX,-256,0,256,0x12345600,-0x12345600};uint32_t raw=(uint32_t)v[index%7];if(bits==16)raw&=0xffff0000;if(bits==24)raw&=0xffffff00;return (int32_t)raw;}
int NxtDecoder_readNative(NxtDecoder *d,int32_t *out,int frames){int n=3000-d->frame;if(n>frames)n=frames;if(n<0)n=0;for(int i=0;i<n*d->info.channels;i++)out[i]=sample(d->frame*d->info.channels+i,d->info.bit_depth);d->frame+=n;return n;}
bool NxtDecoder_seekEpoch(NxtDecoder *d,int ms,unsigned e){d->frame=(int)((int64_t)ms*d->info.sample_rate/1000);return d->frame<=3000;}
size_t snd_pcm_hw_params_sizeof(void){return sizeof(struct _snd_pcm_hw_params);}
size_t snd_pcm_sw_params_sizeof(void){return sizeof(struct _snd_pcm_sw_params);}
int snd_pcm_open(snd_pcm_t **out,const char *name,snd_pcm_stream_t stream,int mode){assert(!strncmp(name,"hw:",3)&&stream==SND_PCM_STREAM_PLAYBACK);assert((mode&SND_PCM_NO_AUTO_RESAMPLE)&&(mode&SND_PCM_NO_AUTO_CHANNELS)&&(mode&SND_PCM_NO_AUTO_FORMAT)&&(mode&SND_PCM_NO_SOFTVOL));pthread_mutex_lock(&mock_lock);mock_opens++;int busy=mock_busy;pthread_mutex_unlock(&mock_lock);if(busy)return -EBUSY;*out=calloc(1,sizeof(struct _snd_pcm));return 0;}
snd_pcm_type_t snd_pcm_type(snd_pcm_t *p){return SND_PCM_TYPE_HW;}
int snd_pcm_drop(snd_pcm_t *p){return 0;}
int snd_pcm_close(snd_pcm_t *p){pthread_mutex_lock(&mock_lock);mock_closes++;pthread_mutex_unlock(&mock_lock);free(p);return 0;}
int snd_pcm_hw_params_any(snd_pcm_t *p,snd_pcm_hw_params_t *s){memset(s,0,sizeof(*s));return 0;}
int snd_pcm_hw_params_set_rate_resample(snd_pcm_t *p,snd_pcm_hw_params_t *s,unsigned int v){assert(!v);return 0;}
int snd_pcm_hw_params_set_access(snd_pcm_t *p,snd_pcm_hw_params_t *s,snd_pcm_access_t a){assert(a==SND_PCM_ACCESS_RW_INTERLEAVED);return 0;}
int snd_pcm_hw_params_set_format(snd_pcm_t *p,snd_pcm_hw_params_t *s,snd_pcm_format_t f){if(f!=mock_format)return -EINVAL;s->format=f;return 0;}
int snd_pcm_hw_params_set_channels(snd_pcm_t *p,snd_pcm_hw_params_t *s,unsigned int ch){assert(ch==(unsigned)mock_ch);s->ch=ch;return 0;}
int snd_pcm_hw_params_set_rate(snd_pcm_t *p,snd_pcm_hw_params_t *s,unsigned int rate,int dir){assert(rate==(unsigned)mock_rate&&dir==0);s->rate=rate;return mock_bad_rate?-EINVAL:0;}
int snd_pcm_hw_params_set_period_time_near(snd_pcm_t *p,snd_pcm_hw_params_t *s,unsigned int *v,int *dir){return 0;}
int snd_pcm_hw_params_set_buffer_time_near(snd_pcm_t *p,snd_pcm_hw_params_t *s,unsigned int *v,int *dir){return 0;}
int snd_pcm_hw_params(snd_pcm_t *p,snd_pcm_hw_params_t *s){p->rate=s->rate;p->ch=s->ch;p->format=s->format;return 0;}
int snd_pcm_hw_params_get_rate(const snd_pcm_hw_params_t *s,unsigned int *v,int *dir){*v=s->rate;return 0;}
int snd_pcm_hw_params_get_channels(const snd_pcm_hw_params_t *s,unsigned int *v){*v=s->ch;return 0;}
int snd_pcm_hw_params_get_format(const snd_pcm_hw_params_t *s,snd_pcm_format_t *v){*v=s->format;return 0;}
int snd_pcm_hw_params_get_sbits(const snd_pcm_hw_params_t *s){return mock_bits;}
int snd_pcm_hw_params_get_period_size(const snd_pcm_hw_params_t *s,snd_pcm_uframes_t *v,int *dir){*v=128;return 0;}
int snd_pcm_hw_free(snd_pcm_t *p){return 0;}
int snd_pcm_sw_params_current(snd_pcm_t *p,snd_pcm_sw_params_t *s){return 0;}
int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *p,snd_pcm_sw_params_t *s,snd_pcm_uframes_t v){return 0;}
int snd_pcm_sw_params_set_avail_min(snd_pcm_t *p,snd_pcm_sw_params_t *s,snd_pcm_uframes_t v){return 0;}
int snd_pcm_sw_params(snd_pcm_t *p,snd_pcm_sw_params_t *s){return 0;}
int snd_pcm_prepare(snd_pcm_t *p){return 0;}
int snd_pcm_delay(snd_pcm_t *p,snd_pcm_sframes_t *v){*v=0;return 0;}
int snd_pcm_wait(snd_pcm_t *p,int timeout){usleep(1000);return 1;}
int snd_pcm_recover(snd_pcm_t *p,int err,int silent){return 0;}
const char *snd_strerror(int err){return err==-EBUSY?"busy":"unsupported";}
snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *p,const void *buffer,snd_pcm_uframes_t frames){
 usleep(500);pthread_mutex_lock(&mock_lock);mock_writes++;if(mock_writes%11==0){pthread_mutex_unlock(&mock_lock);return -EAGAIN;}size_t n=frames>13?13:frames;int width=p->format==SND_PCM_FORMAT_S16_LE?2:p->format==SND_PCM_FORMAT_S24_3LE?3:4;assert(byte_count+n*p->ch*width<=sizeof(bytes));memcpy(bytes+byte_count,buffer,n*p->ch*width);byte_count+=n*p->ch*width;pthread_mutex_unlock(&mock_lock);return n;
}
static void settled(void){usleep(10000);}
static void reset_mock(int bits,int ch,snd_pcm_format_t format){NxtBit_stop();settled();pthread_mutex_lock(&mock_lock);mock_rate=44100;mock_bits=bits;mock_ch=ch;mock_format=format;mock_busy=mock_bad_rate=0;mock_opens=mock_closes=mock_writes=0;byte_count=0;pthread_mutex_unlock(&mock_lock);}
int main(void){
 assert(!NxtBit_init());NxtBit_route(3,true);
 const snd_pcm_format_t f[]={SND_PCM_FORMAT_S16_LE,SND_PCM_FORMAT_S24_3LE,SND_PCM_FORMAT_S24_LE,SND_PCM_FORMAT_S32_LE};
 for(int k=0;k<4;k++){
  int bits=k==3?32:k?24:16,ch=k==2?1:2;reset_mock(bits,ch,f[k]);assert(!NxtBit_load("lossless.flac",0,PLAYER_STATE_PAUSED)&&!NxtBit_waitLoaded());assert(NxtBit_ready()&&NxtBit_rate()==44100&&NxtBit_channels()==ch);settled();assert(byte_count==0);NxtBit_play();
  int16_t visbuf[2048];int vn=0;for(int tries=0;tries<100&&NxtBit_state()==PLAYER_STATE_PLAYING;tries++){vn=NxtBit_vis(visbuf,2048);if(vn==2048)break;usleep(1000);}assert(vn==2048);if(ch==1)for(int i=0;i<2048;i+=2)assert(visbuf[i]==visbuf[i+1]);
  for(int n=0;n<200&&NxtBit_state()!=PLAYER_STATE_STOPPED;n++)usleep(5000);assert(NxtBit_state()==PLAYER_STATE_STOPPED&&!NxtBit_ready());int width=k==0?2:k==1?3:4;assert(byte_count==(size_t)3000*ch*width);
  for(int i=0;i<3000*ch;i++){int32_t v=sample(i,bits);int64_t code=k==0?v/65536:k==1||k==2?v/256:v;for(int j=0;j<width;j++)assert(bytes[i*width+j]==(unsigned char)((uint64_t)code>>(8*j)));}
  int opens=mock_opens,writes=mock_writes;settled();assert(mock_opens==opens&&mock_writes==writes&&NxtBit_vis(visbuf,2048)==0);printf("PASS exclusive native format %d: exact bytes, partial writes/EAGAIN, paused silence, EOF and idle worker\n",k);
 }
 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);NxtBit_route(-1,true);assert(!NxtBit_load("lossless.flac",0,PLAYER_STATE_PAUSED)&&NxtBit_waitLoaded()==-1);assert(!NxtBit_ready()&&NxtBit_state()==PLAYER_STATE_PAUSED&&mock_opens==0);NxtBit_route(3,true);assert(!NxtBit_waitLoaded());NxtBit_play();usleep(10000);NxtBit_pause();settled();int writes=mock_writes;settled();assert(mock_writes==writes&&NxtBit_state()==PLAYER_STATE_PAUSED);NxtBit_seek(20);settled();assert(NxtBit_position()==20);NxtBit_play();usleep(10000);NxtBit_route(-1,true);settled();assert(NxtBit_state()==PLAYER_STATE_PAUSED&&!NxtBit_ready());puts("PASS connect/disconnect, pause/seek/resume without speaker fallback");
 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);NxtBit_route(3,true);mock_busy=1;assert(!NxtBit_load("lossless.flac",0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&mock_opens==1&&NxtBit_state()==PLAYER_STATE_PAUSED);puts("PASS busy DAC pauses; no plug/dmix fallback");
 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);mock_bad_rate=1;assert(!NxtBit_load("lossless.flac",0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&!NxtBit_ready());puts("PASS incompatible rate rejected without resampling");
 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);assert(!NxtBit_load("lossy.mp3",0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&mock_opens==0);puts("PASS incompatible source rejected before hardware open");
 NxtBit_quit();puts("PASS graceful worker termination; ALSA and source in this test are explicit simulators");return 0;
}
