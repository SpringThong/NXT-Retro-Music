#include "nxt_log.h"
#include "nxt_online_ramp.h"
#include "nxt_spotify.h"
#include "nxt_soundcloud.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "player.h"
#include "nxt.h"
#include "nxt_dsd.h"
#include "nxt_decoder.h"
#include "nxt_bitperfect.h"
#include "nxt_audio_mode.h"
#include "nxt_skin.h"
#include "nxt_volume.h"
#include "podcast.h"
#include "nxt_stream.h"
#include "nxt_drive.h"
#include "radio.h"
#include "audio/sonic.h"
#include "api.h"
#include "msettings.h"
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <dirent.h>
#include <samplerate.h>
#include <alsa/asoundlib.h>
#define BLOCK 1024
#define BLOCKS 64
typedef struct { float pcm[BLOCK*2]; int count,used,position; unsigned generation; TrackInfo info; char path[512]; float gain,speed; bool mixed; } PCMBlock;
typedef struct {
    pthread_mutex_t lock; pthread_cond_t wake; pthread_t worker;
    PCMBlock ring[BLOCKS]; int read,write,queued;
    bool running,eof,loaded,load_done,load_ok,seeking,worker_started,preloaded,seekable,network,buffering;
    unsigned request,worker_request,gen,visible_gen;
    char request_path[512],next_path[512],visible_path[512];
    int seek_ms,position,rate,bits,sink; PlayerState state;
    TrackInfo visible_info;
    SDL_AudioDeviceID device; SDL_AudioFormat format;
    float volume,speed; NxtConfig options; int radio_rate,radio_epoch; SRC_STATE *radio_src;
    int16_t vis[2048]; int vis_count; double listened_ms;
} Engine;
static Engine e={.lock=PTHREAD_MUTEX_INITIALIZER,.wake=PTHREAD_COND_INITIALIZER};
static NxtOnlineRamp online_ramp;
static TrackInfo ui_info; static char ui_path[512]; static unsigned ui_gen,sink_generation;
static SDL_Surface *art; static char *embedded;
static bool bluetooth_audio_active,usbdac_audio_active;
static volatile bool sink_changed;
static Uint32 sink_retry_at,sink_retry_started;
static char output_error[256];
static int usb_hid_fd=-1; static bool hid_missing_logged; static uint32_t hid_retry_at;
#define HID_RETRY_MS 1500
#define EV_KEY 1
#define KEY_VOLUMEDOWN 114
#define KEY_VOLUMEUP 115
#define KEY_NEXTSONG 163
#define KEY_PLAYPAUSE 164
#define KEY_PREVIOUSSONG 165
#define KEY_PLAYCD 200
#define KEY_PAUSECD 201
struct input_event_raw { uint64_t tv_sec,tv_usec; uint16_t type,code; int32_t value; };

