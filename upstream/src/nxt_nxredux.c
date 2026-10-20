/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_nxredux.h"
#if defined(NXT_ROM_NXREDUX) && !defined(PLATFORM_DESKTOP)
#include "nxt.h"
#include "nxt_playback.h"
#include "module_player.h"
#include "radio.h"
#include "podcast.h"
#include "msettings.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "nxt_nx_server.inc"
extern void SetMusicVolume(int);
static bool connected,sleep_paused,attempted_init;
static uint32_t next_attempt;
static unsigned cover_generation=~0u;
static char cover_path[512];
static SDL_Surface *cover_seen;
static void skip(int direction){
 if(NxtSoundCloud_active())NxtSoundCloud_skip(direction);
 else if(NxtSpotify_active())NxtSpotify_skip(direction);
 else if(PlayerModule_isActive()){if(direction>0)PlayerModule_nextTrack();else PlayerModule_prevTrack();}
}
static void snapshot(MusicSnapshotWire *s){
 memset(s,0,sizeof(*s));const TrackInfo *info=NxtPlayback_info();const char *path=NxtPlayback_path();
 PlayerState state=NxtPlayback_state();bool loaded=path&&*path&&state!=PLAYER_STATE_STOPPED&&!NxtStartup_preview();
 s->state=state==PLAYER_STATE_PLAYING?MUSIC_STATE_PLAYING:state==PLAYER_STATE_PAUSED?MUSIC_STATE_PAUSED:MUSIC_STATE_STOPPED;
 s->loaded=loaded;s->source=loaded?MUSIC_SOURCE_LOCAL:MUSIC_SOURCE_NONE;s->source_state=s->state;
 s->position_ms=NxtPlayback_position();s->duration_ms=NxtPlayback_duration();s->volume=GetMusicVolume()/20.f;
 s->repeat=nxt.repeat_mode;s->shuffle=nxt.shuffle;s->playback_speed=1;
 s->source_sample_rate=info?info->sample_rate:0;s->output_sample_rate=Player_getOutputRate();s->audio_open=loaded;
 if(path)snprintf(s->current_file,sizeof(s->current_file),"%s",path);
 if(info){snprintf(s->title,sizeof(s->title),"%s",info->title);snprintf(s->artist,sizeof(s->artist),"%s",info->artist);snprintf(s->album,sizeof(s->album),"%s",info->album);}
 snprintf(s->artwork_path,sizeof(s->artwork_path),"%s",cover_path);
 if(state==PLAYER_STATE_PLAYING&&!nxt.bit_perfect){int16_t samples[MUSIC_VIS_SAMPLE_COUNT]={0};Player_getVisBuffer(samples,MUSIC_VIS_SAMPLE_COUNT);memcpy(s->visualization_samples,samples,sizeof(samples));}
 if(NxtSoundCloud_active()){s->queue_count=NxtSoundCloud_queueCount();s->queue_index=NxtSoundCloud_queueIndex();}
 else if(NxtSpotify_active()){s->queue_count=NxtSpotify_queueCount();s->queue_index=NxtSpotify_queueIndex();}
 else if(PlayerModule_isActive()){PlaylistContext *q=PlayerModule_queue();if(q){s->queue_count=q->track_count;s->queue_index=q->current_index;}}
 if(Radio_isActive()){
  const RadioMetadata *meta=Radio_getMetadata();s->loaded=1;s->source=MUSIC_SOURCE_RADIO;s->state=Radio_getState()==RADIO_STATE_PLAYING?MUSIC_STATE_PLAYING:MUSIC_STATE_PAUSED;
  s->source_state=Radio_getState();s->position_ms=s->duration_ms=0;s->capabilities=0;s->queue_count=0;
  snprintf(s->current_file,sizeof(s->current_file),"%s",Radio_getCurrentUrl());
  if(meta){snprintf(s->title,sizeof(s->title),"%s",meta->title);snprintf(s->artist,sizeof(s->artist),"%s",meta->artist);snprintf(s->album,sizeof(s->album),"%s",meta->station_name);s->radio_bitrate=meta->bitrate;}
  s->radio_buffer_percent=(int)(Radio_getBufferLevel()*100);return;
 }
 if(Podcast_isActive())s->source=MUSIC_SOURCE_PODCAST;
 if(loaded){s->capabilities=MUSIC_CAP_PLAY|MUSIC_CAP_PAUSE;if(s->duration_ms>0)s->capabilities|=MUSIC_CAP_SEEK;
  if(s->queue_count>1)s->capabilities|=MUSIC_CAP_NEXT|MUSIC_CAP_PREVIOUS;}
}
static void handle(uint16_t command,const void *payload,size_t length,MusicResponseWire *response){
 memset(response,0,sizeof(*response));bool known=true;int value=0;
 bool takes_int=command==MUSIC_CMD_SEEK||command==MUSIC_CMD_SET_SHUFFLE||command==MUSIC_CMD_SET_VOLUME||command==MUSIC_CMD_REPEAT;
 if((takes_int&&length!=sizeof(MusicIntRequest))||(!takes_int&&length!=0)){
  response->status=MUSIC_STATUS_BAD_REQUEST;snprintf(response->error,sizeof(response->error),"Invalid command payload");snapshot(&response->snapshot);return;
 }
 if(takes_int){MusicIntRequest request;memcpy(&request,payload,sizeof(request));value=request.value;}
 switch(command){
 case MUSIC_CMD_SNAPSHOT:break;
 case MUSIC_CMD_PLAY:if(NxtPlayback_state()==PLAYER_STATE_PAUSED)NxtPlayback_toggle();break;
 case MUSIC_CMD_PAUSE:if(NxtPlayback_state()==PLAYER_STATE_PLAYING)NxtPlayback_toggle();break;
 case MUSIC_CMD_TOGGLE:NxtPlayback_toggle();break;
 case MUSIC_CMD_NEXT:skip(1);break;
 case MUSIC_CMD_PREVIOUS:skip(-1);break;
 case MUSIC_CMD_STOP:Nxt_stop();break;
 case MUSIC_CMD_SEEK:if(value>=0&&value<=NxtPlayback_duration())NxtPlayback_seek(value);else response->status=MUSIC_STATUS_BAD_REQUEST;break;
 case MUSIC_CMD_REPEAT:if(value>=0&&value<=2){nxt.repeat_mode=value;Nxt_save();}else response->status=MUSIC_STATUS_BAD_REQUEST;break;
 case MUSIC_CMD_SHUFFLE:nxt.shuffle=!nxt.shuffle;Nxt_save();break;
 case MUSIC_CMD_SET_SHUFFLE:if(value==0||value==1){nxt.shuffle=value;Nxt_save();}else response->status=MUSIC_STATUS_BAD_REQUEST;break;
 case MUSIC_CMD_SET_VOLUME:if(value>=0&&value<=20)SetMusicVolume(value);else response->status=MUSIC_STATUS_BAD_REQUEST;break;
 case MUSIC_CMD_SLEEP:sleep_paused=NxtPlayback_state()==PLAYER_STATE_PLAYING;if(sleep_paused)NxtPlayback_toggle();break;
 case MUSIC_CMD_WAKE:if(sleep_paused&&NxtPlayback_state()==PLAYER_STATE_PAUSED)NxtPlayback_toggle();sleep_paused=false;break;
 default:known=false;break;
 }
 if(!known){response->status=MUSIC_STATUS_UNAVAILABLE;snprintf(response->error,sizeof(response->error),"NXT is active; open NXT to select music");}
 snapshot(&response->snapshot);
}
static void update_cover(void){
 unsigned generation=NxtPlayback_generation();SDL_Surface *source=Radio_isActive()?Radio_getAlbumArt():NxtPlayback_art();if(cover_generation==generation&&cover_seen==source)return;cover_generation=generation;cover_seen=source;
 if(*cover_path)unlink(cover_path);cover_path[0]=0;if(!source)return;
 int w=source->w,h=source->h;if(w<=0||h<=0)return;
 float factor=w>h?160.f/w:160.f/h;if(factor>1)factor=1;w=(int)(w*factor);h=(int)(h*factor);if(w<1||h<1)return;
 SDL_Surface *small=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);if(!small)return;
 SDL_BlitScaled(source,NULL,small,NULL);char path[512],temp[544];
 const char *base=getenv(MUSIC_SERVICE_SOCKET_ENV);if(!base||!*base)base=MUSIC_SERVICE_DEFAULT_SOCKET;
 snprintf(path,sizeof(path),"%s.nxt-cover.png",base);snprintf(temp,sizeof(temp),"%s.tmp",path);
 if(IMG_SavePNG(small,temp)==0&&rename(temp,path)==0)snprintf(cover_path,sizeof(cover_path),"%s",path);else unlink(temp);
 SDL_FreeSurface(small);
}
void NxtNX_tick(void){
 if(!connected){uint32_t now=SDL_GetTicks();if(attempted_init&&(int32_t)(now-next_attempt)<0)return;attempted_init=true;next_attempt=now+1000;
  if(!Radio_isActive()&&(NxtStartup_preview()||!NxtPlayback_path()||!*NxtPlayback_path()))return;
  connected=MusicServiceServer_open()==0;if(!connected)return;
 }
 update_cover();MusicServiceServer_poll(handle,0);
}
void NxtNX_quit(void){if(connected)MusicServiceServer_close();connected=false;attempted_init=false;if(*cover_path)unlink(cover_path);cover_path[0]=0;cover_generation=~0u;cover_seen=NULL;}
const char *NxtNX_status(void){return connected?"NX Redux Home: NXT connected":"NX Redux Home: waiting for music";}
#else
void NxtNX_tick(void){}
void NxtNX_quit(void){}
const char *NxtNX_status(void){return "";}
#endif
