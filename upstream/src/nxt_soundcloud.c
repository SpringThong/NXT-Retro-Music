#include "nxt_startup.h"
#include "nxt_audio_mode.h"
#include "nxt_custom.h"
#include "nxt_log.h"
/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_soundcloud.h"
#include "nxt_skin.h"
#include "nxt_controls.h"
#include "parson/parson.h"
#include "player.h"
#include "background.h"
#include "radio.h"
#include "podcast.h"
#include "api.h"
#include "msettings.h"
#include "keyboard.h"
#include "file_utils.h"
#include "toast.h"
#include "debug.h"
#include "lyrics.h"
#include "resume.h"
#include "module_player.h"
#include "spectrum.h"
#include "settings.h"
#include "ui_music_playing.h"
#include <SDL2/SDL_image.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nxt_soundcloud_pcm.inc"
#define SC_MAX 20
static pid_t child=-1;
static int channel=-1;
static char incoming[65536];static size_t used;
static bool connected,playing,track_loaded,busy,has_next,cover_dirty,stop_requested;
static int request_serial,expected_serial,revision,offset,position_ms,duration_ms,queue_count,queue_index,last_sink=-1,last_dac=-1;
static char status[256],creds[1024],diag_path[1200],cover_path[1024],query[512],playlist_id[64],result_kind[32];
typedef struct {char title[256],artist[256],album[256],uri[128],id[64],cover[512],permalink[512];int duration;} Track;
static Track *results,*queue,current;
static size_t result_capacity,queue_capacity;
static bool queue_more,queue_request,queue_waiting;
static int next_offset,queue_offset,browser_depth;
static uint32_t queue_retry;
static char queue_kind[32],queue_query[256],queue_id[64];
static int result_count;
static SDL_Surface *cover;
static TrackInfo track_info;
static unsigned track_generation;
static uint32_t loading_since;static bool pcm_seen;
static bool favorite,favorite_pending,favorite_change;
static char lyrics_path[1024],lyrics_status[160];
static char source_qr[57][58];static int source_qr_count;
static void copy(char *out,size_t n,const char *in){if(!in)in="";size_t k=strlen(in);if(k>=n){k=n-1;while(k&&((unsigned char)in[k]&0xc0)==0x80)k--;}memcpy(out,in,k);out[k]=0;for(size_t i=0;i<k;i++)if(out[i]=='\n'||out[i]=='\r')out[i]=' ';}
static void paths(void){if(*creds)return;char dir[1024];userdata_snpath("soundcloud",dir,sizeof(dir));mkdir(dir,0700);userdata_snpath("soundcloud/credentials.json",creds,sizeof(creds));userdata_snpath("soundcloud/cover.jpg",cover_path,sizeof(cover_path));userdata_snpath("soundcloud/lyrics.txt",lyrics_path,sizeof(lyrics_path));if(getcwd(dir,sizeof(dir)))snprintf(diag_path,sizeof(diag_path),"%s/NXT_SoundCloud_Log.txt",dir);}
static void diagnostic(const char *message){if(!*diag_path)return;int fd=open(diag_path,O_WRONLY|O_CREAT|O_APPEND,0600);if(fd>=0){dprintf(fd,"%s\n",message);close(fd);}}
static bool send_value(JSON_Value *v){char *s=json_serialize_to_string(v);if(!s)return false;size_t n=strlen(s);char *line=malloc(n+2);if(!line){json_free_serialized_string(s);return false;}memcpy(line,s,n);line[n]='\n';ssize_t got=channel>=0?send(channel,line,n+1,MSG_NOSIGNAL|MSG_DONTWAIT):-1;free(line);json_free_serialized_string(s);return got==(ssize_t)n+1;}
static bool command(const char *name){JSON_Value *v=json_value_init_object();json_object_set_string(json_value_get_object(v),"cmd",name);bool ok=send_value(v);json_value_free(v);return ok;}
void NxtSoundCloud_quit(void){
 bool had_track=track_loaded;if(Background_getActive()==BG_SOUNDCLOUD)Background_setActive(BG_NONE);
 pcm_close();free(results);free(queue);results=queue=NULL;result_count=queue_count=0;result_capacity=queue_capacity=0;queue_more=queue_request=queue_waiting=false;if(channel>=0){command("quit");close(channel);channel=-1;}
 if(child>0){int result=0;for(int i=0;i<10;i++){pid_t p=waitpid(child,&result,WNOHANG);if(p==child||p<0){child=-1;break;}usleep(10000);}if(child>0){kill(child,SIGTERM);for(int i=0;i<10;i++){if(waitpid(child,&result,WNOHANG)!=0){child=-1;break;}usleep(10000);}}if(child>0){kill(child,SIGKILL);waitpid(child,NULL,0);child=-1;}}
 connected=playing=track_loaded=busy=false;used=0;last_sink=last_dac=-1;source_qr_count=0;creds[0]=cover_path[0]=lyrics_path[0]=0;
 memset(&current,0,sizeof(current));memset(&track_info,0,sizeof(track_info));
 if(cover){SDL_FreeSurface(cover);cover=NULL;}cover_dirty=false;favorite_pending=false;if(had_track)Lyrics_clear();
 if(had_track)ModuleCommon_setAutosleepDisabled(false);
}
bool NxtSoundCloud_active(void){return track_loaded;}
bool NxtSoundCloud_isPlaying(void){return playing;}
bool NxtSoundCloud_hasMetadata(void){return child>0&&*current.uri&&*track_info.title;}
const TrackInfo *NxtSoundCloud_info(void){return &track_info;}
const char *NxtSoundCloud_uri(void){return current.uri;}
SDL_Surface *NxtSoundCloud_art(void){return cover;}
unsigned NxtSoundCloud_generation(void){return track_generation;}
const char *NxtSoundCloud_lyricsStatus(void){return *lyrics_status?lyrics_status:Nxt_text("No matching lyrics on LRCLIB","LRCLIB chưa có lời khớp bài này");}
bool NxtSoundCloud_isFavorite(void){return favorite;}
int NxtSoundCloud_position(void){pthread_mutex_lock(&pcm_lock);double elapsed=pcm_position;pthread_mutex_unlock(&pcm_lock);int64_t pos=position_ms+(int64_t)elapsed;if(pos<0)pos=0;if(duration_ms>0&&pos>duration_ms)pos=duration_ms;return (int)pos;}
void NxtSoundCloud_togglePause(void){if(!track_loaded)return;bool resume=!playing;if(command(resume?"resume":"pause")){if(resume)Player_resumeAudio();pcm_enable(resume);playing=resume;}}
void NxtSoundCloud_seek(int ms){
 if(!track_loaded)return;if(ms<0)ms=0;if(duration_ms>0&&ms>duration_ms)ms=duration_ms;
 pcm_begin();pcm_enable(playing);position_ms=ms;loading_since=SDL_GetTicks();pcm_seen=false;
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd","seek");json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);json_object_set_number(o,"position_ms",ms);json_object_set_boolean(o,"paused",!playing);
 if(!send_value(v))Toast_show(Nxt_text("Seek failed; try again","Chưa tua được; thử lại"),1800);json_value_free(v);
}
static void favorite_request(bool change){
 if(!connected||!track_loaded||(change&&favorite_pending))return;
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",change?"favorite":"check_favorite");json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);json_object_set_boolean(o,"saved",!favorite);
 json_object_set_string(o,"title",current.title);json_object_set_string(o,"artist",current.artist);json_object_set_string(o,"album",current.album);json_object_set_string(o,"cover",current.cover);json_object_set_string(o,"permalink",current.permalink);json_object_set_number(o,"duration_ms",current.duration);
 if(send_value(v)){favorite_pending=change;favorite_change=change;}json_value_free(v);
}
void NxtSoundCloud_favorite(void){favorite_request(true);}
static bool start(void){
 if(child>0)return true;paths();
 if(*diag_path){int fd=open(diag_path,O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd>=0){fchmod(fd,0600);close(fd);}}diagnostic("Frontend build: " NXT_BUILD_REVISION);
 int audio[2];if(socketpair(AF_UNIX,SOCK_SEQPACKET,0,audio)){copy(status,sizeof(status),"PCM IPC unavailable");return false;}
 int sv[2];if(socketpair(AF_UNIX,SOCK_STREAM,0,sv)){close(audio[0]);close(audio[1]);copy(status,sizeof(status),"IPC unavailable");return false;}
 fcntl(audio[0],F_SETFD,FD_CLOEXEC);fcntl(audio[1],F_SETFD,FD_CLOEXEC);int size=262144;setsockopt(audio[1],SOL_SOCKET,SO_SNDBUF,&size,sizeof(size));
 if(!pcm_open(audio[0])){close(audio[1]);close(sv[0]);close(sv[1]);pcm_close();return false;}
 setenv("NXT_SOUNDCLOUD_PCM_FD","3",1);fcntl(sv[0],F_SETFD,FD_CLOEXEC);fcntl(sv[1],F_SETFD,FD_CLOEXEC);
 setenv("NXT_SOUNDCLOUD_CREDENTIALS",creds,1);setenv("NXT_SOUNDCLOUD_COVER",cover_path,1);setenv("NXT_SOUNDCLOUD_LYRICS",lyrics_path,1);pid_t parent=getpid(),pid=fork();
 if(pid==0){prctl(PR_SET_PDEATHSIG,SIGTERM);if(getppid()!=parent)_exit(125);dup2(sv[1],STDIN_FILENO);dup2(sv[1],STDOUT_FILENO);dup2(audio[1],3);fcntl(3,F_SETFD,0);for(int fd=4;fd<4096;fd++)close(fd);execl("./bin/tg5040/nxt-soundcloud-backend","nxt-soundcloud-backend",(char*)NULL);_exit(127);}
 close(audio[1]);close(sv[1]);if(pid<0){close(sv[0]);pcm_close();return false;}child=pid;channel=sv[0];fcntl(channel,F_SETFL,fcntl(channel,F_GETFL)|O_NONBLOCK);used=0;connected=false;last_sink=GetAudioSink();last_dac=NxtHardware_hasDAC();busy=true;copy(status,sizeof(status),Nxt_text("Connecting to SoundCloud…","Đang kết nối SoundCloud…"));command("login");return true;
}
static bool append_track(Track **list,int *count,size_t *capacity,const Track *t){for(int i=0;i<*count;i++)if((*t->uri&&!strcmp((*list)[i].uri,t->uri))||(!*t->uri&&*t->id&&!strcmp((*list)[i].id,t->id)))return true;if((size_t)*count==*capacity){size_t cap=*capacity?*capacity*2:32;Track *n=realloc(*list,cap*sizeof(**list));if(!n)return false;*list=n;*capacity=cap;}(*list)[(*count)++]=*t;return true;}
static void queue_fetch(void){if(!queue_more||busy||browser_depth||(int32_t)(SDL_GetTicks()-queue_retry)<0)return;JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",queue_kind);json_object_set_string(o,"query",queue_query);json_object_set_string(o,"id",queue_id);json_object_set_number(o,"offset",queue_offset);json_object_set_number(o,"request_id",++request_serial);expected_serial=request_serial;if(send_value(v)){busy=true;queue_request=true;}else queue_retry=SDL_GetTicks()+3000;json_value_free(v);}
static void capture_queue(const char *kind){queue_count=0;for(int i=0;i<result_count;i++)if(!append_track(&queue,&queue_count,&queue_capacity,&results[i]))break;copy(queue_kind,sizeof(queue_kind),kind);copy(queue_query,sizeof(queue_query),query);copy(queue_id,sizeof(queue_id),playlist_id);queue_more=has_next;queue_offset=next_offset;queue_waiting=false;queue_retry=0;}
static void request(const char *kind,int page){
 if(!connected)return;queue_request=false;if(page==0){result_count=0;has_next=false;next_offset=0;}JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",kind);json_object_set_number(o,"request_id",++request_serial);expected_serial=request_serial;json_object_set_number(o,"offset",page);json_object_set_string(o,"query",query);json_object_set_string(o,"id",playlist_id);
 if(send_value(v)){busy=true;copy(status,sizeof(status),Nxt_text("Loading…","Đang tải…"));}json_value_free(v);
}
static void select_track(int index){
 if(index<0||index>=queue_count||!connected)return;
 NxtAudio_streaming();
 if(Background_getActive()!=BG_SOUNDCLOUD)Background_stopAll();
 if(PlayerModule_isActive())Resume_updatePosition(Player_getPosition());PlayerModule_markStopped();Player_stop();Player_resumeAudio();
 pcm_begin();pcm_seen=false;loading_since=SDL_GetTicks();stop_requested=false;queue_index=index;current=queue[index];position_ms=0;duration_ms=current.duration;track_loaded=true;playing=false;Background_setActive(BG_SOUNDCLOUD);
 memset(&track_info,0,sizeof(track_info));copy(track_info.title,sizeof(track_info.title),current.title);copy(track_info.artist,sizeof(track_info.artist),current.artist);copy(track_info.album,sizeof(track_info.album),current.album);copy(track_info.codec,sizeof(track_info.codec),"SoundCloud");track_info.channels=2;track_info.duration_ms=duration_ms;track_info.track_number=index+1;track_generation++;
 favorite=favorite_pending=favorite_change=false;Lyrics_clear();unlink(lyrics_path);copy(lyrics_status,sizeof(lyrics_status),Nxt_text("Looking for matching lyrics…","Đang tìm lời khớp bài…"));
 if(cover){SDL_FreeSurface(cover);cover=NULL;}unlink(cover_path);source_qr_count=0;
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd","play");json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);send_value(v);json_value_free(v);
 copy(status,sizeof(status),Nxt_text("Buffering…","Đang tải âm thanh…"));
 if(*current.cover){v=json_value_init_object();o=json_value_get_object(v);json_object_set_string(o,"cmd","cover");json_object_set_string(o,"url",current.cover);json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);send_value(v);json_value_free(v);}
 v=json_value_init_object();o=json_value_get_object(v);json_object_set_string(o,"cmd","lyrics");json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);json_object_set_string(o,"title",current.title);json_object_set_string(o,"artist",current.artist);json_object_set_number(o,"duration_ms",duration_ms);send_value(v);json_value_free(v);favorite_request(false);ModuleCommon_setAutosleepDisabled(true);
}
static int adjacent(int direction){if(!queue_count)return -1;if(nxt.shuffle&&queue_count>1){int next=rand()%(queue_count-1);return next>=queue_index?next+1:next;}int next=queue_index+direction;if(direction>0&&next>=queue_count&&queue_more)return -1;if(next<0||next>=queue_count)return nxt.repeat_mode==2?(next+queue_count)%queue_count:-1;return next;}
static void stop_track(void){command("stop");pcm_enable(false);playing=track_loaded=false;favorite_pending=false;if(Background_getActive()==BG_SOUNDCLOUD)Background_setActive(BG_NONE);ModuleCommon_setAutosleepDisabled(false);}
void NxtSoundCloud_skip(int direction){if(direction==1||direction==-1)select_track(adjacent(direction));}
int NxtSoundCloud_queueCount(void){return queue_count;}
int NxtSoundCloud_queueIndex(void){return queue_index;}
static const char *friendly(const char *code){
 if(!strcmp(code,"credentials_invalid"))return Nxt_text("Invalid API configuration; import it again","Cấu hình API không hợp lệ; nhập lại");
 if(!strcmp(code,"extractor_missing"))return Nxt_text("SoundCloud runtime missing; reinstall the complete Pak","Thiếu công cụ SoundCloud; cài lại đầy đủ Pak");
 if(!strcmp(code,"extractor_timeout"))return Nxt_text("SoundCloud timed out; check Wi-Fi and retry","SoundCloud phản hồi chậm; kiểm tra Wi-Fi và thử lại");
 if(!strcmp(code,"extractor_failed"))return Nxt_text("SoundCloud extraction failed; check Wi-Fi / update app","Chưa lấy được nguồn; kiểm tra Wi-Fi / cập nhật app");
 if(!strcmp(code,"track_restricted")||!strcmp(code,"api_http_403"))return Nxt_text("SoundCloud did not provide a playable stream; retry / send log","SoundCloud chưa cấp luồng phát phù hợp; thử lại / gửi log");
 if(!strcmp(code,"api_http_404"))return Nxt_text("Track / playlist is unavailable","Bài / playlist không còn khả dụng");
 if(!strcmp(code,"api_http_429"))return Nxt_text("SoundCloud limit reached; retry later","SoundCloud giới hạn lượt; thử lại sau");
 if(!strcmp(code,"network_error")||!strcmp(code,"network_ca_invalid"))return Nxt_text("Network error; check Wi-Fi and system time","Lỗi mạng; kiểm tra Wi-Fi và giờ máy");
 if(!strcmp(code,"link_invalid"))return Nxt_text("Enter a public HTTPS SoundCloud link","Nhập liên kết HTTPS SoundCloud công khai");
 if(!strncmp(code,"hls_",4)||!strcmp(code,"audio_format_unsupported"))return Nxt_text("This audio stream is not supported","Luồng âm thanh này chưa được hỗ trợ");
 if(!strncmp(code,"pcm_",4))return Nxt_text("Audio channel error; reopen the app","Lỗi kênh âm thanh; mở lại app");
 if(!strcmp(code,"decode_failed")||!strcmp(code,"track_unavailable"))return Nxt_text("Cannot decode this track","Chưa phát được bài này");
 if(!strncmp(code,"media_http_",11))return Nxt_text("Audio link unavailable; replay the track","Liên kết âm thanh hết hạn; phát lại bài");
 if(!strncmp(code,"media_",6)||!strcmp(code,"response_too_large")||!strcmp(code,"api_invalid_json"))return Nxt_text("Unsupported SoundCloud response; send log","Phản hồi SoundCloud chưa hỗ trợ; gửi log");
 return code;
}
static void event(const char *line){
 JSON_Value *v=json_parse_string(line);JSON_Object *o=json_value_get_object(v);const char *e=o?json_object_get_string(o,"event"):NULL;if(!e){json_value_free(v);return;}
 if(!strcmp(e,"ready")||!strcmp(e,"logged_in")||!strcmp(e,"error")||!strcmp(e,"paired")||!strcmp(e,"login_progress"))NxtLog_step("SoundCloud",e,strcmp(e,"error")!=0,"Provider event received");
 const char *uri=json_object_get_string(o,"uri");if(uri&&*current.uri&&strcmp(uri,current.uri)){json_value_free(v);return;}
 const char *operation=json_object_get_string(o,"operation");
 bool audio_event=!strcmp(e,"playing")||(!strcmp(e,"error")&&operation&&(!strcmp(operation,"play")||!strcmp(operation,"seek")));
 if(audio_event&&json_object_has_value_of_type(o,"stream_id",JSONNumber)&&(uint32_t)json_object_get_number(o,"stream_id")!=pcm_epoch){json_value_free(v);return;}
 if(json_object_has_value_of_type(o,"request_id",JSONNumber)&&(int)json_object_get_number(o,"request_id")!=expected_serial){json_value_free(v);return;}
 if(!strcmp(e,"logged_in")){connected=true;busy=false;copy(status,sizeof(status),Nxt_text("Ready · public tracks","Sẵn sàng · nhạc công khai"));diagnostic("Public SoundCloud via yt-dlp; no personal credentials");}
 else if(!strcmp(e,"error")){busy=false;if(queue_request){queue_request=false;queue_retry=SDL_GetTicks()+15000;}const char *reason=json_object_get_string(o,"reason");if(!reason)reason="unknown";const char *op=json_object_get_string(o,"operation");
  if(op&&(!strcmp(op,"play")||!strcmp(op,"seek")))stop_track();if(op&&!strcmp(op,"login"))connected=false;
  copy(status,sizeof(status),friendly(reason));diagnostic(reason);if(op&&(!strcmp(op,"play")||!strcmp(op,"seek")))Toast_show(status,4000);
 }else if(!strcmp(e,"results")){JSON_Array *a=json_object_get_array(o,"items");bool target=queue_request;int page=(int)json_object_get_number(o,"offset");if(!target&&page==0)result_count=0;int before=target?queue_count:result_count;bool memory_ok=true;
 for(size_t i=0;i<json_array_get_count(a);i++){JSON_Object *x=json_array_get_object(a,i);if(!x)continue;Track value={0};Track *t=&value;
#define FIELD(name) copy(t->name,sizeof(t->name),json_object_get_string(x,#name))
 FIELD(title);FIELD(artist);FIELD(album);FIELD(uri);FIELD(id);FIELD(cover);FIELD(permalink);
#undef FIELD
 t->duration=(int)json_object_get_number(x,"duration_ms");if(!append_track(target?&queue:&results,target?&queue_count:&result_count,target?&queue_capacity:&result_capacity,t)){memory_ok=false;break;}}
 int following=json_object_has_value(o,"next_offset")?(int)json_object_get_number(o,"next_offset"):page+20;bool more=memory_ok&&json_object_get_boolean(o,"has_next")==1&&following>page;busy=false;queue_request=false;
 if(target){queue_offset=following;queue_more=more;queue_retry=0;if(queue_waiting){if(queue_count>before){queue_waiting=false;select_track(before);}else if(!more){queue_waiting=false;stop_track();}}}
 else{copy(result_kind,sizeof(result_kind),json_object_get_string(o,"kind"));offset=page;next_offset=following;has_next=more;revision++;copy(status,sizeof(status),result_count?Nxt_text("A Play / open","A Phát / mở"):Nxt_text("No full-length items here · try another genre or page","Chưa có bài đầy đủ · thử thể loại hoặc trang khác"));}
 }else if(!strcmp(e,"playing")){
  if(track_loaded){pthread_mutex_lock(&pcm_lock);bool enabled=pcm_enabled;pthread_mutex_unlock(&pcm_lock);playing=enabled;
   int sr=(int)json_object_get_number(o,"sample_rate");if(sr>=8000&&sr<=192000)track_info.sample_rate=sr;
   const char *codec=json_object_get_string(o,"codec");copy(track_info.codec,sizeof(track_info.codec),codec?codec:"SoundCloud");track_info.bitrate=(int)json_object_get_number(o,"bitrate_kbps")*1000;copy(status,sizeof(status),Nxt_text("Playing · SoundCloud","Đang phát · SoundCloud"));int restore=NxtStartup_takeSeek("SoundCloud",current.uri);if(restore>0)NxtSoundCloud_seek(restore);
  }
 }else if(!strcmp(e,"cover"))cover_dirty=true;
 else if(!strcmp(e,"lyrics")){if(track_loaded&&Lyrics_loadTextFile(lyrics_path,duration_ms))copy(lyrics_status,sizeof(lyrics_status),Lyrics_isSynced()?Nxt_text("LRCLIB · synced lyrics","LRCLIB · lời đồng bộ"):Nxt_text("LRCLIB · scroll with ↑↓","LRCLIB · cuộn bằng ↑↓"));}
 else if(!strcmp(e,"lyrics_unavailable"))copy(lyrics_status,sizeof(lyrics_status),Nxt_text("No matching lyrics on LRCLIB","LRCLIB chưa có lời khớp bài này"));
 else if(!strcmp(e,"favorite")){favorite=json_object_get_boolean(o,"saved")==1;favorite_pending=false;if(favorite_change)Toast_show(favorite?Nxt_text("SoundCloud bookmark saved on this device","Đã lưu bài SoundCloud trên máy"):Nxt_text("Bookmark removed from this device","Đã bỏ bài đã lưu trên máy"),1800);favorite_change=false;}
 else if(!strcmp(e,"favorite_error")){favorite_pending=false;favorite_change=false;Toast_show(Nxt_text("Cannot save bookmark; check storage","Không lưu được bài; kiểm tra thẻ nhớ"),2000);}
 else if(!strcmp(e,"source_link")){JSON_Array *a=json_object_get_array(o,"qr");source_qr_count=0;size_t n=json_array_get_count(a);bool valid=n&&n<=57;for(size_t y=0;valid&&y<n;y++){const char *row=json_array_get_string(a,y);if(!row||strlen(row)!=n){valid=false;break;}for(size_t x=0;x<n;x++)if(row[x]!='0'&&row[x]!='1')valid=false;}if(valid){for(size_t y=0;y<n;y++)copy(source_qr[y],sizeof(source_qr[y]),json_array_get_string(a,y));source_qr_count=n;}}
 json_value_free(v);
}
void NxtSoundCloud_tick(void){
 if(track_loaded&&queue_more&&queue_count-queue_index<=6)queue_fetch();
 if(child<=0)return;
 char chunk[4096];ssize_t n;while((n=recv(channel,chunk,sizeof(chunk),MSG_DONTWAIT))>0){for(ssize_t i=0;i<n;i++){if(chunk[i]=='\n'){incoming[used]=0;event(incoming);used=0;}else if(used+1<sizeof(incoming))incoming[used++]=chunk[i];else{used=0;copy(status,sizeof(status),"SoundCloud response too large");}}}
 pid_t dead=waitpid(child,NULL,WNOHANG);if(n==0||dead==child){if(dead==child)child=-1;NxtSoundCloud_quit();copy(status,sizeof(status),Nxt_text("SoundCloud stopped; check log","SoundCloud đã dừng; kiểm tra log"));return;}
 if(track_loaded){pthread_mutex_lock(&pcm_lock);unsigned packets=pcm_packets;bool ended=pcm_eof&&pcm_used>=pcm_count;int source_rate=pcm_source_rate;pthread_mutex_unlock(&pcm_lock);
  if(source_rate)track_info.sample_rate=source_rate;
  if(packets&&!pcm_seen){pcm_seen=true;diagnostic("Decoded PCM reached NXT audio engine");}
  if(ended){int next=nxt.repeat_mode==1?queue_index:adjacent(1);if(!stop_requested&&next>=0)select_track(next);else if(!stop_requested&&queue_more){queue_waiting=true;queue_fetch();copy(status,sizeof(status),Nxt_text("Loading next tracks…","Đang tải thêm bài…"));}else{position_ms=duration_ms;stop_track();copy(status,sizeof(status),Nxt_text("Queue ended","Hết hàng đợi"));}}
  if(!pcm_seen&&loading_since&&SDL_GetTicks()-loading_since>90000){copy(status,sizeof(status),Nxt_text("No audio received; check SoundCloud log","Chưa có âm thanh; kiểm tra log SoundCloud"));loading_since=0;}
 }
 if(*current.uri&&(Radio_isActive()||Podcast_isActive())){NxtSoundCloud_quit();return;}
 if(cover_dirty){cover_dirty=false;if(cover)SDL_FreeSurface(cover);cover=IMG_Load(cover_path);if(cover&&(cover->w>2048||cover->h>2048)){SDL_FreeSurface(cover);cover=NULL;}}
 if(playing&&nxt.unplug_pause&&last_dac==1&&!NxtHardware_hasDAC()){pcm_enable(false);command("pause");playing=false;copy(status,sizeof(status),Nxt_text("DAC removed; paused","Đã rút DAC; tạm dừng"));}last_dac=NxtHardware_hasDAC();
 if(GetAudioSink()!=last_sink){last_sink=GetAudioSink();LOG_info("Online PCM: output handoff, sink=%d\n",last_sink);}
}
static ModuleExitReason setup_help(DisplayContext *d){
 int selected=0;const char *rows[]={Nxt_text("1. Enable Wi-Fi on the Brick","1. Bật Wi-Fi trên Brick"),Nxt_text("2. Search tracks or open a public SoundCloud link","2. Tìm bài hoặc mở liên kết SoundCloud công khai"),Nxt_text("No account / Artist Pro / personal API key needed","Không cần tài khoản / Artist Pro / khóa API riêng"),Nxt_text("yt-dlp is included in this Pak","Pak đã kèm sẵn yt-dlp"),Nxt_text("Tracks and public playlists are supported","Hỗ trợ bài hát và playlist công khai"),Nxt_text("Go+ / previews / region restrictions still apply","Giới hạn Go+ / nghe thử / khu vực vẫn áp dụng"),Nxt_text("R2 saves bookmarks on this device","R2 lưu bài yêu thích trên máy"),Nxt_text("Bookmarks are not synced to a SoundCloud account","Bài đã lưu không đồng bộ tài khoản SoundCloud"),Nxt_text("START opens the original-source QR","START mở QR nguồn bài gốc"),Nxt_text("Pause / seek / EQ / DAC use the NXT player","Tạm dừng / tua / EQ / DAC dùng trình phát NXT"),Nxt_text("Public extraction is unofficial and may need updates","Cách lấy nguồn không chính thức, có thể cần cập nhật"),Nxt_text("If playback fails, send NXT_SoundCloud_Log.txt","Nếu lỗi phát, gửi NXT_SoundCloud_Log.txt")};
 return Nxt_document(d,Nxt_text("Guide","Hướng dẫn"),rows,sizeof(rows)/sizeof(rows[0]));
}
static ModuleExitReason source_link(DisplayContext *d){
 if(!track_loaded||!*current.permalink){Toast_show(Nxt_text("No SoundCloud link available","Chưa có liên kết SoundCloud"),1800);return MODULE_EXIT_TO_MENU;}
 source_qr_count=0;JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd","source_link");json_object_set_string(o,"url",current.permalink);json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);send_value(v);json_value_free(v);int show=0;
 for(;;){ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit)return MODULE_EXIT_QUIT;if(!g.input_consumed&&!Nxt_lockInput()&&NxtPad_justPressed(BTN_B))return MODULE_EXIT_TO_MENU;
  NxtSkin_begin(s,Nxt_text("Open on SoundCloud","Mở bài trên SoundCloud"),NULL,false);int pad=s->w/24,top=NxtSkin_top(s)+pad,edge=s->w*35/100,room=NxtSkin_bottom(s,false)-top-pad*2;if(edge>room)edge=room;
  if(source_qr_count>0){int module=edge/(source_qr_count+8),size=module*(source_qr_count+8),qx=pad+(edge-size)/2,qy=top;Uint32 black=SDL_MapRGB(s->format,0,0,0),white=SDL_MapRGB(s->format,255,255,255);SDL_FillRect(s,&(SDL_Rect){qx,qy,size,size},white);for(int y=0;y<source_qr_count;y++)for(int x=0;x<source_qr_count;x++)if(source_qr[y][x]=='1')SDL_FillRect(s,&(SDL_Rect){qx+(x+4)*module,qy+(y+4)*module,module,module},black);}
  int x=pad+edge+pad,w=s->w-x-pad;NxtSkin_text(s,24,"SoundCloud",x,top,w,NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,22,current.title,x,top+pad*2,w,NxtSkin_color(NXT_INK));NxtSkin_text(s,20,current.album,x,top+pad*4,w,NxtSkin_color(NXT_MUTED));NxtSkin_text(s,20,Nxt_text("Scan to open the original track","Quét mã để mở bài gốc"),x,top+pad*6,w,NxtSkin_color(NXT_INK));NxtSkin_text(s,18,current.permalink,pad,top+edge+pad,s->w-pad*2,NxtSkin_color(NXT_MUTED));
  NxtSkin_footer(s,Nxt_text("B Back · music continues","B Quay lại · nhạc vẫn phát"),false);ModuleCommon_markSurfaceDrawn();int dirty=1;ModuleCommon_PWR_update(&dirty,&show);ModuleCommon_frameEnd(s);
 }
}
static ModuleExitReason playing_options(DisplayContext *d){int selected=0;const char *rows[]={Nxt_text("Playback options","Tùy chọn phát nhạc"),Nxt_text("Open original SoundCloud track · QR","Mở bài gốc trên SoundCloud · QR")};for(;;){NxtPick p=Nxt_pick(d,"SoundCloud",rows,2,&selected,Nxt_text("A Open • B Back","A Mở • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==1)return selected?source_link(d):Nxt_quickOptions(d);}}

ModuleExitReason NxtSoundCloud_queue(DisplayContext *d){
 int selected=queue_index;
 for(;;){char (*labels)[540]=calloc(queue_count+1,sizeof(*labels));const char **rows=calloc(queue_count+1,sizeof(*rows));if(!labels||!rows){free(labels);free(rows);return MODULE_EXIT_TO_MENU;}for(int i=0;i<queue_count;i++){snprintf(labels[i],sizeof(labels[i]),"%s%02d  %.255s\n%.255s",i==queue_index?"▶ ":"",i+1,queue[i].title,queue[i].artist);rows[i]=labels[i];}
  NxtPick p=Nxt_pickLive(d,Nxt_text("SoundCloud · Playing queue","SoundCloud · Danh sách đang phát"),rows,queue_count,&selected,Nxt_text("A Play • X Remove • L1/R1 Move • B Back","A Phát • X Bỏ • L1/R1 Đổi chỗ • B Về"));free(labels);free(rows);
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;
  if(p.action==1&&queue_count){select_track(selected);return MODULE_EXIT_TO_MENU;}
  if(p.action==3&&queue_count){if(selected==queue_index)Toast_show(Nxt_text("Current track is kept","Giữ lại bài đang phát"),1500);else{memmove(&queue[selected],&queue[selected+1],(queue_count-selected-1)*sizeof(Track));if(selected<queue_index)queue_index--;queue_count--;if(selected>=queue_count)selected=queue_count-1;}}
  int other=p.action==11?selected-1:p.action==12?selected+1:-1;
  if(other>=0&&other<queue_count&&selected>=0){Track temp=queue[other];queue[other]=queue[selected];queue[selected]=temp;if(queue_index==selected)queue_index=other;else if(queue_index==other)queue_index=selected;selected=other;}
 }
}
ModuleExitReason NxtSoundCloud_nowPlaying(DisplayContext *d){
 if(!track_loaded){Toast_show(Nxt_text("Choose a SoundCloud track first","Chọn một bài SoundCloud để phát trước"),1800);return MODULE_EXIT_TO_MENU;}
 int show=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;MusicPlaying_leave();MusicPlaying_invalidate();Spectrum_init();
 for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit||ModuleCommon_stopSignalled()){result=MODULE_EXIT_QUIT;break;}if(!track_loaded){Nxt_returnHome();break;}
  if(!g.input_consumed&&!Nxt_lockInput()){
   ModuleExitReason child_result=MODULE_EXIT_TO_MENU;
   if(NxtControl_aux()){}
   else if(NxtControl_pressed(NC_MENU)){result=NxtSession_exitPrompt(d);break;}
   else if(NxtControl_pressed(NC_PLAY)){if(!track_loaded&&queue_count>0)select_track(queue_index);else NxtSoundCloud_togglePause();}
   else if(NxtControl_pressed(NC_PREV))select_track(adjacent(-1));
   else if(NxtControl_pressed(NC_NEXT))select_track(adjacent(1));
   else if(NxtControl_pressed(NC_QUEUE))child_result=NxtSoundCloud_queue(d);
   else if(NxtControl_pressed(NC_LYRICS)){Settings_setBool(&SETTING_LYRICS_ENABLED,true);child_result=Nxt_lyricsScreen(d);}
   else if(NxtControl_pressed(NC_SPECTRUM))Spectrum_cycleNext();
   else if(NxtControl_pressed(NC_FAVORITE))NxtSoundCloud_favorite();
   else if(NxtControl_pressed(NC_EQ))child_result=Nxt_dspScreen(d);
   else if(NxtControl_pressed(NC_QUICK))child_result=playing_options(d);
   else if(NxtControl_repeated(NC_SEEK_BACK))NxtSoundCloud_seek(NxtSoundCloud_position()-10000);
   else if(NxtControl_repeated(NC_SEEK_FORWARD))NxtSoundCloud_seek(NxtSoundCloud_position()+10000);
   if(child_result==MODULE_EXIT_QUIT){result=child_result;break;}
  }
  int dirty=g.dirty||PAD_anyJustPressed()||PAD_anyJustReleased();ModuleCommon_PWR_update(&dirty,&show);
  bool redraw=MusicPlaying_frame(false);
  if(!PWR_getDisplayOff()&&!NxtSession_isHidden()&&(redraw||dirty)){Nxt_renderPlaying(s,show,queue_count?queue_index+1:0,queue_count,nxt.shuffle,nxt.repeat_mode);MusicPlaying_paintLayers();ModuleCommon_markSurfaceDrawn();}
  ModuleCommon_frameEnd(s);
 }
 MusicPlaying_leave();Spectrum_quit();return result;
}
static ModuleExitReason browse_impl(DisplayContext *,const char *);
static ModuleExitReason browse(DisplayContext *d,const char *kind){browser_depth++;ModuleExitReason r=browse_impl(d,kind);browser_depth--;return r;}
static ModuleExitReason browse_impl(DisplayContext *d,const char *kind){
 int selected=0,wanted=revision;copy(result_kind,sizeof(result_kind),kind);request(kind,0);
 for(;;){char title[340];char (*labels)[540]=calloc(result_count+2,sizeof(*labels));const char **rows=calloc(result_count+2,sizeof(*rows));if(!labels||!rows){free(labels);free(rows);return MODULE_EXIT_TO_MENU;}int count=0;snprintf(title,sizeof(title),"SoundCloud · %s",busy?Nxt_text("Loading…","Đang tải…"):status);
  if(result_count){for(int i=0;i<result_count;i++){snprintf(labels[count],sizeof(labels[count]),"%.255s\n%.255s",results[i].title,results[i].artist);rows[count]=labels[count];count++;}}
  if(count&&(has_next||busy))rows[count++]=busy?Nxt_text("Loading more…","Đang tải thêm…"):Nxt_text("Scroll for more tracks","Cuộn để tải thêm bài");
  if(!count){rows[0]=busy?Nxt_text("Please wait; B cancels","Đang tải; B quay lại"):status;count=1;}
  NxtPick p=Nxt_pickLive(d,title,rows,count,&selected,Nxt_text("A Play/Open • Scroll for more • B Back","A Phát/Mở • Cuộn tải thêm • B Về"));free(labels);free(rows);
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;
  if(!busy&&has_next&&(selected>=result_count-5||p.action==12))request(kind,next_offset);
  if(p.action==1&&result_count>0&&selected<result_count){
   if(!strcmp(result_kind,"search_playlists")){char saved_id[64];copy(saved_id,sizeof(saved_id),playlist_id);copy(playlist_id,sizeof(playlist_id),results[selected].id);ModuleExitReason rr=browse(d,"playlist_tracks");if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}copy(playlist_id,sizeof(playlist_id),saved_id);selected=0;wanted=revision;request(kind,0);}
   else{capture_queue(kind);select_track(selected);browser_depth--;ModuleExitReason rr=NxtSoundCloud_nowPlaying(d);browser_depth++;if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}
  }
 }
}
static ModuleExitReason search_menu(DisplayContext *d){
 int selected=0;const char *rows[]={Nxt_text("Tracks","Bài hát"),Nxt_text("Public playlists / albums","Playlist / album công khai")};const char *kinds[]={"search","search_playlists"};
 for(;;){NxtPick p=Nxt_pick(d,Nxt_text("Search SoundCloud","Tìm trên SoundCloud"),rows,2,&selected,Nxt_text("A Search • B Back","A Tìm • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
  char *q=Keyboard_open(Nxt_text("Search SoundCloud","Tìm trên SoundCloud"),200);if(!q)continue;copy(query,sizeof(query),q);free(q);if(!*query)continue;ModuleExitReason rr=browse(d,kinds[selected]);if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
 }
}
static ModuleExitReason discover(DisplayContext *d){
 const char *terms[]={"Vietnam pop","K-pop","J-pop","Mandopop","Thai pop","acoustic","jazz","piano","lofi","rock"};
 const char *rows[]={Nxt_text("Vietnamese music","Nhạc Việt"),"K-pop","J-pop",Nxt_text("Chinese pop","Nhạc Hoa"),Nxt_text("Thai pop","Nhạc Thái"),"Acoustic","Jazz","Piano","Lo-fi","Rock"};int selected=0;
 for(;;){NxtPick p=Nxt_pick(d,Nxt_text("SoundCloud · themed searches","SoundCloud · tìm theo chủ đề"),rows,10,&selected,Nxt_text("A Browse • B Back","A Xem • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==1){copy(query,sizeof(query),terms[selected]);ModuleExitReason rr=browse(d,"search");if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}}
}
static ModuleExitReason configuration(DisplayContext *d){
 NxtSoundCloud_quit();start();return MODULE_EXIT_TO_MENU;
}
static ModuleExitReason soundcloud_menu_run(DisplayContext *d){
 int selected=0;paths();start();
 for(;;){char heading[340];snprintf(heading,sizeof(heading),"SoundCloud · %s",status);
  const char *online[]={Nxt_text("Search tracks / playlists","Tìm bài / playlist"),Nxt_text("Suggested online tracks / playlists","Bài hát / playlist online gợi ý"),Nxt_text("Saved tracks on this device","Bài đã lưu trên máy"),Nxt_text("Open SoundCloud link","Mở liên kết SoundCloud"),Nxt_text("Now playing","Đang phát"),Nxt_text("Restart SoundCloud","Kết nối lại SoundCloud"),Nxt_text("Setup guide","Hướng dẫn thiết lập")};
  const char *offline[]={Nxt_text("Restart SoundCloud","Kết nối lại SoundCloud"),Nxt_text("Setup guide","Hướng dẫn thiết lập")};
  NxtPick p=Nxt_pickLive(d,heading,connected?online:offline,connected?7:2,&selected,Nxt_text("A Open • B Back","A Mở • B Quay lại"));
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
  ModuleExitReason rr=MODULE_EXIT_TO_MENU;
  if(!connected)rr=selected==0?configuration(d):setup_help(d);
  else if(selected==0)rr=search_menu(d);
  else if(selected==1)rr=NxtSoundCloud_recommendations(d,false);
  else if(selected==2)rr=browse(d,"saved_tracks");
  else if(selected==3){char *url=Keyboard_openLatin("https://soundcloud.com/…",500);if(url){copy(query,sizeof(query),url);free(url);if(*query)rr=browse(d,"resolve");}}
  else if(selected==4)rr=NxtSoundCloud_nowPlaying(d);
  else if(selected==5)rr=configuration(d);
  else rr=setup_help(d);
  if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
 }
}
ModuleExitReason NxtSoundCloud_run(DisplayContext *d){
 NxtSkinSource previous=NxtSkin_setSource(NXT_SOURCE_SOUNDCLOUD);
 ModuleExitReason result=soundcloud_menu_run(d);
 NxtSkin_setSource(previous);return result;
}

ModuleExitReason NxtSoundCloud_playSaved(DisplayContext *d,const char *uri,const char *title,const char *artist){
 if(!uri||!*uri||!start()){NxtLog_step("SoundCloud","history-start",false,"Cannot start source");return MODULE_EXIT_TO_MENU;}
 uint32_t began=SDL_GetTicks();int selected=0;const char *waiting[]={Nxt_text("Connecting to source…","Đang kết nối nguồn nhạc…")};
 while(!connected&&SDL_GetTicks()-began<30000){NxtSoundCloud_tick();NxtPick p=Nxt_pickLive(d,"SoundCloud",waiting,1,&selected,Nxt_text("B Cancel","B Hủy"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}
 if(!connected){Toast_show(Nxt_text("Open source and reconnect your account","Mở nguồn nhạc và kết nối lại tài khoản"),3000);NxtLog_step("SoundCloud","history-login",false,"Connection timed out");return MODULE_EXIT_TO_MENU;}
 Track track={0};copy(track.uri,sizeof(track.uri),uri);copy(track.title,sizeof(track.title),title);copy(track.artist,sizeof(track.artist),artist);queue_count=0;queue_more=queue_request=queue_waiting=false;queue_retry=0;
 if(!append_track(&queue,&queue_count,&queue_capacity,&track))return MODULE_EXIT_TO_MENU;select_track(0);return NxtSoundCloud_nowPlaying(d);
}

ModuleExitReason NxtSoundCloud_recommendations(DisplayContext *d,bool playlists){
 if(!start())return MODULE_EXIT_TO_MENU;
 if(!connected){uint32_t began=SDL_GetTicks();int select=0;const char *wait[]={Nxt_text("Connecting to music source…","Đang kết nối nguồn nhạc…")};while(!connected&&busy&&SDL_GetTicks()-began<30000){NxtPick p=Nxt_pickLive(d,"SoundCloud",wait,1,&select,Nxt_text("B Cancel","B Hủy"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}if(!connected)return NxtSoundCloud_run(d);}
 if(playlists){NxtCustom_query(query,sizeof(query),1);return browse(d,"search_playlists");}
 int selected=0;for(;;){char preference[200];snprintf(preference,sizeof(preference),"%s · %s",NxtCustom_genre(nxt.genre),NxtCustom_mood(nxt.mood));const char *rows[]={preference,Nxt_text("By your genre","Theo thể loại của bạn"),Nxt_text("By your mood","Theo tâm trạng"),Nxt_text("Acoustic versions","Phiên bản acoustic"),Nxt_text("Live performances","Bản biểu diễn live"),Nxt_text("Playable playlists","Playlist nghe được"),Nxt_text("More genres","Thêm thể loại")};NxtPick p=Nxt_pick(d,Nxt_text("Suggested online music","Nhạc online gợi ý"),rows,7,&selected,Nxt_text("A Browse · B Back","A Xem · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;ModuleExitReason r;if(selected==6)r=discover(d);else{NxtCustom_query(query,sizeof(query),selected);r=browse(d,selected==5?"search_playlists":"search");}if(r==MODULE_EXIT_QUIT)return r;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}
}