static float replay(const TrackInfo *info,const NxtConfig *c) { float v=c->replaygain==1?info->replay_track:c->replaygain==2?info->replay_album:0; return fmaxf(-24,fminf(12,v)); }
static void *decode_worker(void *unused) {
    (void)unused;
    NxtDecoder *current=NULL,*next=NULL; sonicStream stretch=NULL;
    char path[512]="",prepared[512]=""; TrackInfo info={0}; unsigned local_request=0,generation=0;
    float pcm[BLOCK*2],incoming[BLOCK*2],stretched[BLOCK*2]; int frame_position=0;double stretch_position=0;bool stretch_flushed=false;int last_transition=-1;
    for(;;) {
        pthread_mutex_lock(&e.lock);
        if(!e.running) { pthread_mutex_unlock(&e.lock); break; }
        unsigned request=e.request,network_epoch=NxtStream_epoch(); char requested[512],candidate[512];
        snprintf(requested,sizeof(requested),"%s",e.request_path); snprintf(candidate,sizeof(candidate),"%s",e.next_path);
        int seek=e.seek_ms,rate=e.rate>0?e.rate:48000; float speed=e.speed; NxtConfig options=e.options;
        bool room=e.queued<BLOCKS,seek_now=e.seeking;
        pthread_mutex_unlock(&e.lock);
        if(request!=local_request) {
            NxtDecoder_close(current); NxtDecoder_close(next); current=next=NULL; prepared[0]=0;
            if(stretch) sonicDestroyStream(stretch); stretch=sonicCreateStream(rate,2);
            snprintf(path,sizeof(path),"%s",requested);if(!*path)NxtStream_cleanup(); current=*path?NxtDecoder_openEpoch(path,rate,network_epoch):NULL;
            local_request=request; generation++;stretch_position=seek;stretch_flushed=false;
            pthread_mutex_lock(&e.lock);
            if(request==e.request) {
                e.read=e.write=e.queued=0; e.network=NxtDrive_isStream(path);e.buffering=e.network; e.eof=false; e.loaded=current!=NULL; e.seekable=NxtDecoder_canSeek(current); e.load_done=true; e.load_ok=current!=NULL; e.seeking=false;
                if(current) { info=*NxtDecoder_info(current); e.visible_info=info; snprintf(e.visible_path,sizeof(e.visible_path),"%s",path); e.visible_gen=generation; e.position=seek;if(e.state==PLAYER_STATE_STOPPED)e.state=PLAYER_STATE_PAUSED; }
                pthread_cond_broadcast(&e.wake);
            }
            pthread_mutex_unlock(&e.lock);
            if(current&&seek>0) NxtDecoder_seekEpoch(current,seek,network_epoch);
            continue;
        }
        if(seek_now&&current) {
            stretch_position=seek;stretch_flushed=false;NxtDecoder_seekEpoch(current,seek,network_epoch); NxtDecoder_close(next); next=NULL; prepared[0]=0;
            if(stretch) sonicDestroyStream(stretch); stretch=sonicCreateStream(rate,2);
            pthread_mutex_lock(&e.lock); if(request==e.request&&network_epoch==NxtStream_epoch()) { e.read=e.write=e.queued=0; e.buffering=e.network; e.eof=false; e.seeking=false; e.position=NxtDecoder_position(current); } pthread_mutex_unlock(&e.lock);
            continue;
        }
        if(!current||!room) { pthread_mutex_lock(&e.lock);if(e.running&&request==e.request&&!e.seeking&&(!current||e.queued>=BLOCKS))pthread_cond_wait(&e.wake,&e.lock);pthread_mutex_unlock(&e.lock);continue; }
        if(last_transition!=options.transition){NxtDecoder_close(next);next=NULL;prepared[0]=0;last_transition=options.transition;}
        if(*candidate&&NxtDrive_isStream(candidate)){NxtDriveEntry entry;if(NxtDrive_streamEntry(candidate,&entry)){NxtStream_prefetch(&entry);if(current&&!next&&strcmp(candidate,path)&&options.transition>0&&NxtStream_prefetched(&entry)){next=NxtDecoder_openEpoch(candidate,rate,network_epoch);snprintf(prepared,sizeof(prepared),"%s",candidate);}}}
        if(strcmp(candidate,prepared)!=0) {
            NxtDecoder_close(next); next=NULL; snprintf(prepared,sizeof(prepared),"%s",candidate);
            if(*candidate&&options.transition>0&&!NxtDrive_isStream(candidate)) next=NxtDecoder_openEpoch(candidate,rate,network_epoch);
        }
        bool use_stretch=stretch&&fabsf(speed-1)>.001f,mixed=false;
        frame_position=use_stretch?(int)stretch_position:NxtDecoder_position(current);
        int n=use_stretch?sonicReadFloatFromStream(stretch,pcm,BLOCK):0;
        float gain=replay(&info,&options);
        if(!n) {
            int raw=NxtDecoder_read(current,pcm,BLOCK);n=raw;
            if(raw>0&&next&&options.transition==2&&info.duration_ms>0&&frame_position>=info.duration_ms-options.crossfade_ms&&!use_stretch) {
                int got=NxtDecoder_read(next,incoming,raw);float nextgain=powf(10,replay(NxtDecoder_info(next),&options)/20),oldgain=powf(10,gain/20);
                for(int i=0;i<raw;i++){float t=(frame_position+i*1000.f/rate-(info.duration_ms-options.crossfade_ms))/(float)options.crossfade_ms;t=fmaxf(0,fminf(1,t));float a=cosf(t*M_PI*.5f)*oldgain,b=sinf(t*M_PI*.5f)*nextgain;for(int ch=0;ch<2;ch++)pcm[i*2+ch]=pcm[i*2+ch]*a+(i<got?incoming[i*2+ch]*b:0);}
                mixed=true;gain=0;
            }
            if(use_stretch){sonicSetSpeed(stretch,speed);if(raw>0)sonicWriteFloatToStream(stretch,pcm,raw);else if(!stretch_flushed){sonicFlushStream(stretch);stretch_flushed=true;}n=sonicReadFloatFromStream(stretch,stretched,BLOCK);if(n>0)memcpy(pcm,stretched,n*2*sizeof(float));}
            if(!raw&&!n) {
                pthread_mutex_lock(&e.lock);
                bool interrupted=request!=e.request||e.seeking||network_epoch!=NxtStream_epoch();
                pthread_mutex_unlock(&e.lock);
                if(interrupted){usleep(3000);continue;}
                if(next){NxtDecoder_close(current);current=next;next=NULL;snprintf(path,sizeof(path),"%s",prepared);prepared[0]=0;info=*NxtDecoder_info(current);generation++;stretch_position=NxtDecoder_position(current);stretch_flushed=false;if(stretch)sonicDestroyStream(stretch);stretch=sonicCreateStream(rate,2);pthread_mutex_lock(&e.lock);if(request==e.request){e.next_path[0]=0;e.eof=false;e.seekable=NxtDecoder_canSeek(current);}pthread_mutex_unlock(&e.lock);continue;}
                pthread_mutex_lock(&e.lock);if(request==e.request&&!e.seeking)e.eof=true;pthread_cond_broadcast(&e.wake);if(e.running&&request==e.request&&!e.seeking)pthread_cond_wait(&e.wake,&e.lock);pthread_mutex_unlock(&e.lock);continue;
            }
        }
        if(use_stretch&&n>0)stretch_position+=n*1000.0/rate*speed;
        if(n<=0)continue;
        pthread_mutex_lock(&e.lock);
        if(request==e.request&&!e.seeking&&e.queued<BLOCKS) {
            PCMBlock *b=&e.ring[e.write]; memcpy(b->pcm,pcm,n*2*sizeof(float)); b->count=n; b->used=0; b->position=frame_position; b->generation=generation; b->info=info; snprintf(b->path,sizeof(b->path),"%s",path); b->gain=gain;b->speed=speed;b->mixed=mixed;
            e.write=(e.write+1)%BLOCKS; e.queued++; pthread_cond_broadcast(&e.wake);
        }
        pthread_mutex_unlock(&e.lock);
    }
    NxtDecoder_close(current); NxtDecoder_close(next); if(stretch)sonicDestroyStream(stretch); return NULL;
}
static int radio_samples(float *out,int wanted) {
    static float input[BLOCK*2];static int count,used,last_rate;static bool was_active;static int last_epoch;
    pthread_mutex_lock(&e.lock);int rate=e.radio_rate,epoch=e.radio_epoch;pthread_mutex_unlock(&e.lock);
    if(!was_active||last_rate!=rate||last_epoch!=epoch){count=used=0;if(e.radio_src)src_reset(e.radio_src);last_rate=rate;last_epoch=epoch;}was_active=true;
    if(!e.radio_src){int16_t raw[BLOCK*2];int n=Radio_getAudioSamples(raw,wanted*2);for(int i=0;i<n;i++)out[i]=raw[i]/32768.f;return n/2;}
    int done=0;
    for(int iteration=0;iteration<8&&done<wanted;iteration++){
        if(used>=count){int16_t raw[BLOCK*2];count=Radio_getAudioSamples(raw,BLOCK*2)/2;used=0;for(int i=0;i<count*2;i++)input[i]=raw[i]/32768.f;if(!count)break;}
        SRC_DATA data={.data_in=input+used*2,.data_out=out+done*2,.input_frames=count-used,.output_frames=wanted-done,.src_ratio=(double)e.rate/rate};
        if(src_process(e.radio_src,&data))break;used+=data.input_frames_used;done+=data.output_frames_gen;if(!data.input_frames_used&&!data.output_frames_gen)break;
    }return done;
}
static void output_audio(void *userdata,Uint8 *stream,int bytes) {
    (void)userdata;
    int sample_bytes=e.format==AUDIO_F32SYS?4:2; int frames=bytes/(sample_bytes*2),done=0;
    memset(stream,0,bytes);
    while(done<frames) {
        float pcm[BLOCK*2]={0}; int n=frames-done; if(n>BLOCK)n=BLOCK; float gain=0; int got=0;
        if(Radio_isActive()) {
            got=radio_samples(pcm,n);
        } else if((got=NxtSoundCloud_audioSamples(pcm,n,e.rate))>0) {
            /* SoundCloud uses the same DSP, visualization and output below. */
        } else if((got=NxtSpotify_audioSamples(pcm,n,e.rate))>0) {
            /* Spotify uses the same DSP, visualization and output volume below. */
        } else {
            pthread_mutex_lock(&e.lock);
            if(e.network&&e.buffering&&(e.queued>=16||e.eof))e.buffering=false;
            if(e.network&&!e.queued&&!e.eof)e.buffering=true;
            if(e.state==PLAYER_STATE_PLAYING&&!e.seeking&&e.queued&&!e.buffering) {
                PCMBlock *b=&e.ring[e.read]; if(n>b->count-b->used)n=b->count-b->used;
                memcpy(pcm,b->pcm+b->used*2,n*2*sizeof(float)); gain=b->mixed?0:replay(&b->info,&e.options); b->used+=n; got=n;
                e.visible_gen=b->generation; e.visible_info=b->info; snprintf(e.visible_path,sizeof(e.visible_path),"%s",b->path);
                e.position=b->position+(int)(b->used*1000.f/e.rate*b->speed);
                if(b->used==b->count) { e.read=(e.read+1)%BLOCKS; e.queued--;pthread_cond_signal(&e.wake); }
            } else if(e.state==PLAYER_STATE_PLAYING&&!e.seeking&&e.eof&&!e.queued) e.state=PLAYER_STATE_STOPPED;
            pthread_mutex_unlock(&e.lock);
        }
        NxtDSP_process(pcm,n,gain);
        pthread_mutex_lock(&e.lock); float volume=e.volume;
        if(__atomic_load_n(&e.sink,__ATOMIC_RELAXED)==AUDIO_SINK_USBDAC)volume*=NxtVolume_gain();
#ifdef NXT_ROM_STOCK
        if(__atomic_load_n(&e.sink,__ATOMIC_RELAXED)==AUDIO_SINK_USBDAC&&!NxtVolume_hardware())volume*=GetUSBDACSoftwareVolume()/100.f;
#endif
#ifdef NXT_ROM_NXREDUX
        /* Firmware music gain and OSD mute are read once per block, with no
         * file access, allocation, subprocess or shared-memory layout copy. */
        volume*=GetSpeakerMute()?0.f:GetMusicVolume()/20.f;
#endif
        for(int i=0;i<n*2;i++) pcm[i]*=volume;
        if(e.network||NxtSpotify_active()||NxtSoundCloud_active())NxtOnlineRamp_process(&online_ramp,pcm,n,got,e.rate);
        else memset(&online_ramp,0,sizeof(online_ramp));
        if(got) { if(e.rate>0)e.listened_ms+=got*1000.0/e.rate;int s=got*2; if(s>2048)s=2048; for(int i=0;i<s;i++)e.vis[i]=lrintf(fmaxf(-1,fminf(.999969f,pcm[i]))*32768); e.vis_count=s; }
        pthread_mutex_unlock(&e.lock);
        if(e.format==AUDIO_F32SYS) memcpy((float*)stream+done*2,pcm,n*2*sizeof(float));
        else for(int i=0;i<n*2;i++)((int16_t*)stream)[done*2+i]=lrintf(fmaxf(-1,fminf(.999969f,pcm[i]))*32768);
        done+=n;
    }
}
static int open_audio(void) {
    if(nxt.bit_perfect){NxtBit_route(NxtHardware_dacCard(),nxt.output==2);e.rate=e.bits=0;return 0;}
    /* Both ROMs now have a private, named PCM. Never reuse an inherited
     * speaker device or ALSA tree after the user changes output. */
    if(SDL_setenv("AUDIODEV","nxt_music_output",1)){snprintf(output_error,sizeof(output_error),"Cannot select app PCM");return -1;}
#ifdef NXT_ROM_NXREDUX
    /* The firmware SDL has an ALSA hotplug thread. Stop it while replacing
     * the process-local config; restart audio after the cache is reset. */
    if(SDL_WasInit(SDL_INIT_AUDIO))SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif
    /* Called only with the previous SDL device closed; process-local ALSA tree. */
    if(snd_config_update_free_global()<0){snprintf(output_error,sizeof(output_error),"Cannot reload ALSA route");LOG_info("Audio: ALSA config reset failed\n");return -1;}
#ifdef NXT_ROM_NXREDUX
    if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0){snprintf(output_error,sizeof(output_error),"%s",SDL_GetError());return -1;}
