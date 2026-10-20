/* SPDX-License-Identifier: GPL-3.0-only
 * Exclusive, integer USB PCM. The worker owns both decoder and ALSA handle.
 * Frontend calls only update bounded state or signal the condition variable. */
#include "nxt_bitperfect.h"
#include "nxt_decoder.h"
#include "nxt_native_pcm.h"
#include "nxt_stream.h"
#include "nxt.h"
#include "nxt_dsd.h"
#include "nxt_dsd_alsa.h"
#include "nxt_log.h"
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#define BP_BLOCK 1024
#define DSD_BLOCK_MAX_FRAMES 16384
typedef struct {
    pthread_mutex_t lock;pthread_cond_t wake;pthread_t thread;
    bool running,started,done,ready,seeking,usb;
    unsigned request,generation;int card,seek_ms,position,rate,bits,channels,dsd_mode,dsd_active_mode,dsd_format,dsd_rate;bool dsd;
    PlayerState state;char path[512],error[256];TrackInfo info;
    int16_t vis[2048];int vis_count;int64_t listened;float peak;
} BitEngine;
static BitEngine b={.lock=PTHREAD_MUTEX_INITIALIZER,.wake=PTHREAD_COND_INITIALIZER,.card=-1};
static void close_pcm(snd_pcm_t **pcm){if(*pcm){snd_pcm_drop(*pcm);snd_pcm_close(*pcm);*pcm=NULL;}}
static void fail(unsigned request,const char *message){
    pthread_mutex_lock(&b.lock);
    if(request==b.request){b.ready=false;b.done=true;b.seeking=false;b.state=PLAYER_STATE_PAUSED;snprintf(b.error,sizeof(b.error),"%s",message);pthread_cond_broadcast(&b.wake);}
    pthread_mutex_unlock(&b.lock);NxtLog_step("audio","bit-perfect",false,"%s",message);
}
static int open_pcm(snd_pcm_t **result,int card,const TrackInfo *info,NxtNativeFormat *format,char error[256]){
    int last=-ENODEV;
    for(int device=0;device<8;device++){
        char name[64];snprintf(name,sizeof(name),"hw:%d,%d",card,device);snd_pcm_t *pcm=NULL;
        int r=snd_pcm_open(&pcm,name,SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK|SND_PCM_NO_AUTO_RESAMPLE|SND_PCM_NO_AUTO_CHANNELS|SND_PCM_NO_AUTO_FORMAT|SND_PCM_NO_SOFTVOL);
        if(r<0){last=r;if(r==-EBUSY)break;continue;}
        if(snd_pcm_type(pcm)!=SND_PCM_TYPE_HW){close_pcm(&pcm);last=-EINVAL;continue;}
        const snd_pcm_format_t types[]={SND_PCM_FORMAT_S16_LE,SND_PCM_FORMAT_S24_3LE,SND_PCM_FORMAT_S24_LE,SND_PCM_FORMAT_S32_LE};
        for(int f=0;f<4;f++){
            if(NxtNative_precision((NxtNativeFormat)f)<info->bit_depth)continue;
            snd_pcm_hw_params_t *p;snd_pcm_hw_params_alloca(&p);
            r=snd_pcm_hw_params_any(pcm,p);
            if(r>=0)r=snd_pcm_hw_params_set_rate_resample(pcm,p,0);
            if(r>=0)r=snd_pcm_hw_params_set_access(pcm,p,SND_PCM_ACCESS_RW_INTERLEAVED);
            if(r>=0)r=snd_pcm_hw_params_set_format(pcm,p,types[f]);
            if(r>=0)r=snd_pcm_hw_params_set_channels(pcm,p,info->channels);
            if(r>=0)r=snd_pcm_hw_params_set_rate(pcm,p,info->sample_rate,0);
            unsigned period_us=20000;int direction=0;
            if(r>=0)r=snd_pcm_hw_params_set_period_time_near(pcm,p,&period_us,&direction);
            unsigned buffer_us=80000;direction=0;
            if(r>=0)r=snd_pcm_hw_params_set_buffer_time_near(pcm,p,&buffer_us,&direction);
            if(r>=0)r=snd_pcm_hw_params(pcm,p);
            if(r>=0){
                unsigned actual_rate=0,actual_channels=0;snd_pcm_format_t actual_format;
                snd_pcm_hw_params_get_rate(p,&actual_rate,NULL);snd_pcm_hw_params_get_channels(p,&actual_channels);snd_pcm_hw_params_get_format(p,&actual_format);
                int precision=snd_pcm_hw_params_get_sbits(p);
                if(actual_rate!=(unsigned)info->sample_rate||actual_channels!=(unsigned)info->channels||actual_format!=types[f]||precision<info->bit_depth)r=-EINVAL;
            }
            if(r>=0){
                snd_pcm_sw_params_t *sw;snd_pcm_sw_params_alloca(&sw);snd_pcm_uframes_t period=0;snd_pcm_hw_params_get_period_size(p,&period,NULL);
                r=snd_pcm_sw_params_current(pcm,sw);
                if(r>=0)r=snd_pcm_sw_params_set_start_threshold(pcm,sw,1);
                if(r>=0)r=snd_pcm_sw_params_set_avail_min(pcm,sw,period?period:BP_BLOCK);
                if(r>=0)r=snd_pcm_sw_params(pcm,sw);
            }
            if(r>=0)r=snd_pcm_prepare(pcm);
            if(r>=0){*format=(NxtNativeFormat)f;*result=pcm;return 0;}
            last=r;snd_pcm_hw_free(pcm);
        }
        close_pcm(&pcm);
    }
    snprintf(error,256,"%s (%s)",last==-EBUSY?"DAC busy; stop the other audio app":"DAC cannot open exact source rate, channels and sample precision",snd_strerror(last));return -1;
}
static void position_update(unsigned request,snd_pcm_t *pcm,int base,int64_t submitted,int rate){
    snd_pcm_sframes_t delay=0;if(snd_pcm_delay(pcm,&delay)<0||delay<0)delay=0;
    int64_t played=submitted-delay;if(played<0)played=0;
    int position=base+(int)(played*1000/rate);
    pthread_mutex_lock(&b.lock);if(request==b.request){if(b.info.duration_ms>0&&position>b.info.duration_ms)position=b.info.duration_ms;b.position=position;}pthread_mutex_unlock(&b.lock);
}
static void *worker(void *unused){
    (void)unused;NxtDecoder *decoder=NULL;NxtDSD*dsd=NULL;snd_pcm_t *pcm=NULL;unsigned request=0;
    int32_t native[BP_BLOCK*2];uint8_t packed[DSD_BLOCK_MAX_FRAMES*2*4];int count=0,used=0,base=0,play_rate=0,dsd_block=BP_BLOCK;NxtDSDFormat dsd_format=NXT_DSD_U8;int64_t submitted=0;
    NxtNativeFormat format=NXT_PCM_S32;TrackInfo info={0};
    for(;;){
        pthread_mutex_lock(&b.lock);
        while(b.running&&request==b.request&&!b.seeking&&(b.state!=PLAYER_STATE_PLAYING||!b.ready))pthread_cond_wait(&b.wake,&b.lock);
        bool running=b.running,changed=request!=b.request,seeking=b.seeking,usb=b.usb;int card=b.card,seek=b.seek_ms,dsd_mode=b.dsd_mode;char path[512];snprintf(path,sizeof(path),"%s",b.path);request=b.request;
        pthread_mutex_unlock(&b.lock);if(!running)break;
        if(changed){
            close_pcm(&pcm);NxtDecoder_close(decoder);decoder=NULL;NxtDSD_close(dsd);dsd=NULL;count=used=0;submitted=0;base=seek;play_rate=0;
            if(!*path)continue;
            if(NxtDSD_isPath(path)){
                char error[256];dsd=NxtDSD_open(path,error);if(!dsd){fail(request,error);continue;}
                const NxtDSDInfo*di=NxtDSD_info(dsd);memset(&info,0,sizeof(info));
                snprintf(info.title,sizeof(info.title),"%s",di->title);snprintf(info.artist,sizeof(info.artist),"%s",di->artist);snprintf(info.album,sizeof(info.album),"%s",di->album);
                snprintf(info.codec,sizeof(info.codec),"%s DSD%u",di->container,di->rate%44100==0?di->rate/44100:di->rate/48000);info.sample_rate=di->rate;info.bit_depth=1;info.channels=di->channels;info.bitrate=di->rate*di->channels;info.duration_ms=di->duration_ms;
                pthread_mutex_lock(&b.lock);if(request==b.request){b.info=info;b.generation++;b.position=seek;b.dsd=true;b.dsd_rate=di->rate;}pthread_mutex_unlock(&b.lock);
                NxtLog_step("audio","dsd-source",true,"container=%s; DSD_rate=%u; channels=%d; transport_setting=%d; card=%d",di->container,di->rate,di->channels,dsd_mode,card);
                if(!usb||card<0){fail(request,"DSD requires a compatible USB DAC; no analog/Bluetooth fallback");continue;}
                int active_mode=dsd_mode==1?1:0;
                int opened=NxtDSD_openALSA(&pcm,card,di,active_mode,&dsd_format,error);
                if(opened<0&&dsd_mode==2){
                    pthread_mutex_lock(&b.lock);bool current=request==b.request&&b.running&&b.usb&&b.card==card;pthread_mutex_unlock(&b.lock);
                    if(!current)continue;
                    NxtLog_step("audio","dsd-auto",false,"Native failed; trying explicitly approved DoP: %s",error);
                    active_mode=1;opened=NxtDSD_openALSA(&pcm,card,di,active_mode,&dsd_format,error);
                }
                if(opened<0){fail(request,error);continue;}
                play_rate=NxtDSD_frameRate(di,dsd_format);dsd_block=play_rate/50;if(dsd_block<BP_BLOCK)dsd_block=BP_BLOCK;if(dsd_block>DSD_BLOCK_MAX_FRAMES)dsd_block=DSD_BLOCK_MAX_FRAMES;
                base=NxtDSD_seek(dsd,seek,dsd_format);if(base<0){fail(request,"Cannot seek DSD");close_pcm(&pcm);continue;}
                count=NxtDSD_read(dsd,packed,dsd_block,dsd_format);if(count<=0){fail(request,"DSD truncated, empty or unaligned for the DAC format");close_pcm(&pcm);continue;}
                pthread_mutex_lock(&b.lock);if(request!=b.request||!b.running){pthread_mutex_unlock(&b.lock);close_pcm(&pcm);continue;}
                b.dsd=true;b.dsd_active_mode=active_mode;b.dsd_format=dsd_format;b.dsd_rate=di->rate;b.rate=active_mode?play_rate:di->rate;b.bits=active_mode?(dsd_format==NXT_DSD_DOP32_LE?32:24):1;b.channels=di->channels;b.ready=b.done=true;b.seeking=false;b.error[0]=0;b.position=base;b.peak=0;b.vis_count=0;pthread_cond_broadcast(&b.wake);pthread_mutex_unlock(&b.lock);
                NxtLog_step("audio","dsd-open",true,"hw:%d; DSD=%u; format=%d; frame_rate=%d; no PCM/DSP/resampler/gain",card,di->rate,dsd_format,play_rate);continue;
            }

            unsigned epoch=NxtStream_epoch();decoder=NxtDecoder_openEpoch(path,0,epoch);
            if(!decoder){fail(request,"This source does not provide supported lossless integer PCM; choose Normal mode");continue;}
            info=*NxtDecoder_info(decoder);play_rate=info.sample_rate;
            pthread_mutex_lock(&b.lock);if(request==b.request){b.info=info;b.generation++;b.position=seek;}pthread_mutex_unlock(&b.lock);
            if(!usb){fail(request,"Select USB DAC output for Bit-Perfect");continue;}
            if(card<0){fail(request,"Connect USB DAC; playback is paused without speaker fallback");continue;}
            if(seek>0&&!NxtDecoder_seekEpoch(decoder,seek,epoch)){fail(request,"This source cannot seek to the saved position");continue;}
            count=NxtDecoder_readNative(decoder,native,BP_BLOCK);
            if(count<=0){fail(request,"Cannot decode unchanged native PCM from this source");continue;}
            char error[256];if(open_pcm(&pcm,card,&info,&format,error)<0){fail(request,error);continue;}
            pthread_mutex_lock(&b.lock);
            if(request!=b.request||!b.running){pthread_mutex_unlock(&b.lock);close_pcm(&pcm);continue;}
            b.rate=info.sample_rate;b.bits=NxtNative_precision(format);b.channels=info.channels;b.ready=b.done=true;b.seeking=false;b.error[0]=0;pthread_cond_broadcast(&b.wake);
            pthread_mutex_unlock(&b.lock);NxtLog_step("audio","bit-perfect",true,"hw:%d; rate=%d; source_bits=%d; container=%d; channels=%d; no DSP/resampler",card,info.sample_rate,info.bit_depth,NxtNative_width(format)*8,info.channels);
            NxtNative_pack(native,(size_t)count*info.channels,format,packed);continue;
        }
        if(!pcm||(!decoder&&!dsd))continue;
        if(seeking){
            snd_pcm_drop(pcm);snd_pcm_prepare(pcm);
            int aligned=dsd?NxtDSD_seek(dsd,seek,dsd_format):seek;
            if(aligned<0||(!dsd&&!NxtDecoder_seekEpoch(decoder,seek,NxtStream_epoch()))){fail(request,"Cannot seek this native PCM source");close_pcm(&pcm);continue;}
            used=count=0;submitted=0;base=aligned;
            pthread_mutex_lock(&b.lock);if(request==b.request){b.seeking=false;b.position=seek;}pthread_mutex_unlock(&b.lock);continue;
        }
        pthread_mutex_lock(&b.lock);bool playing=b.state==PLAYER_STATE_PLAYING&&request==b.request;pthread_mutex_unlock(&b.lock);if(!playing)continue;
        if(used>=count){
            count=dsd?NxtDSD_read(dsd,packed,dsd_block,dsd_format):NxtDecoder_readNative(decoder,native,BP_BLOCK);used=0;
            pthread_mutex_lock(&b.lock);bool interrupted=request!=b.request||b.seeking||b.state!=PLAYER_STATE_PLAYING;pthread_mutex_unlock(&b.lock);if(interrupted)continue;
            if(count<0){fail(request,"PCM format changed or native decoding failed; playback stopped");close_pcm(&pcm);continue;}
            if(!count){
                position_update(request,pcm,base,submitted,play_rate);snd_pcm_sframes_t delay=0;
                if(snd_pcm_delay(pcm,&delay)>=0&&delay>0){usleep(5000);continue;}
                pthread_mutex_lock(&b.lock);if(request==b.request){b.position=info.duration_ms>0?info.duration_ms:b.position;b.state=PLAYER_STATE_STOPPED;b.ready=false;}pthread_mutex_unlock(&b.lock);close_pcm(&pcm);continue;
            }
            if(!dsd)NxtNative_pack(native,(size_t)count*info.channels,format,packed);
        }
        pthread_mutex_lock(&b.lock);bool write_ready=request==b.request&&!b.seeking&&b.state==PLAYER_STATE_PLAYING;pthread_mutex_unlock(&b.lock);if(!write_ready)continue;
        int frame_bytes=(dsd?NxtDSD_width(dsd_format):NxtNative_width(format))*info.channels;
        snd_pcm_sframes_t n=snd_pcm_writei(pcm,packed+used*frame_bytes,count-used);
        if(n==-EAGAIN||n==0){snd_pcm_wait(pcm,20);continue;}
        if(n<0){
            if(n==-EPIPE||n==-ESTRPIPE){if(snd_pcm_recover(pcm,(int)n,1)>=0){NxtLog_step("audio","bit-perfect-xrun",false,"PCM underrun/recovery; no rate or gain conversion");continue;}}
            char error[256];snprintf(error,sizeof(error),"USB PCM write failed (%s); playback paused",snd_strerror((int)n));fail(request,error);close_pcm(&pcm);continue;
        }
        pthread_mutex_lock(&b.lock);
        if(request==b.request&&!dsd){int vis=(int)n*2;if(vis>2048)vis=2048;
        int retained=b.vis_count;if(retained+vis>2048){int drop=retained+vis-2048;memmove(b.vis,b.vis+drop,(retained-drop)*sizeof(*b.vis));retained-=drop;}
        b.peak=0;for(int i=0;i<vis;i++){int channel=info.channels==1?0:i%2;int32_t value=native[used*info.channels+(i/2)*info.channels+channel];b.vis[retained+i]=(int16_t)(value/65536);float level=(float)(value<0?-(int64_t)value:value)/2147483648.0f;if(level>b.peak)b.peak=level;}
        b.vis_count=retained+vis;}
        if(request==b.request)b.listened+=((submitted+n)*1000/play_rate)-(submitted*1000/play_rate);
        pthread_mutex_unlock(&b.lock);
        used+=(int)n;submitted+=n;position_update(request,pcm,base,submitted,play_rate);
    }
    close_pcm(&pcm);NxtDecoder_close(decoder);NxtDSD_close(dsd);return NULL;
}
int NxtBit_init(void){
    if(b.started)return 0;b.running=true;
    if(pthread_create(&b.thread,NULL,worker,NULL)){b.running=false;return -1;}b.started=true;return 0;
}
void NxtBit_quit(void){
    pthread_mutex_lock(&b.lock);b.running=false;b.request++;pthread_cond_broadcast(&b.wake);pthread_mutex_unlock(&b.lock);NxtDecoder_cancelStreams();if(b.started)pthread_join(b.thread,NULL);b.started=false;
}
int NxtBit_load(const char *path,int position,PlayerState state){
    if(!path||!*path||strlen(path)>=sizeof(b.path))return -1;
    NxtDecoder_cancelStreams();
    pthread_mutex_lock(&b.lock);if(!b.running){pthread_mutex_unlock(&b.lock);return -1;}
    b.request++;memset(&b.info,0,sizeof(b.info));const char *name=strrchr(path,'/');snprintf(b.info.title,sizeof(b.info.title),"%s",name?name+1:path);b.generation++;snprintf(b.path,sizeof(b.path),"%s",path);b.seek_ms=b.position=position>0?position:0;b.seeking=false;b.done=b.ready=false;b.rate=b.bits=b.channels=0;b.vis_count=0;b.dsd=NxtDSD_isPath(path);b.dsd_mode=nxt.dsd_output;b.dsd_rate=0;b.peak=0;b.state=state;b.error[0]=0;pthread_cond_broadcast(&b.wake);pthread_mutex_unlock(&b.lock);return 0;
}
int NxtBit_loadStatus(void){pthread_mutex_lock(&b.lock);int r=!b.running||!*b.path?-1:!b.done?1:b.ready?0:-1;pthread_mutex_unlock(&b.lock);return r;}
int NxtBit_waitLoaded(void){pthread_mutex_lock(&b.lock);unsigned request=b.request;while(b.running&&request==b.request&&!b.done)pthread_cond_wait(&b.wake,&b.lock);int r=b.running&&request==b.request&&b.ready?0:-1;pthread_mutex_unlock(&b.lock);return r;}
void NxtBit_route(int card,bool usb){
    pthread_mutex_lock(&b.lock);
    if(b.card!=card||b.usb!=usb){if(*b.path)NxtDecoder_cancelStreams();b.card=card;b.usb=usb;if(*b.path){b.request++;b.seek_ms=b.position;b.done=b.ready=false;b.seeking=false;if(!usb||card<0)b.state=PLAYER_STATE_PAUSED;pthread_cond_broadcast(&b.wake);}}
    pthread_mutex_unlock(&b.lock);
}
void NxtBit_play(void){pthread_mutex_lock(&b.lock);if(b.ready){b.state=PLAYER_STATE_PLAYING;pthread_cond_broadcast(&b.wake);}pthread_mutex_unlock(&b.lock);}
void NxtBit_pause(void){pthread_mutex_lock(&b.lock);if(b.state==PLAYER_STATE_PLAYING){NxtDecoder_cancelStreams();b.state=PLAYER_STATE_PAUSED;b.seek_ms=b.position;b.seeking=true;pthread_cond_broadcast(&b.wake);}pthread_mutex_unlock(&b.lock);}
void NxtBit_stop(void){NxtDecoder_cancelStreams();pthread_mutex_lock(&b.lock);b.request++;b.path[0]=0;b.state=PLAYER_STATE_STOPPED;b.ready=b.done=b.seeking=false;b.vis_count=0;pthread_cond_broadcast(&b.wake);pthread_mutex_unlock(&b.lock);}
void NxtBit_seek(int ms){pthread_mutex_lock(&b.lock);if(b.ready){NxtDecoder_cancelStreams();b.seek_ms=ms<0?0:b.info.duration_ms>0&&ms>b.info.duration_ms?b.info.duration_ms:ms;b.seeking=true;pthread_cond_broadcast(&b.wake);}pthread_mutex_unlock(&b.lock);}
bool NxtBit_seeking(void){pthread_mutex_lock(&b.lock);bool v=b.seeking;pthread_mutex_unlock(&b.lock);return v;}
PlayerState NxtBit_state(void){pthread_mutex_lock(&b.lock);PlayerState v=b.state;pthread_mutex_unlock(&b.lock);return v;}
int NxtBit_position(void){pthread_mutex_lock(&b.lock);int v=b.position;pthread_mutex_unlock(&b.lock);return v;}
int64_t NxtBit_listened(void){pthread_mutex_lock(&b.lock);int64_t v=b.listened;pthread_mutex_unlock(&b.lock);return v;}
bool NxtBit_ready(void){pthread_mutex_lock(&b.lock);bool v=b.ready;pthread_mutex_unlock(&b.lock);return v;}
int NxtBit_rate(void){pthread_mutex_lock(&b.lock);int v=b.ready?b.rate:0;pthread_mutex_unlock(&b.lock);return v;}
int NxtBit_bits(void){pthread_mutex_lock(&b.lock);int v=b.ready?b.bits:0;pthread_mutex_unlock(&b.lock);return v;}
int NxtBit_channels(void){pthread_mutex_lock(&b.lock);int v=b.channels;pthread_mutex_unlock(&b.lock);return v;}
int NxtBit_vis(int16_t *out,int max){pthread_mutex_lock(&b.lock);int n=b.state==PLAYER_STATE_PLAYING?(b.vis_count<max?b.vis_count:max):0;memcpy(out,b.vis,n*sizeof(*out));pthread_mutex_unlock(&b.lock);return n;}
unsigned NxtBit_snapshot(TrackInfo *info,char *path,size_t size){pthread_mutex_lock(&b.lock);*info=b.info;snprintf(path,size,"%s",b.path);unsigned gen=b.generation;pthread_mutex_unlock(&b.lock);return gen|0x40000000u;}
const char *NxtBit_error(void){static __thread char message[256];pthread_mutex_lock(&b.lock);snprintf(message,sizeof(message),"%s",b.error);pthread_mutex_unlock(&b.lock);return message;}
bool NxtBit_isDSD(void){pthread_mutex_lock(&b.lock);bool v=b.dsd;pthread_mutex_unlock(&b.lock);return v;}
void NxtBit_setDSDMode(int mode){if(mode<0||mode>2)return;pthread_mutex_lock(&b.lock);b.dsd_mode=mode;if(b.dsd&&*b.path){b.request++;b.seek_ms=b.position;b.done=b.ready=false;b.seeking=false;b.vis_count=0;pthread_cond_broadcast(&b.wake);}pthread_mutex_unlock(&b.lock);}
const char *NxtBit_status(void){
 static __thread char text[256];pthread_mutex_lock(&b.lock);bool raw=b.dsd,ready=b.ready;int mode=b.dsd_active_mode,rate=b.dsd_rate,carrier=b.rate,fmt=b.dsd_format;PlayerState state=b.state;pthread_mutex_unlock(&b.lock);
 if(raw){if(!ready)return Nxt_text("DSD · USB output not ready · see output information","DSD · chưa mở được đầu ra USB · xem thông tin đầu ra");const char*types[]={"DSD_U8","DSD_U16_LE","DSD_U32_LE","DSD_U16_BE","DSD_U32_BE","S24_3LE","S24_LE","S32_LE"};snprintf(text,sizeof(text),"DSD%u · %s · %.4f MHz · %s%s",rate/(rate%44100?48000:44100),mode?"DoP":"Native DSD",rate/1000000.f,types[fmt>=0&&fmt<8?fmt:0],state==PLAYER_STATE_PAUSED?Nxt_text(" · paused"," · tạm dừng"):"");if(mode){size_t n=strlen(text);snprintf(text+n,sizeof(text)-n," · carrier %d Hz",carrier);}return text;}
 return !ready?Nxt_text("Bit-Perfect requested · paused / unavailable","Yêu cầu Bit-Perfect · tạm dừng / chưa sẵn sàng"):state==PLAYER_STATE_PLAYING?Nxt_text("Bit-Perfect · native USB PCM","Bit-Perfect · PCM USB nguyên bản"):Nxt_text("Bit-Perfect · ready","Bit-Perfect · sẵn sàng");
}

float NxtBit_peak(void){pthread_mutex_lock(&b.lock);float v=b.peak;pthread_mutex_unlock(&b.lock);return v;}
