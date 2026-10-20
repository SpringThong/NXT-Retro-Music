#include "nxt.h"
#include "player.h"
#include "module_player.h"
#include "radio.h"
#include "podcast.h"
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#define NXT_PLAYBACK_H
NxtConfig nxt;
static PlayerState playing=PLAYER_STATE_PLAYING;
static int position=12345,source,music_gain=20,saved,skips,stops;
static TrackInfo info={.title="NXT Fixture Song",.artist="Fixture Artist",.album="Fixture Album",.duration_ms=180000,.sample_rate=44100};
static PlaylistContext queue={.track_count=3,.current_index=1};
const TrackInfo *NxtPlayback_info(void){return &info;}
PlayerState NxtPlayback_state(void){return playing;}
int NxtPlayback_position(void){return position;}
int NxtPlayback_duration(void){return info.duration_ms;}
const char *NxtPlayback_path(void){return "/Music/fixture.flac";}
SDL_Surface *NxtPlayback_art(void){return NULL;}
unsigned NxtPlayback_generation(void){return 1;}
void NxtPlayback_toggle(void){playing=playing==PLAYER_STATE_PLAYING?PLAYER_STATE_PAUSED:PLAYER_STATE_PLAYING;}
void NxtPlayback_seek(int p){position=p;}
bool NxtStartup_preview(void){return false;}
int GetMusicVolume(void){return music_gain;}
void SetMusicVolume(int v){music_gain=v;}
int Player_getOutputRate(void){return 48000;}
int Player_getVisBuffer(int16_t *p,int n){for(int i=0;i<n;i++)p[i]=i;return n;}
bool NxtSpotify_active(void){return source==1;}
bool NxtSoundCloud_active(void){return source==2;}
void NxtSpotify_skip(int d){skips+=10*d;}
void NxtSoundCloud_skip(int d){skips+=100*d;}
int NxtSpotify_queueCount(void){return 4;}int NxtSpotify_queueIndex(void){return 2;}
int NxtSoundCloud_queueCount(void){return 5;}int NxtSoundCloud_queueIndex(void){return 3;}
bool PlayerModule_isActive(void){return !source;}
PlaylistContext *PlayerModule_queue(void){return &queue;}
void PlayerModule_nextTrack(void){skips++;}void PlayerModule_prevTrack(void){skips--;}
bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
const RadioMetadata *Radio_getMetadata(void){return NULL;}
RadioState Radio_getState(void){return RADIO_STATE_STOPPED;}
const char *Radio_getCurrentUrl(void){return "";}
float Radio_getBufferLevel(void){return 0;}
SDL_Surface *Radio_getAlbumArt(void){return NULL;}
bool Nxt_save(void){saved++;return true;}
void Nxt_stop(void){playing=PLAYER_STATE_STOPPED;stops++;}
uint32_t SDL_GetTicks(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint32_t)(t.tv_sec*1000+t.tv_nsec/1000000);}
static unsigned char captured[8192];static size_t captured_length;
static ssize_t capture_send(int fd,const void *data,size_t size,int flags){assert(size<=sizeof(captured)-captured_length);memcpy(captured+captured_length,data,size);captured_length+=size;return size;}
#define send capture_send
#include "nxt_nxredux.c"
int main(void){
 MusicResponseWire r;MusicIntRequest v={.value=12};
 handle(MUSIC_CMD_SNAPSHOT,NULL,0,&r);assert(!r.status&&r.snapshot.loaded&&r.snapshot.state==MUSIC_STATE_PLAYING);
 assert(!strcmp(r.snapshot.title,info.title)&&r.snapshot.position_ms==12345&&r.snapshot.source_sample_rate==44100&&r.snapshot.output_sample_rate==48000);
 assert(r.snapshot.queue_count==3&&r.snapshot.queue_index==1&&r.snapshot.visualization_samples[100]==100);
 handle(MUSIC_CMD_PAUSE,NULL,0,&r);assert(playing==PLAYER_STATE_PAUSED&&r.snapshot.state==MUSIC_STATE_PAUSED);
 handle(MUSIC_CMD_PLAY,NULL,0,&r);assert(playing==PLAYER_STATE_PLAYING);
 handle(MUSIC_CMD_SEEK,&v,sizeof(v),&r);assert(position==12);
 handle(MUSIC_CMD_SET_VOLUME,&v,sizeof(v),&r);assert(music_gain==12&&r.snapshot.volume>.59f&&r.snapshot.volume<.61f);
 v.value=2;handle(MUSIC_CMD_REPEAT,&v,sizeof(v),&r);assert(nxt.repeat_mode==2&&saved);
 handle(MUSIC_CMD_NEXT,NULL,0,&r);assert(skips==1);source=1;handle(MUSIC_CMD_NEXT,NULL,0,&r);assert(skips==11&&r.snapshot.queue_count==4);
 source=2;handle(MUSIC_CMD_PREVIOUS,NULL,0,&r);assert(skips==-89&&r.snapshot.queue_count==5);source=0;
 handle(MUSIC_CMD_SLEEP,NULL,0,&r);assert(playing==PLAYER_STATE_PAUSED);handle(MUSIC_CMD_WAKE,NULL,0,&r);assert(playing==PLAYER_STATE_PLAYING);
 handle(MUSIC_CMD_PAUSE,&v,sizeof(v),&r);assert(r.status==MUSIC_STATUS_BAD_REQUEST&&playing==PLAYER_STATE_PLAYING);
 handle(MUSIC_CMD_LOAD,NULL,0,&r);assert(r.status==MUSIC_STATUS_UNAVAILABLE);
 handle(MUSIC_CMD_SHUTDOWN,NULL,0,&r);assert(r.status==MUSIC_STATUS_UNAVAILABLE&&playing==PLAYER_STATE_PLAYING);
 /* Wire parser and response writer use captured transport: this executor
  * prohibits AF_UNIX sockets. This does not claim physical Home testing. */
 Client client={.fd=open("/dev/null",O_RDWR)};assert(client.fd>=0);
 MusicFrameHeader h={.magic=MUSIC_SERVICE_MAGIC,.version=MUSIC_SERVICE_PROTOCOL_VERSION,.command=MUSIC_CMD_SNAPSHOT,.request_id=99};
 memcpy(client.input,&h,5);client.input_length=5;process_client(&client,handle);assert(!captured_length);
 memcpy(client.input+5,(char*)&h+5,sizeof(h)-5);client.input_length=sizeof(h);process_client(&client,handle);
 MusicFrameHeader reply;memcpy(&reply,captured,sizeof(reply));assert(reply.request_id==99&&reply.payload_length==sizeof(r));memcpy(&r,captured+sizeof(reply),sizeof(r));assert(!r.status&&r.snapshot.loaded);close_client(&client);
 client.fd=open("/dev/null",O_RDWR);captured_length=0;h.version=999;memcpy(client.input,&h,sizeof(h));client.input_length=sizeof(h);process_client(&client,handle);assert(client.fd==-1&&!captured_length);
 handle(MUSIC_CMD_STOP,NULL,0,&r);assert(stops==1&&!r.snapshot.loaded);
 snprintf(socket_path,sizeof(socket_path),"%s",getenv("NX_MUSIC_SOCKET"));snprintf(lock_path,sizeof(lock_path),"%s.lock",socket_path);
 assert(!acquire_lock());assert(acquire_lock()!=0);assert(!access(lock_path,F_OK));for(int i=0;i<MAX_CLIENTS;i++)clients[i].fd=-1;
 connected=true;NxtNX_quit();assert(access(lock_path,F_OK));
 puts("PASS Home bridge: ARM64 wire frames, metadata, waveform, queue controls, music gain, malformed clients, owner lock and cleanup; Unix transport mocked.");
}