#endif
    SDL_AudioSpec want={0},have; want.freq=48000; want.channels=2; want.samples=1024; want.format=AUDIO_F32SYS; want.callback=output_audio;
    e.device=SDL_OpenAudioDevice(NULL,0,&want,&have,0);
    if(!e.device) { want.format=AUDIO_S16SYS; e.device=SDL_OpenAudioDevice(NULL,0,&want,&have,0); }
    if(!e.device) { snprintf(output_error,sizeof(output_error),"%s",SDL_GetError());fprintf(stderr,"NXT audio: %s\n",output_error);NxtLog_step("audio","pcm-open",false,"Cannot open requested PCM; retry controller will handle it");return -1; }
    output_error[0]=0;
    e.rate=have.freq; e.format=have.format; e.bits=have.format==AUDIO_F32SYS?32:16;
    NxtLog_step("audio","pcm-open",true,"rate=%d; bits=%d",e.rate,e.bits);NxtDSP_configure(&nxt,e.rate); SDL_PauseAudioDevice(e.device,0); return 0;
}
static void sink_event(int type,int event) { (void)type;(void)event; sink_changed=true; }
int Player_init(void) {
    if(NxtBit_init()<0)return -1;
    e.volume=e.speed=1; e.running=true; e.options=nxt; e.state=PLAYER_STATE_STOPPED;
    #ifdef NXT_ROM_STOCK
    __atomic_store_n(&e.sink,GetAudioSink(),__ATOMIC_RELAXED);
#endif
    e.radio_rate=44100;int src_error=0;e.radio_src=src_new(SRC_SINC_FASTEST,2,&src_error);
    if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0||open_audio()<0){e.running=false;return -1;}
    #ifndef NXT_ROM_STOCK
    e.sink=GetAudioSink();
#endif
    usbdac_audio_active=e.sink==2; bluetooth_audio_active=e.sink==1;
    PLAT_audioDeviceWatchRegister(sink_event);if(pthread_create(&e.worker,NULL,decode_worker,NULL)!=0){e.running=false;SDL_CloseAudioDevice(e.device);e.device=0;return -1;}e.worker_started=true;Player_initUSBHID();return 0;
}
void Player_quit(void) {
    PLAT_audioDeviceWatchUnregister(); Player_quitUSBHID();
    NxtBit_quit();
    if(e.device) { SDL_CloseAudioDevice(e.device); e.device=0; }
    pthread_mutex_lock(&e.lock); NxtDecoder_cancelStreams(); e.running=false; pthread_cond_broadcast(&e.wake); pthread_mutex_unlock(&e.lock); if(e.worker_started)pthread_join(e.worker,NULL);NxtStream_cleanup();e.worker_started=false;if(e.radio_src)e.radio_src=src_delete(e.radio_src);
    if(art){NxtSkin_forgetArt();SDL_FreeSurface(art);} art=NULL; free(embedded);embedded=NULL; SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
/* Network opens run on the existing decoder worker while the UI keeps polling. */
static int begin_load_at(const char *path,int position,PlayerState state) {
    NxtSpotify_quit(); NxtSoundCloud_quit();
    if(!path||!*path||strlen(path)>=sizeof(e.request_path))return -1;
    if(nxt.bit_perfect){NxtBit_route(NxtHardware_dacCard(),nxt.output==2);return NxtBit_load(path,position,state==PLAYER_STATE_STOPPED?PLAYER_STATE_PAUSED:state);}
    pthread_mutex_lock(&e.lock);
    if(!e.running){pthread_mutex_unlock(&e.lock);return -1;}
    NxtDecoder_cancelStreams();e.state=state;e.request++;
    snprintf(e.request_path,sizeof(e.request_path),"%s",path);e.next_path[0]=0;
    e.load_done=e.load_ok=e.loaded=e.preloaded=e.eof=e.seeking=false;
    e.read=e.write=e.queued=0;e.seek_ms=e.position=position>0?position:0;
    pthread_cond_broadcast(&e.wake);pthread_mutex_unlock(&e.lock);return 0;
}
int Player_beginLoad(const char *path){NxtAudio_prepare(path);return begin_load_at(path,0,PLAYER_STATE_STOPPED);}
int Player_loadStatus(void) {
    NxtAudio_tick();
    if(nxt.bit_perfect){int r=NxtBit_loadStatus();if(!r)Player_update();return r;}
    pthread_mutex_lock(&e.lock);int result=1;
    if(!e.running||!*e.request_path||(e.load_done&&(!e.load_ok||(e.eof&&!e.queued))))result=-1;
    else if(e.load_done&&(e.queued>=4||e.eof)){e.preloaded=true;result=0;}
    pthread_mutex_unlock(&e.lock);
    if(!result)Player_update();return result;
}
int Player_load(const char *path) {
    NxtAudio_prepare(path);
    if(nxt.bit_perfect){if(begin_load_at(path,0,PLAYER_STATE_PAUSED)<0)return -1;int r=NxtBit_waitLoaded();NxtAudio_tick();if(nxt.bit_perfect){Player_update();return r;}}
    pthread_mutex_lock(&e.lock);
    bool ready=path&&e.preloaded&&e.loaded&&!strcmp(path,e.request_path);
    if(ready)e.preloaded=false;
    pthread_mutex_unlock(&e.lock);
    if(ready){Player_update();return 0;}
    if(Player_beginLoad(path)<0)return -1;
    pthread_mutex_lock(&e.lock);unsigned request=e.request;
    while(e.running&&request==e.request&&!e.load_done)pthread_cond_wait(&e.wake,&e.lock);
    while(e.running&&request==e.request&&e.load_ok&&!e.eof&&e.queued<4)pthread_cond_wait(&e.wake,&e.lock);
    int result=e.running&&request==e.request&&e.load_ok&&e.queued?0:-1;
    pthread_mutex_unlock(&e.lock);Player_update();return result;
}
int Player_play(void) { if(nxt.bit_perfect){if(!NxtBit_ready())return -1;NxtBit_play();return 0;}pthread_mutex_lock(&e.lock); bool loaded=e.loaded; if(loaded)e.state=PLAYER_STATE_PLAYING; pthread_mutex_unlock(&e.lock); return loaded?0:-1; }
void Player_pause(void) { if(nxt.bit_perfect){NxtBit_pause();return;}pthread_mutex_lock(&e.lock); if(e.state==PLAYER_STATE_PLAYING)e.state=PLAYER_STATE_PAUSED; pthread_mutex_unlock(&e.lock); }
void Player_stop(void) { NxtBit_stop();pthread_mutex_lock(&e.lock); NxtDecoder_cancelStreams(); e.state=PLAYER_STATE_STOPPED; e.request++; e.request_path[0]=e.next_path[0]=0; e.loaded=e.preloaded=e.eof=e.seeking=false; e.read=e.write=e.queued=0; pthread_cond_broadcast(&e.wake); pthread_mutex_unlock(&e.lock); }
void Player_togglePause(void) { if(Player_getState()==PLAYER_STATE_PLAYING)Player_pause();else Player_play(); }
void Player_seek(int ms) { if(nxt.bit_perfect){NxtBit_seek(ms);return;}pthread_mutex_lock(&e.lock); if(e.loaded&&e.seekable) { e.seek_ms=ms<0?0:ms>e.visible_info.duration_ms?e.visible_info.duration_ms:ms; NxtDecoder_cancelStreams(); e.seeking=true; e.buffering=e.network; e.eof=false; e.queued=0; e.read=e.write=0;pthread_cond_broadcast(&e.wake); } pthread_mutex_unlock(&e.lock); }
bool Player_resume(void) { if(nxt.bit_perfect)return NxtBit_seeking();pthread_mutex_lock(&e.lock); bool result=e.seeking; pthread_mutex_unlock(&e.lock); return result; }
static bool mode_reload=true;static int mode_state=-1;
void Player_refreshOptions(void){
    pthread_mutex_lock(&e.lock);bool changed=e.options.bit_perfect!=nxt.bit_perfect,old_bit=e.options.bit_perfect,running=e.running;int position=e.position;PlayerState state=e.state;char path[512];snprintf(path,sizeof(path),"%s",e.visible_path);pthread_mutex_unlock(&e.lock);
    if(changed&&running){
        if(old_bit){TrackInfo info;NxtBit_snapshot(&info,path,sizeof(path));position=NxtBit_position();state=NxtBit_state();}
        if(mode_state>=0)state=(PlayerState)mode_state;
        if(e.device){SDL_CloseAudioDevice(e.device);e.device=0;}
        if(mode_reload){NxtSpotify_quit();NxtSoundCloud_quit();if(Radio_isActive())Radio_stop();if(Podcast_isActive()){Podcast_stop();Podcast_flushProgress();}}
        Player_stop();
    }
    pthread_mutex_lock(&e.lock);e.options=nxt;if(changed&&!nxt.bit_perfect){e.rate=48000;e.bits=32;e.format=AUDIO_F32SYS;e.position=position;}pthread_mutex_unlock(&e.lock);
    if(changed&&running){
        ui_gen=~0u;
        if(mode_reload&&*path&&state!=PLAYER_STATE_STOPPED){
            if(nxt.bit_perfect){NxtBit_route(NxtHardware_dacCard(),nxt.output==2);NxtBit_load(path,position,state);}
            else begin_load_at(path,position,state);
        }
        Player_requestOutputRefresh();
    }
}
void Player_switchBitMode(bool bit,bool reload,int state){mode_reload=reload;mode_state=state;nxt.bit_perfect=bit;Player_refreshOptions();mode_reload=true;mode_state=-1;if(!bit)NxtDSP_configure(&nxt,48000);}
void Player_prepareNext(const char *path) { if(nxt.bit_perfect)return;pthread_mutex_lock(&e.lock); snprintf(e.next_path,sizeof(e.next_path),"%s",path?path:""); e.options=nxt; pthread_mutex_unlock(&e.lock); }
void Player_setVolume(float v) { if(nxt.bit_perfect)return;pthread_mutex_lock(&e.lock); e.volume=fmaxf(0,fminf(1,v)); pthread_mutex_unlock(&e.lock); }
float Player_getVolume(void) { pthread_mutex_lock(&e.lock); float v=e.volume; pthread_mutex_unlock(&e.lock); return v; }
void Player_setPlaybackSpeed(float v) { if(nxt.bit_perfect)return;pthread_mutex_lock(&e.lock);v=fmaxf(.5f,fminf(2,v));if(fabsf(v-e.speed)>.001f&&e.loaded&&e.seekable){e.seek_ms=e.position;NxtDecoder_cancelStreams();e.eof=false;e.seeking=true;e.queued=0;e.read=e.write=0;pthread_cond_broadcast(&e.wake);}e.speed=v;pthread_mutex_unlock(&e.lock); }
float Player_getPlaybackSpeed(void) { if(nxt.bit_perfect)return 1.0f;pthread_mutex_lock(&e.lock); float v=e.speed; pthread_mutex_unlock(&e.lock); return v; }
PlayerState Player_getState(void) { if(nxt.bit_perfect)return NxtBit_state();pthread_mutex_lock(&e.lock); PlayerState s=e.state; pthread_mutex_unlock(&e.lock); return s; }
int64_t Player_getListeningTime(void){pthread_mutex_lock(&e.lock);int64_t ms=e.listened_ms;pthread_mutex_unlock(&e.lock);return ms+NxtBit_listened();}
int Player_getPosition(void) { if(nxt.bit_perfect)return NxtBit_position();pthread_mutex_lock(&e.lock); int p=e.position; pthread_mutex_unlock(&e.lock); return p; }
int Player_getDuration(void) { return ui_info.duration_ms; }
const TrackInfo *Player_getTrackInfo(void) { return &ui_info; }
const char *Player_getCurrentFile(void) { return ui_path; }
SDL_Surface *Player_getAlbumArt(void) { return art; }
const char *Player_getEmbeddedLyrics(void) { return embedded; }
const WaveformData *Player_getWaveform(void) { static WaveformData w; return &w; }
int Player_getVisBuffer(int16_t *buf,int max) { if(nxt.bit_perfect)return NxtBit_vis(buf,max);pthread_mutex_lock(&e.lock); int n=e.vis_count<max?e.vis_count:max; memcpy(buf,e.vis,n*sizeof(int16_t)); pthread_mutex_unlock(&e.lock); return n; }
int Player_getOutputRate(void) { if(nxt.bit_perfect)return NxtBit_rate();return e.rate>0?e.rate:48000; }
int Player_getOutputBits(void) { if(nxt.bit_perfect)return NxtBit_bits();return e.bits; }
unsigned Player_getTrackGeneration(void) { return ui_gen; }
unsigned Player_getAudioSinkGeneration(void) { return sink_generation; }
void Player_update(void) {
    if(nxt.bit_perfect){TrackInfo info;char path[512];unsigned gen=NxtBit_snapshot(&info,path,sizeof(path));if(gen!=ui_gen){ui_info=info;snprintf(ui_path,sizeof(ui_path),"%s",path);ui_gen=gen;if(art){NxtSkin_forgetArt();SDL_FreeSurface(art);}art=NxtDecoder_art(ui_path);free(embedded);embedded=NxtDecoder_lyrics(ui_path);}return;}
    pthread_mutex_lock(&e.lock); unsigned gen=e.visible_gen;
    if(gen!=ui_gen) { ui_info=e.visible_info; snprintf(ui_path,sizeof(ui_path),"%s",e.visible_path); } pthread_mutex_unlock(&e.lock);
    if(gen!=ui_gen) { ui_gen=gen; if(art){NxtSkin_forgetArt();SDL_FreeSurface(art);} art=NxtDecoder_art(ui_path); free(embedded);embedded=NxtDecoder_lyrics(ui_path); }
}
AudioFormat Player_detectFormat(const char *path) {
if(NxtDSD_isPath(path))return AUDIO_FORMAT_DSD;
 const char *x=path?strrchr(path,'.'):NULL; if(!x)return AUDIO_FORMAT_UNKNOWN; x++; if(!strcasecmp(x,"wav"))return AUDIO_FORMAT_WAV; if(!strcasecmp(x,"mp3"))return AUDIO_FORMAT_MP3; if(!strcasecmp(x,"flac"))return AUDIO_FORMAT_FLAC; if(!strcasecmp(x,"ogg"))return AUDIO_FORMAT_OGG; if(!strcasecmp(x,"opus"))return AUDIO_FORMAT_OPUS; if(!strcasecmp(x,"m4a")||!strcasecmp(x,"mp4")||!strcasecmp(x,"alac"))return AUDIO_FORMAT_M4A; if(!strcasecmp(x,"aac"))return AUDIO_FORMAT_AAC;return AUDIO_FORMAT_UNKNOWN; }
void Player_requestOutputRefresh(void) { sink_retry_at=sink_retry_started=0;sink_changed=true; }
void Player_handleAudioSinkChange(void) {
    if(nxt.bit_perfect){if(e.device){SDL_CloseAudioDevice(e.device);e.device=0;}NxtBit_route(NxtHardware_dacCard(),nxt.output==2);if(sink_changed){sink_changed=false;sink_generation++;}usbdac_audio_active=nxt.output==2;bluetooth_audio_active=false;return;}
    if(!sink_changed||(sink_retry_at&&(int32_t)(SDL_GetTicks()-sink_retry_at)<0))return;
    sink_changed=false;sink_retry_at=0;sink_generation++;
    int pos=Player_getPosition(); PlayerState state=Player_getState(); char path[512];snprintf(path,sizeof(path),"%s",ui_path);
    bool was_usb=usbdac_audio_active;
#ifdef NXT_ROM_STOCK
    __atomic_store_n(&e.sink,GetAudioSink(),__ATOMIC_RELAXED);
#else
    e.sink=GetAudioSink();
#endif
    usbdac_audio_active=e.sink==2; bluetooth_audio_active=e.sink==1;
    if(e.device) { SDL_CloseAudioDevice(e.device);e.device=0; }
    int opened=open_audio();
    if(opened==0){sink_retry_started=0;if(*path&&state!=PLAYER_STATE_STOPPED){Player_seek(pos);if(was_usb&&!usbdac_audio_active&&nxt.unplug_pause)Player_pause();}}
    else {
        Uint32 now=SDL_GetTicks();bool first=!sink_retry_started;
        if(first)sink_retry_started=now?now:1;
        /* A USB card may enumerate before its PCM is ready. Keep recovery
         * pending after the initial fast retries; no new hotplug event is
         * guaranteed when the driver finally finishes starting. */
        sink_changed=true;sink_retry_at=now+((Uint32)(now-sink_retry_started)<5000?300:2000);
        if(first)LOG_info("Audio: waiting for output, sink=%d\n",GetAudioSink());
    }
    Player_initUSBHID(); Player_syncOutputVolume();
}
void Player_syncOutputVolume(void) { if(nxt.bit_perfect)return;Player_setVolume(1); NxtVolume_restore(); }
bool Player_outputReady(void) { return nxt.bit_perfect?NxtBit_ready():e.device!=0; }
bool Player_outputRetryPending(void) { return sink_changed; }
const char *Player_lastOutputError(void) { return nxt.bit_perfect?NxtBit_error():output_error; }
void Player_resumeAudio(void) { if(e.device)SDL_PauseAudioDevice(e.device,0); }
void Player_pauseAudio(void) { if(e.device)SDL_PauseAudioDevice(e.device,1); }
void Player_resetSampleRate(void) { Player_setSampleRate(44100); }
void Player_setSampleRate(int rate) { if(rate<8000||rate>192000)return;pthread_mutex_lock(&e.lock);e.radio_rate=rate;e.radio_epoch++;pthread_mutex_unlock(&e.lock); }
bool Player_isBluetoothActive(void) { return bluetooth_audio_active; }
bool Player_isUSBDACActive(void) { return usbdac_audio_active; }
#include "nxt_hid.inc"
