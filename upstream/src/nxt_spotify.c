#include "nxt_startup.h"
#include "nxt_audio_mode.h"
#include "nxt_custom.h"
#include "nxt_log.h"
/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_spotify.h"
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
#include <ctype.h>
#include "nxt_spotify_pcm.inc"
#define SP_MAX 24
static pid_t child=-1;
static int channel=-1;
static char incoming[65536];static size_t used;
static bool connected,playing,track_loaded,busy,has_next,cover_dirty,stop_requested;
static int request_serial,expected_serial;
static int revision,offset,position_ms,duration_ms,queue_count,queue_index,last_sink=-1,last_dac=-1;
static char status[256],creds[1024],playback_creds[1024],diag_path[1200],cover_path[1024],query[256],playlist_id[64],result_kind[32];
static uint32_t login_retry_at;
static bool pairing,pair_done,pair_failed,pair_finishing;
static char pair_code[32],pair_qr[57][58];static int pair_qr_count;
typedef struct {char title[256],artist[256],album[256],uri[128],id[64],cover[512];int duration;} Track;
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
static uint32_t position_since;
static uint32_t loading_since;static bool pcm_seen;
static bool backend_ended;
static bool auth_required,auth_upgrade,favorite,favorite_pending,favorite_change;
static char lyrics_path[1024],lyrics_status[160],account_name[256];
static void copy(char *out,size_t n,const char *in){if(!in)in="";size_t k=strlen(in);if(k>=n){k=n-1;while(k&&((unsigned char)in[k]&0xc0)==0x80)k--;}memcpy(out,in,k);out[k]=0;for(size_t i=0;i<k;i++)if(out[i]=='\n'||out[i]=='\r')out[i]=' ';}
static void paths(void){if(*creds)return;char dir[1024];userdata_snpath("spotify",dir,sizeof(dir));mkdir(dir,0700);userdata_snpath("spotify/credentials.json",creds,sizeof(creds));userdata_snpath("spotify/playback.json",playback_creds,sizeof(playback_creds));userdata_snpath("spotify/cover.jpg",cover_path,sizeof(cover_path));userdata_snpath("spotify/lyrics.txt",lyrics_path,sizeof(lyrics_path));if(getcwd(dir,sizeof(dir)))snprintf(diag_path,sizeof(diag_path),"%s/NXT_Spotify_Log.txt",dir);}
static void diagnostic(const char *message){if(!*diag_path)return;int fd=open(diag_path,O_WRONLY|O_CREAT|O_APPEND,0600);if(fd>=0){dprintf(fd,"%s\n",message);close(fd);}}
static void pair_clear(void){pairing=false;pair_finishing=false;pair_qr_count=0;*pair_code=0;}
static bool pair_read_qr(JSON_Array *a){
 pair_qr_count=0;size_t n=json_array_get_count(a);if(!n||n>57)return false;
 for(size_t y=0;y<n;y++){const char *row=json_array_get_string(a,y);if(!row||strlen(row)!=n)return false;for(size_t x=0;x<n;x++)if(row[x]!='0'&&row[x]!='1')return false;}
 for(size_t y=0;y<n;y++)copy(pair_qr[y],sizeof(pair_qr[y]),json_array_get_string(a,y));pair_qr_count=(int)n;return true;
}
static bool send_value(JSON_Value *v){char *s=json_serialize_to_string(v);if(!s)return false;size_t n=strlen(s);char *line=malloc(n+2);if(!line){json_free_serialized_string(s);return false;}memcpy(line,s,n);line[n]='\n';ssize_t got=channel>=0?send(channel,line,n+1,MSG_NOSIGNAL|MSG_DONTWAIT):-1;free(line);json_free_serialized_string(s);return got==(ssize_t)n+1;}
static void command(const char *name){JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",name);
 if(!strcmp(name,"login"))json_object_set_string(o,"source",nxt.spotify_auth_source==1?"pc":nxt.spotify_auth_source==2?"qr":"auto");
 send_value(v);json_value_free(v);}
void NxtSpotify_quit(void){
 bool had_track=track_loaded;
 if(Background_getActive()==BG_SPOTIFY)Background_setActive(BG_NONE);
 pcm_close();free(results);free(queue);results=queue=NULL;result_count=queue_count=0;result_capacity=queue_capacity=0;queue_more=queue_request=queue_waiting=false;
 if(channel>=0){command("quit");close(channel);channel=-1;}
 if(child>0){int result=0;for(int i=0;i<10;i++){pid_t p=waitpid(child,&result,WNOHANG);if(p==child||p<0){child=-1;break;}usleep(10000);}if(child>0){kill(child,SIGTERM);for(int i=0;i<10;i++){if(waitpid(child,&result,WNOHANG)!=0){child=-1;break;}usleep(10000);}}if(child>0){kill(child,SIGKILL);waitpid(child,NULL,0);child=-1;}}
 login_retry_at=0;connected=playing=track_loaded=busy=false;pair_clear();pair_done=pair_failed=false;used=0;last_sink=last_dac=-1;
 if(cover){SDL_FreeSurface(cover);cover=NULL;}cover_dirty=false;
 favorite_pending=false;if(had_track)Lyrics_clear();
 if(had_track)ModuleCommon_setAutosleepDisabled(false);
}
bool NxtSpotify_active(void){return track_loaded;}
bool NxtSpotify_isPlaying(void){return playing;}
bool NxtSpotify_hasMetadata(void){return child>0&&*current.uri&&*track_info.title;}
const TrackInfo *NxtSpotify_info(void){return &track_info;}
const char *NxtSpotify_uri(void){return current.uri;}
SDL_Surface *NxtSpotify_art(void){return cover;}
unsigned NxtSpotify_generation(void){return track_generation;}
const char *NxtSpotify_lyricsStatus(void){return *lyrics_status?lyrics_status:Nxt_text("No lyrics available for this track.","Bài này chưa có lời khả dụng.");}
bool NxtSpotify_isFavorite(void){return favorite;}
int NxtSpotify_position(void){
 pthread_mutex_lock(&pcm_lock);double elapsed=pcm_position;pthread_mutex_unlock(&pcm_lock);
 int64_t pos=position_ms+(int64_t)elapsed;if(pos<0)pos=0;if(duration_ms>0&&pos>duration_ms)pos=duration_ms;return (int)pos;
}
void NxtSpotify_togglePause(void){
 if(!track_loaded)return;position_since=SDL_GetTicks();
 bool resume=!playing;if(resume)Player_resumeAudio();pcm_enable(resume);command(resume?"resume":"pause");playing=resume;
}
void NxtSpotify_seek(int ms){
 if(!track_loaded)return;if(ms<0)ms=0;if(duration_ms>0&&ms>duration_ms)ms=duration_ms;
 bool resume=playing;backend_ended=false;pcm_begin();pcm_enable(resume);
 position_ms=ms;position_since=SDL_GetTicks();pcm_seen=false;loading_since=SDL_GetTicks();
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);
 json_object_set_string(o,"cmd","play");json_object_set_string(o,"uri",current.uri);
 json_object_set_number(o,"stream_id",pcm_epoch);json_object_set_number(o,"position_ms",ms);
 json_object_set_boolean(o,"autoplay",resume);send_value(v);json_value_free(v);
}
static void favorite_request(bool change){
 if(!connected||!track_loaded||(change&&favorite_pending))return;
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",change?"favorite":"check_favorite");json_object_set_string(o,"uri",current.uri);json_object_set_boolean(o,"saved",!favorite);if(send_value(v)){favorite_pending=change;favorite_change=change;}json_value_free(v);
}
void NxtSpotify_favorite(void){favorite_request(true);}
static bool start(void){
 if(child>0)return true;paths();LOG_info("Spotify frontend build: NXT-v1.10.1-Login5-Library\n");
 if(*diag_path){struct stat st;if(!stat(diag_path,&st)&&st.st_size>262144){char previous[1300];snprintf(previous,sizeof(previous),"%s.previous",diag_path);rename(diag_path,previous);}int fd=open(diag_path,O_WRONLY|O_CREAT|O_APPEND,0600);if(fd>=0){fchmod(fd,0600);close(fd);}setenv("NXT_SPOTIFY_DIAG",diag_path,1);}diagnostic("Frontend build: NXT-v1.10.1-Login5-Library");
 int audio[2];if(socketpair(AF_UNIX,SOCK_SEQPACKET,0,audio)){copy(status,sizeof(status),"PCM IPC unavailable");return false;}
 int sv[2];if(socketpair(AF_UNIX,SOCK_STREAM,0,sv)){close(audio[0]);close(audio[1]);copy(status,sizeof(status),"IPC unavailable");return false;}
 fcntl(audio[0],F_SETFD,FD_CLOEXEC);fcntl(audio[1],F_SETFD,FD_CLOEXEC);
 int size=262144;setsockopt(audio[1],SOL_SOCKET,SO_SNDBUF,&size,sizeof(size));
 if(!pcm_open(audio[0])){close(audio[1]);close(sv[0]);close(sv[1]);pcm_close();return false;}
 setenv("NXT_SPOTIFY_PCM_FD","3",1);
 fcntl(sv[0],F_SETFD,FD_CLOEXEC);fcntl(sv[1],F_SETFD,FD_CLOEXEC);setenv("NXT_SPOTIFY_CREDENTIALS",creds,1);setenv("NXT_SPOTIFY_PLAYBACK_CREDS",playback_creds,1);setenv("NXT_SPOTIFY_COVER",cover_path,1);setenv("NXT_SPOTIFY_LYRICS",lyrics_path,1);pid_t parent=getpid();pid_t pid=fork();
 if(pid==0){prctl(PR_SET_PDEATHSIG,SIGTERM);if(getppid()!=parent)_exit(125);dup2(sv[1],STDIN_FILENO);dup2(sv[1],STDOUT_FILENO);dup2(audio[1],3);fcntl(3,F_SETFD,0);for(int fd=4;fd<4096;fd++)close(fd);execl("./bin/tg5040/nxt-spotify-backend","nxt-spotify-backend",(char*)NULL);_exit(127);}
 close(audio[1]);close(sv[1]);if(pid<0){close(sv[0]);pcm_close();return false;}child=pid;channel=sv[0];fcntl(channel,F_SETFL,fcntl(channel,F_GETFL)|O_NONBLOCK);used=0;connected=false;last_sink=GetAudioSink();last_dac=NxtHardware_hasDAC();busy=true;copy(status,sizeof(status),Nxt_text("Connecting to Spotify…","Đang kết nối Spotify…"));command("login");return true;
}
static bool import_credentials(void){
 paths();struct stat st;
 if(stat("spotify-credentials.json",&st)||st.st_size<2||st.st_size>65536){copy(status,sizeof(status),Nxt_text("Missing / oversized PC login file","Thiếu file đăng nhập PC / file quá lớn"));return false;}
 JSON_Value *v=json_parse_file("spotify-credentials.json");JSON_Object *o=json_value_get_object(v);
 const char *access=o?json_object_get_string(o,"access_token"):NULL,*refresh=o?json_object_get_string(o,"refresh_token"):NULL,*id=o?json_object_get_string(o,"client_id"):NULL;
 bool valid=access&&*access&&refresh&&*refresh&&id&&strlen(id)==32;
 if(valid)for(int i=0;i<32;i++)if(!isxdigit((unsigned char)id[i])){valid=false;break;}
 if(!valid){json_value_free(v);copy(status,sizeof(status),Nxt_text("Invalid spotify-credentials.json","Sai file spotify-credentials.json"));return false;}
 // Import a private copy. Keep the user's original and the last good login if
 // serialization / writing fails. Tokens never go into the diagnostic log.
 char temp[1060];snprintf(temp,sizeof(temp),"%s.tmp.XXXXXX",creds);char *bytes=json_serialize_to_string(v);bool ok=false;
 if(bytes){int fd=mkstemp(temp);if(fd>=0){size_t n=strlen(bytes),written=0;ok=!fchmod(fd,0600);
  while(ok&&written<n){ssize_t count=write(fd,bytes+written,n-written);if(count<0&&errno==EINTR)continue;if(count<=0){ok=false;break;}written+=(size_t)count;}
  if(ok)ok=!fsync(fd);if(close(fd))ok=false;if(ok){NxtSpotify_quit();ok=!rename(temp,creds);}if(!ok)unlink(temp);
 }json_free_serialized_string(bytes);}
 if(ok){nxt.spotify_auth_source=1;Nxt_save();diagnostic("Frontend: explicit PC login selected; original retained");}
 json_value_free(v);copy(status,sizeof(status),ok?Nxt_text("PC login copied; original kept","Đã chép đăng nhập PC; giữ file gốc"):Nxt_text("Cannot save login; original kept","Không lưu được; vẫn giữ file gốc"));return ok;
}
static bool append_track(Track **list,int *count,size_t *capacity,const Track *t){for(int i=0;i<*count;i++)if((*t->uri&&!strcmp((*list)[i].uri,t->uri))||(!*t->uri&&*t->id&&!strcmp((*list)[i].id,t->id)))return true;if((size_t)*count==*capacity){size_t cap=*capacity?*capacity*2:32;Track *n=realloc(*list,cap*sizeof(**list));if(!n)return false;*list=n;*capacity=cap;}(*list)[(*count)++]=*t;return true;}
static void queue_fetch(void){if(!queue_more||busy||browser_depth||(int32_t)(SDL_GetTicks()-queue_retry)<0)return;JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",queue_kind);json_object_set_string(o,"query",queue_query);json_object_set_string(o,"id",queue_id);json_object_set_number(o,"offset",queue_offset);json_object_set_number(o,"request_id",++request_serial);expected_serial=request_serial;if(send_value(v)){busy=true;queue_request=true;}else queue_retry=SDL_GetTicks()+3000;json_value_free(v);}
static void capture_queue(const char *kind){queue_count=0;for(int i=0;i<result_count;i++)if(!append_track(&queue,&queue_count,&queue_capacity,&results[i]))break;copy(queue_kind,sizeof(queue_kind),kind);copy(queue_query,sizeof(queue_query),query);copy(queue_id,sizeof(queue_id),playlist_id);queue_more=has_next;queue_offset=next_offset;queue_waiting=false;queue_retry=0;}
static void request(const char *kind,int page){
 if(!connected)return;queue_request=false;if(page==0){result_count=0;has_next=false;next_offset=0;}JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd",kind);json_object_set_number(o,"request_id",++request_serial);expected_serial=request_serial;json_object_set_number(o,"offset",page);json_object_set_string(o,"query",query);json_object_set_string(o,"id",playlist_id);if(send_value(v)){busy=true;copy(status,sizeof(status),Nxt_text("Loading…","Đang tải…"));}json_value_free(v);
}
static void select_track(int index){
 if(index<0||index>=queue_count||!connected)return;
 NxtAudio_streaming();
 if(Background_getActive()!=BG_SPOTIFY)Background_stopAll();
 if(PlayerModule_isActive())Resume_updatePosition(Player_getPosition());PlayerModule_markStopped();Player_stop();Player_resumeAudio();
 backend_ended=false;pcm_begin();pthread_mutex_lock(&pcm_lock);pcm_packets=0;pthread_mutex_unlock(&pcm_lock);pcm_seen=false;loading_since=SDL_GetTicks();LOG_info("Spotify play requested: %s, PCM epoch %u\n",queue[index].uri,pcm_epoch);diagnostic("Frontend: track requested");
 stop_requested=false;queue_index=index;current=queue[index];position_ms=0;position_since=SDL_GetTicks();duration_ms=current.duration;track_loaded=true;playing=false;Background_setActive(BG_SPOTIFY);
 memset(&track_info,0,sizeof(track_info));copy(track_info.title,sizeof(track_info.title),current.title);copy(track_info.artist,sizeof(track_info.artist),current.artist);copy(track_info.album,sizeof(track_info.album),current.album);copy(track_info.codec,sizeof(track_info.codec),"Spotify");track_info.sample_rate=44100;track_info.channels=2;track_info.duration_ms=duration_ms;track_info.track_number=index+1;track_generation++;
 favorite=favorite_pending=favorite_change=false;Lyrics_clear();unlink(lyrics_path);copy(lyrics_status,sizeof(lyrics_status),Nxt_text("Loading lyrics…","Đang tải lời bài hát…"));
 if(cover){SDL_FreeSurface(cover);cover=NULL;}unlink(cover_path);
 JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"cmd","play");json_object_set_string(o,"uri",current.uri);json_object_set_number(o,"stream_id",pcm_epoch);send_value(v);json_value_free(v);
 copy(status,sizeof(status),Nxt_text("Loading track…","Đang tải bài…"));
 if(*current.cover){v=json_value_init_object();o=json_value_get_object(v);json_object_set_string(o,"cmd","cover");json_object_set_string(o,"url",current.cover);json_object_set_string(o,"uri",current.uri);send_value(v);json_value_free(v);}
 v=json_value_init_object();o=json_value_get_object(v);json_object_set_string(o,"cmd","lyrics");json_object_set_string(o,"uri",current.uri);json_object_set_string(o,"title",current.title);json_object_set_string(o,"artist",current.artist);json_object_set_string(o,"album",current.album);json_object_set_number(o,"duration_ms",duration_ms);send_value(v);json_value_free(v);favorite_request(false);
 ModuleCommon_setAutosleepDisabled(true);
}
static int adjacent(int direction){
 if(!queue_count)return -1;if(nxt.shuffle&&queue_count>1){int next=rand()%(queue_count-1);return next>=queue_index?next+1:next;}
 int next=queue_index+direction;if(direction>0&&next>=queue_count&&queue_more)return -1;if(next<0||next>=queue_count)return nxt.repeat_mode==2?(next+queue_count)%queue_count:-1;return next;
}
static void stop_track(void){pcm_enable(false);playing=track_loaded=false;favorite_pending=false;if(Background_getActive()==BG_SPOTIFY)Background_setActive(BG_NONE);ModuleCommon_setAutosleepDisabled(false);}
void NxtSpotify_skip(int direction){if(direction==1||direction==-1)select_track(adjacent(direction));}
int NxtSpotify_queueCount(void){return queue_count;}
int NxtSpotify_queueIndex(void){return queue_index;}
static const char *friendly(const char *code){
 if(!strcmp(code,"empty_search_query"))return Nxt_text("Enter an artist, album or track name","Nhập tên nghệ sĩ, album hoặc bài hát");
 if(!strcmp(code,"session_token_denied"))return Nxt_text("Spotify denied library access for this session","Spotify chưa cấp quyền thư viện cho phiên này");
 if(!strcmp(code,"session_token_unavailable"))return Nxt_text("Cannot refresh Spotify session; reconnect the account","Không làm mới được phiên Spotify; kết nối lại tài khoản");
 if(!strcmp(code,"api_scope_missing"))return Nxt_text("Spotify API denied the required permission; reconnect or use PC login","Spotify API từ chối quyền cần thiết; kết nối lại hoặc dùng đăng nhập PC");
 if(!strcmp(code,"session_token_timeout"))return Nxt_text("Spotify library timed out; reconnect","Kết nối thư viện Spotify quá lâu; kết nối lại");
 if(!strcmp(code,"pc_credentials_missing"))return Nxt_text("PC login missing; import the file again","Thiếu đăng nhập PC; nhập lại file");
 if(!strcmp(code,"pair_required"))return Nxt_text("Log in by QR to begin","Đăng nhập bằng QR để bắt đầu");
 if(!strcmp(code,"pair_setup_failed")||!strcmp(code,"pair_request_failed"))return Nxt_text("Cannot request pairing code; check Wi-Fi / time","Không lấy được mã ghép; kiểm tra Wi-Fi / giờ máy");
 if(!strcmp(code,"pair_approval_failed"))return Nxt_text("Pairing declined / expired; request a new code","Ghép bị từ chối / hết hạn; lấy mã mới");
 if(!strcmp(code,"pair_save_failed"))return Nxt_text("Cannot save pairing; check free storage","Không lưu được ghép; kiểm tra thẻ nhớ");
 if(!strncmp(code,"playback_refresh_",17))return nxt.spotify_auth_source==1?Nxt_text("PC login expired; sign in on PC and import again","Đăng nhập PC hết hạn; đăng nhập PC rồi nhập lại file"):Nxt_text("Playback login expired; pair again by QR / code","Đăng nhập bộ phát hết hạn; ghép lại QR / mã");
 if(!strcmp(code,"playback_auth_rejected"))return Nxt_text("Spotify rejected playback authentication; send log","Spotify từ chối xác thực phát nhạc; gửi log");
 if(!strncmp(code,"playback_rate_limited:",22))return Nxt_text("Spotify login rate limited; reconnect later","Spotify giới hạn đăng nhập; kết nối lại sau");
 if(!strcmp(code,"playback_network_timeout"))return Nxt_text("Playback connection timed out; reconnect","Kết nối bộ phát hết thời gian; kết nối lại");
 if(!strcmp(code,"account_mismatch"))return Nxt_text("PC and QR logins belong to different accounts","Đăng nhập PC và QR khác tài khoản");
 if(!strcmp(code,"playback_auth_timeout"))return Nxt_text("Playback authentication timed out; retry connection","Xác thực phát nhạc hết thời gian; kết nối lại");
 if(!strcmp(code,"playlist_access_restricted"))return Nxt_text("Playlist unavailable; try Suggested tracks / NXT Mix","Playlist chưa mở được; chọn Bài gợi ý / NXT Mix");
 if(!strcmp(code,"track_unavailable"))return Nxt_text("Cannot load track; check log / Premium / network","Không tải được bài; kiểm tra log / Premium / mạng");
 if(!strcmp(code,"pcm_channel_missing")||!strcmp(code,"pcm_channel_invalid"))return Nxt_text("PCM channel failed; reinstall the full app","Lỗi kênh âm thanh; chép lại đầy đủ app");
 if(!strcmp(code,"credentials_missing"))return Nxt_text("Log in by QR to begin","Đăng nhập bằng QR để bắt đầu");
 if(!strncmp(code,"api_http_403",12))return Nxt_text("Spotify denied access; check Development users","Spotify từ chối; kiểm tra User Management");
 if(!strcmp(code,"api_http_401")||!strncmp(code,"refresh_",8))return Nxt_text("Login expired; scan QR again","Đăng nhập hết hạn; quét QR lại");
 if(!strncmp(code,"rate_limited:",13))return Nxt_text("Library rate limited; use a track link or retry later","Thư viện đang giới hạn; phát link hoặc thử lại sau");
 if(!strcmp(code,"api_http_429"))return Nxt_text("Rate limited; retry later","Giới hạn truy cập; thử lại sau");
 if(!strcmp(code,"authentication_failed"))return Nxt_text("Player authentication failed; check pairing/network","Xác thực bộ phát lỗi; kiểm tra ghép / mạng");
 if(!strcmp(code,"network_error"))return Nxt_text("Network error; check Wi-Fi/time","Lỗi mạng; kiểm tra Wi-Fi / giờ máy");
 return code;
}
static void event(const char *line){
 JSON_Value *v=json_parse_string(line);JSON_Object *o=json_value_get_object(v);const char *e=o?json_object_get_string(o,"event"):NULL;
 if(!e){json_value_free(v);return;}
 if(!strcmp(e,"ready")||!strcmp(e,"logged_in")||!strcmp(e,"error")||!strcmp(e,"paired")||!strcmp(e,"login_progress"))NxtLog_step("Spotify",e,strcmp(e,"error")!=0,"Provider event received");
 const char *uri=json_object_get_string(o,"uri");if(uri&&*current.uri&&strcmp(uri,current.uri)){json_value_free(v);return;}
 if(json_object_has_value(o,"request_id")&&(int)json_object_get_number(o,"request_id")!=expected_serial){json_value_free(v);return;}
 if(!strcmp(e,"ready")){const char *build=json_object_get_string(o,"build");LOG_info("Spotify helper build: %s\n",build?build:"unknown");}
 else if(!strcmp(e,"retry_wait")){int seconds=(int)json_object_get_number(o,"seconds");snprintf(status,sizeof(status),"%s %d s",Nxt_text("Spotify cooldown · retry in","Spotify yêu cầu chờ · thử lại sau"),seconds);}
 else if(!strcmp(e,"login_progress")){const char *stage=json_object_get_string(o,"stage");if(!stage)stage="";
 copy(status,sizeof(status),!strcmp(stage,"ap")?Nxt_text("Verifying playback account…","Đang xác thực tài khoản phát…"):!strcmp(stage,"login5")?Nxt_text("Verifying playback permission…","Đang xác thực quyền phát…"):!strcmp(stage,"library")?Nxt_text("Playback verified; checking library…","Đã xác thực phát; kiểm tra thư viện…"):Nxt_text("Refreshing saved login…","Đang kiểm tra đăng nhập đã lưu…"));}
 else if(!strcmp(e,"logged_in")){
 login_retry_at=0;busy=false;auth_upgrade=false;
 if(json_object_has_value(o,"playback_ready")&&json_object_get_boolean(o,"playback_ready")!=1){connected=false;copy(status,sizeof(status),friendly("authentication_failed"));json_value_free(v);return;}
 connected=true;auth_required=false;copy(account_name,sizeof(account_name),json_object_get_string(o,"display_name"));
 const char *warning=json_object_get_string(o,"library_warning"),*import_warning=json_object_get_string(o,"import_warning");
 copy(status,sizeof(status),warning&&*warning?friendly(warning):import_warning&&*import_warning?friendly(import_warning):Nxt_text("Ready to play","Sẵn sàng nghe nhạc"));
 if(pairing&&pair_finishing){pair_done=true;pair_clear();}
 diagnostic(warning&&*warning?"Frontend: playback connected; library unavailable":"Frontend: playback and library connected");
 if(warning&&*warning)Toast_show(status,5000);
 }
 else if(!strcmp(e,"pair_code")){if(pairing){copy(pair_code,sizeof(pair_code),json_object_get_string(o,"code"));pair_read_qr(json_object_get_array(o,"qr"));copy(status,sizeof(status),Nxt_text("Approve pairing on Spotify","Xác nhận ghép trên Spotify"));diagnostic("Frontend: pairing code displayed");}}
 else if(!strcmp(e,"paired")){if(pairing){nxt.spotify_auth_source=2;Nxt_save();pair_finishing=true;pair_qr_count=0;*pair_code=0;busy=true;copy(status,sizeof(status),Nxt_text("Finishing login…","Đang hoàn tất đăng nhập…"));command("login");diagnostic("Frontend: player paired; loading account");}}
 else if(!strcmp(e,"error")){busy=false;if(queue_request){queue_request=false;queue_retry=SDL_GetTicks()+15000;}const char *reason=json_object_get_string(o,"reason");if(!reason)reason="unknown";
 const char *operation=json_object_get_string(o,"operation");if(!strcmp(reason,"track_unavailable")||(operation&&!strcmp(operation,"play"))||!strcmp(reason,"pair_required")||!strcmp(reason,"authentication_failed")||!strncmp(reason,"playback_",9))stop_track();
 if(operation&&!strcmp(operation,"login")){connected=false;login_retry_at=0;
 auth_required=nxt.spotify_auth_source!=1&&(!strcmp(reason,"pair_required")||!strcmp(reason,"credentials_missing")||!strcmp(reason,"playback_refresh_http_400")||!strcmp(reason,"playback_refresh_http_401"));
 if(pairing&&pair_finishing){pair_failed=true;pair_clear();}}
 if(nxt.spotify_auth_source!=1&&(!strcmp(reason,"pair_required")||!strcmp(reason,"playback_refresh_http_400")||!strcmp(reason,"playback_refresh_http_401")))auth_required=true;
 if(!strncmp(reason,"pair_",5)&&strcmp(reason,"pair_required")){if(!pairing){json_value_free(v);return;}connected=false;pair_failed=true;pair_clear();}
 LOG_info("Spotify error: %s\n",reason);char note[200];snprintf(note,sizeof(note),"Frontend error: %.160s",reason);diagnostic(note);copy(status,sizeof(status),friendly(reason));if(operation&&!strcmp(operation,"play"))Toast_show(status,4000);}
 else if(!strcmp(e,"results")){JSON_Array *a=json_object_get_array(o,"items");bool target=queue_request;int page=(int)json_object_get_number(o,"offset");if(!target&&page==0)result_count=0;int before=target?queue_count:result_count;bool memory_ok=true;
 for(size_t i=0;i<json_array_get_count(a);i++){JSON_Object *x=json_array_get_object(a,i);if(!x)continue;Track value={0};Track *t=&value;
#define FIELD(name) copy(t->name,sizeof(t->name),json_object_get_string(x,#name))
 FIELD(title);FIELD(artist);FIELD(album);FIELD(uri);FIELD(id);FIELD(cover);
#undef FIELD
 t->duration=(int)json_object_get_number(x,"duration_ms");if(!append_track(target?&queue:&results,target?&queue_count:&result_count,target?&queue_capacity:&result_capacity,t)){memory_ok=false;break;}}
 int following=json_object_has_value(o,"next_offset")?(int)json_object_get_number(o,"next_offset"):page+10;bool more=memory_ok&&json_object_get_boolean(o,"has_next")==1&&following>page;busy=false;queue_request=false;
 if(target){queue_offset=following;queue_more=more;queue_retry=0;if(queue_waiting){if(queue_count>before){queue_waiting=false;select_track(before);}else if(!more){queue_waiting=false;stop_track();}}}
 else{copy(result_kind,sizeof(result_kind),json_object_get_string(o,"kind"));offset=page;next_offset=following;has_next=more;revision++;copy(status,sizeof(status),result_count?Nxt_text("A Play / open","A Phát / mở"):Nxt_text("No playable items here · try another genre or page","Chưa có mục nghe được · thử thể loại hoặc trang khác"));}
 }else if(!strcmp(e,"playing")){pthread_mutex_lock(&pcm_lock);pcm_enabled=true;pthread_mutex_unlock(&pcm_lock);playing=true;position_since=SDL_GetTicks();copy(status,sizeof(status),Nxt_text("Playing","Đang phát"));int restore=NxtStartup_takeSeek("Spotify",current.uri);if(restore>0)NxtSpotify_seek(restore);}
 else if(!strcmp(e,"paused")){pcm_enable(false);playing=false;position_since=SDL_GetTicks();copy(status,sizeof(status),Nxt_text("Paused","Tạm dừng"));}
 else if(!strcmp(e,"position")){position_since=SDL_GetTicks();}
 else if(!strcmp(e,"ended")){backend_ended=true;NxtPCMQueue_finish(pcm_queue);}

 else if(!strcmp(e,"metadata")&&track_loaded){
 copy(current.title,sizeof(current.title),json_object_get_string(o,"title"));copy(current.artist,sizeof(current.artist),json_object_get_string(o,"artist"));copy(current.album,sizeof(current.album),json_object_get_string(o,"album"));
 int length=(int)json_object_get_number(o,"duration_ms");if(length>0)duration_ms=current.duration=length;
 copy(track_info.title,sizeof(track_info.title),current.title);copy(track_info.artist,sizeof(track_info.artist),current.artist);copy(track_info.album,sizeof(track_info.album),current.album);track_info.duration_ms=duration_ms;
 track_generation++;
 const char *image=json_object_get_string(o,"cover");if(image&&*image&&strcmp(image,current.cover)){
 copy(current.cover,sizeof(current.cover),image);JSON_Value *request=json_value_init_object();JSON_Object *body=json_value_get_object(request);json_object_set_string(body,"cmd","cover");json_object_set_string(body,"url",image);json_object_set_string(body,"uri",current.uri);send_value(request);json_value_free(request);
 }
 if(queue_index>=0&&queue_index<queue_count)queue[queue_index]=current;
 }
 else if(!strcmp(e,"cover"))cover_dirty=true;
 else if(!strcmp(e,"lyrics")){if(track_loaded&&Lyrics_loadTextFile(lyrics_path,duration_ms))copy(lyrics_status,sizeof(lyrics_status),Lyrics_isSynced()?Nxt_text("Synced lyrics","Lời đồng bộ"):Nxt_text("Lyrics · unsynced; scroll with ↑↓","Lời chưa đồng bộ; cuộn bằng ↑↓"));else copy(lyrics_status,sizeof(lyrics_status),Nxt_text("No lyrics available for this track.","Bài này chưa có lời khả dụng."));}
 else if(!strcmp(e,"lyrics_unavailable"))copy(lyrics_status,sizeof(lyrics_status),Nxt_text("No lyrics available for this track.","Bài này chưa có lời khả dụng."));
 else if(!strcmp(e,"favorite")){favorite=json_object_get_boolean(o,"saved")==1;favorite_pending=false;if(favorite_change)Toast_show(favorite?Nxt_text("Saved to Spotify liked songs","Đã thêm vào bài hát yêu thích Spotify"):Nxt_text("Removed from Spotify liked songs","Đã bỏ yêu thích trên Spotify"),1800);favorite_change=false;}
 else if(!strcmp(e,"favorite_error")){favorite_pending=false;if(favorite_change){const char *reason=json_object_get_string(o,"reason");Toast_show(reason?friendly(reason):Nxt_text("Cannot update Spotify library","Chưa cập nhật được thư viện Spotify"),3000);}favorite_change=false;}
 json_value_free(v);
}
void NxtSpotify_tick(void){
 if(backend_ended&&track_loaded&&NxtPCMQueue_drained(pcm_queue)){pthread_mutex_lock(&pcm_lock);bool drained=pcm_used>=pcm_count;pthread_mutex_unlock(&pcm_lock);if(drained){backend_ended=false;playing=false;int next=nxt.repeat_mode==1?queue_index:adjacent(1);if(!stop_requested&&next>=0)select_track(next);else if(!stop_requested&&queue_more){queue_waiting=true;queue_fetch();copy(status,sizeof(status),Nxt_text("Loading next tracks…","Đang tải thêm bài…"));}else{position_ms=duration_ms;stop_track();copy(status,sizeof(status),Nxt_text("Queue ended","Hết hàng đợi"));}}}
 /* Login errors require explicit Reconnect; never loop indefinitely after QR. */
 if(track_loaded&&queue_more&&queue_count-queue_index<=6)queue_fetch();
 if(child<=0)return;
 if(track_loaded){pthread_mutex_lock(&pcm_lock);unsigned packets=pcm_packets;pthread_mutex_unlock(&pcm_lock);if(packets&&!pcm_seen){pcm_seen=true;LOG_info("Spotify PCM reached NXT audio engine\n");diagnostic("Frontend: decoded PCM reached NXT audio engine");}if(!pcm_seen&&loading_since&&SDL_GetTicks()-loading_since>60000){copy(status,sizeof(status),Nxt_text("No audio received; send NXT_Spotify_Log.txt","Chưa nhận âm thanh; gửi NXT_Spotify_Log.txt"));loading_since=0;LOG_info("Spotify playback timeout: no decoded PCM\n");diagnostic("Frontend: timeout, no decoded PCM");}}
 char chunk[4096];ssize_t n;
 while((n=recv(channel,chunk,sizeof(chunk),MSG_DONTWAIT))>0){for(ssize_t i=0;i<n;i++){if(chunk[i]=='\n'){incoming[used]=0;event(incoming);used=0;}else if(used+1<sizeof(incoming))incoming[used++]=chunk[i];else{used=0;copy(status,sizeof(status),"Spotify response too large");}}}
 pid_t dead=waitpid(child,NULL,WNOHANG);if(n==0||dead==child){if(dead==child)child=-1;NxtSpotify_quit();copy(status,sizeof(status),Nxt_text("Spotify stopped; check log","Spotify đã dừng; kiểm tra log"));}
 if(track_loaded&&(Radio_isActive()||Podcast_isActive())){NxtSpotify_quit();return;}
 if(cover_dirty){cover_dirty=false;if(cover)SDL_FreeSurface(cover);cover=IMG_Load(cover_path);if(cover&&(cover->w>2048||cover->h>2048)){SDL_FreeSurface(cover);cover=NULL;}}
 if(playing&&nxt.unplug_pause&&last_dac==1&&!NxtHardware_hasDAC()){pcm_enable(false);command("pause");playing=false;copy(status,sizeof(status),Nxt_text("DAC removed; playback paused","Đã rút DAC; tạm dừng"));}last_dac=NxtHardware_hasDAC();
 if(GetAudioSink()!=last_sink){last_sink=GetAudioSink();LOG_info("Online PCM: output handoff, sink=%d\n",last_sink);}
}
static ModuleExitReason login_help(DisplayContext *d){
 int selected=0;const char *rows[]={Nxt_text("1. Turn on Wi-Fi on the Brick","1. Bật Wi-Fi trên Brick"),Nxt_text("2. Choose Log in by QR","2. Chọn Đăng nhập bằng QR"),Nxt_text("3. Scan the QR on your phone","3. Quét QR bằng điện thoại"),Nxt_text("Or enter the code at spotify.com/pair","Hoặc nhập mã tại spotify.com/pair"),Nxt_text("4. Approve your Spotify Premium account","4. Xác nhận tài khoản Spotify Premium"),Nxt_text("5. Choose music and play on the Brick","5. Chọn nhạc và nghe ngay trên Brick"),Nxt_text("Your login is saved for the next launch","App lưu đăng nhập cho lần mở tiếp theo"),Nxt_text("Playback and library permissions are checked separately","Quyền phát và thư viện được kiểm tra riêng"),Nxt_text("Restricted library: try Play Spotify track link","Thư viện bị giới hạn: chọn Phát link bài hát Spotify"),Nxt_text("PC login: copy spotify-credentials.json next to launch.sh","PC: chép spotify-credentials.json cạnh launch.sh"),Nxt_text("Account → Import PC login; original file is kept","Tài khoản → Nhập PC; app giữ file gốc"),Nxt_text("PC and QR must belong to the same account","Đăng nhập PC và QR phải cùng tài khoản"),Nxt_text("Playback rejected: reconnect and inspect NXT_Spotify_Log.txt","Bị từ chối quyền phát: kết nối lại, xem NXT_Spotify_Log.txt")};
 return Nxt_document(d,Nxt_text("Guide","Hướng dẫn"),rows,sizeof(rows)/sizeof(rows[0]));
}
static ModuleExitReason pair_dialog(DisplayContext *d){
 stop_track();Lyrics_clear();pair_clear();pair_done=pair_failed=false;connected=false;auth_required=auth_upgrade=false;busy=true;login_retry_at=0;pairing=true;command("pair");copy(status,sizeof(status),Nxt_text("Requesting pairing code…","Đang lấy mã ghép…"));int show=0;
 for(;;){ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);
  if(g.should_quit){pair_clear();return MODULE_EXIT_QUIT;}
  if(!g.input_consumed&&!Nxt_lockInput()&&NxtPad_justPressed(BTN_B)){if(pairing)command("cancel_pair");pair_clear();busy=false;return MODULE_EXIT_TO_MENU;}
  if(pair_done){pair_done=false;Toast_show(Nxt_text("Logged in; choose music to play","Đã đăng nhập; chọn bài để nghe"),2500);return MODULE_EXIT_TO_MENU;}
  NxtSkin_begin(s,Nxt_text("Spotify · Log in by QR","Spotify · Đăng nhập bằng QR"),NULL,false);
  int pad=s->w/24,top=NxtSkin_top(s)+pad,edge=s->w*35/100,room=NxtSkin_bottom(s,false)-top-pad*2;if(edge>room)edge=room;
  if(pair_qr_count>0&&edge>0){int module=edge/(pair_qr_count+8),size=module*(pair_qr_count+8);if(module>0){int qx=pad+(edge-size)/2,qy=top+(edge-size)/2;Uint32 black=SDL_MapRGB(s->format,0,0,0),white=SDL_MapRGB(s->format,255,255,255);SDL_FillRect(s,&(SDL_Rect){qx,qy,size,size},white);for(int y=0;y<pair_qr_count;y++)for(int x=0;x<pair_qr_count;x++)if(pair_qr[y][x]=='1')SDL_FillRect(s,&(SDL_Rect){qx+(x+4)*module,qy+(y+4)*module,module,module},black);}}
  int x=pad+edge+pad,w=s->w-x-pad;
  NxtSkin_text(s,24,"spotify.com/pair",x,top,w,NxtSkin_color(NXT_INK));
  NxtSkin_text(s,36,*pair_code?pair_code:Nxt_text("Please wait…","Đang chờ…"),x,top+pad*2,w,NxtSkin_color(NXT_ACCENT));
  NxtSkin_text(s,20,Nxt_text("Scan QR or open the link","Quét QR hoặc mở trang trên"),x,top+pad*4,w,NxtSkin_color(NXT_INK));
  NxtSkin_text(s,20,Nxt_text("Enter code, then approve","Nhập mã rồi xác nhận"),x,top+pad*5,w,NxtSkin_color(NXT_INK));
  NxtSkin_text(s,20,Nxt_text("Spotify Premium · saved on this device","Spotify Premium · lưu trên máy"),x,top+pad*6,w,NxtSkin_color(NXT_MUTED));
  NxtSkin_text(s,20,pair_failed?status:!*pair_code?status:Nxt_text("Waiting for Spotify approval…","Đang chờ Spotify xác nhận…"),pad,top+edge+pad,s->w-pad*2,NxtSkin_color(NXT_MUTED));
  NxtSkin_footer(s,Nxt_text("B Cancel / Back","B Hủy / Quay lại"),false);ModuleCommon_markSurfaceDrawn();int dirty=1;ModuleCommon_PWR_update(&dirty,&show);ModuleCommon_frameEnd(s);
 }
}
ModuleExitReason NxtSpotify_queue(DisplayContext *d){
 int selected=queue_index;
 for(;;){char (*labels)[540]=calloc(queue_count+1,sizeof(*labels));const char **rows=calloc(queue_count+1,sizeof(*rows));if(!labels||!rows){free(labels);free(rows);return MODULE_EXIT_TO_MENU;}for(int i=0;i<queue_count;i++){snprintf(labels[i],sizeof(labels[i]),"%s%02d  %.255s\n%.255s",i==queue_index?"▶ ":"",i+1,queue[i].title,queue[i].artist);rows[i]=labels[i];}
  NxtPick p=Nxt_pickLive(d,Nxt_text("Spotify · Playing queue","Spotify · Danh sách đang phát"),rows,queue_count,&selected,Nxt_text("A Play • X Remove • L1/R1 Move • B Back","A Phát • X Bỏ • L1/R1 Đổi chỗ • B Về"));free(labels);free(rows);
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;
  if(p.action==1&&queue_count){select_track(selected);return MODULE_EXIT_TO_MENU;}
  if(p.action==3&&queue_count){if(selected==queue_index)Toast_show(Nxt_text("Current track is kept","Giữ lại bài đang phát"),1500);else{memmove(&queue[selected],&queue[selected+1],(queue_count-selected-1)*sizeof(Track));if(selected<queue_index)queue_index--;queue_count--;if(selected>=queue_count)selected=queue_count-1;}}
  int other=p.action==11?selected-1:p.action==12?selected+1:-1;
  if(other>=0&&other<queue_count&&selected>=0){Track temp=queue[other];queue[other]=queue[selected];queue[selected]=temp;if(queue_index==selected)queue_index=other;else if(queue_index==other)queue_index=selected;selected=other;}
 }
}
ModuleExitReason NxtSpotify_nowPlaying(DisplayContext *d){
 if(!track_loaded){Toast_show(Nxt_text("Choose a Spotify track first","Chọn một bài Spotify để phát trước"),1800);return MODULE_EXIT_TO_MENU;}
 int show=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;MusicPlaying_leave();MusicPlaying_invalidate();Spectrum_init();
 for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit||ModuleCommon_stopSignalled()){result=MODULE_EXIT_QUIT;break;}if(!track_loaded){Nxt_returnHome();break;}
  if(!g.input_consumed&&!Nxt_lockInput()){
   ModuleExitReason child_result=MODULE_EXIT_TO_MENU;
   if(NxtControl_aux()){}
   else if(NxtControl_pressed(NC_MENU)){result=NxtSession_playerBack();break;}
   else if(NxtControl_pressed(NC_PLAY)){if(!track_loaded&&queue_count>0)select_track(queue_index);else NxtSpotify_togglePause();}
   else if(NxtControl_pressed(NC_PREV))select_track(adjacent(-1));
   else if(NxtControl_pressed(NC_NEXT))select_track(adjacent(1));
   else if(NxtControl_pressed(NC_QUEUE))child_result=NxtSpotify_queue(d);
   else if(NxtControl_pressed(NC_LYRICS)){Settings_setBool(&SETTING_LYRICS_ENABLED,true);child_result=Nxt_lyricsScreen(d);}
   else if(NxtControl_pressed(NC_SPECTRUM))Spectrum_cycleNext();
   else if(NxtControl_pressed(NC_FAVORITE))NxtSpotify_favorite();
   else if(NxtControl_pressed(NC_EQ))child_result=Nxt_dspScreen(d);
   else if(NxtControl_pressed(NC_QUICK))child_result=Nxt_quickOptions(d);
   else if(NxtControl_repeated(NC_SEEK_BACK))NxtSpotify_seek(NxtSpotify_position()-10000);
   else if(NxtControl_repeated(NC_SEEK_FORWARD))NxtSpotify_seek(NxtSpotify_position()+10000);
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
 for(;;){char title[340];char (*labels)[540]=calloc(result_count+2,sizeof(*labels));const char **rows=calloc(result_count+2,sizeof(*rows));if(!labels||!rows){free(labels);free(rows);return MODULE_EXIT_TO_MENU;}int count=0;snprintf(title,sizeof(title),"Spotify · %s",busy?Nxt_text("Loading…","Đang tải…"):status);
  if(result_count){for(int i=0;i<result_count;i++){snprintf(labels[count],sizeof(labels[count]),"%.255s\n%.255s",results[i].title,results[i].artist);rows[count]=labels[count];count++;}}
  if(count&&(has_next||busy))rows[count++]=busy?Nxt_text("Loading more…","Đang tải thêm…"):Nxt_text("Scroll for more tracks","Cuộn để tải thêm bài");
  if(!count){rows[0]=busy?Nxt_text("Please wait; B cancels","Đang tải; B quay lại"):status;count=1;}
  NxtPick p=Nxt_pickLive(d,title,rows,count,&selected,Nxt_text("A Play/Open • Scroll for more • B Back","A Phát/Mở • Cuộn tải thêm • B Về"));free(labels);free(rows);
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;
  if(!busy&&has_next&&(selected>=result_count-5||p.action==12))request(kind,next_offset);
  if(p.action==1&&result_count>0&&selected<result_count){
   if(!strcmp(kind,"playlists")||!strcmp(kind,"search_playlists")||!strcmp(kind,"search_albums")||!strcmp(kind,"search_artists")||!strcmp(kind,"artist_albums")||!strcmp(kind,"saved_albums")){char saved_id[64];copy(saved_id,sizeof(saved_id),playlist_id);copy(playlist_id,sizeof(playlist_id),results[selected].id);const char *next=(!strcmp(kind,"playlists")||!strcmp(kind,"search_playlists"))?"playlist_tracks":!strcmp(kind,"search_artists")?"artist_albums":"album_tracks";ModuleExitReason rr=browse(d,next);if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}copy(playlist_id,sizeof(playlist_id),saved_id);selected=0;wanted=revision;request(kind,0);}
   else{capture_queue(kind);select_track(selected);browser_depth--;ModuleExitReason rr=NxtSpotify_nowPlaying(d);browser_depth++;if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}
  }
 }
}
static bool spotify_track_link(const char *input,char *out,size_t size){
 if(!input||size<37)return false;while(isspace((unsigned char)*input))input++;
 const char *id=NULL;
 if(!strncmp(input,"spotify:track:",14))id=input+14;
 else if(!strncmp(input,"https://open.spotify.com/",25)){
  const char *path=input+25;if(!strncmp(path,"intl-",5)){const char *slash=strchr(path,'/');if(!slash||slash-path>20)return false;path=slash+1;}
  if(!strncmp(path,"track/",6))id=path+6;
 }
 if(!id||strlen(id)<22)return false;for(int i=0;i<22;i++)if(!isalnum((unsigned char)id[i])||(unsigned char)id[i]>127)return false;
 const char *rest=id+22;if(*rest!='?'&&*rest!='#'){while(isspace((unsigned char)*rest))rest++;if(*rest)return false;}
 snprintf(out,size,"spotify:track:%.22s",id);return true;
}
static ModuleExitReason play_link(DisplayContext *d){
 char *input=Keyboard_openLatin(Nxt_text("Spotify track link / URI","Link / URI bài hát Spotify"),240);if(!input)return MODULE_EXIT_TO_MENU;
 Track t={0};bool valid=spotify_track_link(input,t.uri,sizeof(t.uri));free(input);
 if(!valid){Toast_show(Nxt_text("Use open.spotify.com/track/... or spotify:track:...","Dùng link open.spotify.com/track/... hoặc spotify:track:..."),4000);return MODULE_EXIT_TO_MENU;}
 copy(t.title,sizeof(t.title),Nxt_text("Spotify track","Bài hát Spotify"));queue_count=0;queue_more=queue_request=queue_waiting=false;queue_retry=0;
 if(!append_track(&queue,&queue_count,&queue_capacity,&t)){Toast_show(Nxt_text("Not enough memory","Không đủ bộ nhớ"),2000);return MODULE_EXIT_TO_MENU;}
 select_track(0);return NxtSpotify_nowPlaying(d);
}
static ModuleExitReason search_menu(DisplayContext *d){
 int selected=0;const char *rows[]={Nxt_text("Tracks","Bài hát"),Nxt_text("Albums","Album"),Nxt_text("Artists","Nghệ sĩ"),Nxt_text("Playlists · access may be restricted","Playlist · có thể bị hạn chế quyền")};const char *kinds[]={"search","search_albums","search_artists","search_playlists"};
 for(;;){NxtPick p=Nxt_pick(d,Nxt_text("Search Spotify","Tìm trên Spotify"),rows,4,&selected,Nxt_text("A Search • B Back","A Tìm • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
  char *q=Keyboard_open(Nxt_text("Search Spotify","Tìm trên Spotify"),200);if(!q)continue;copy(query,sizeof(query),q);free(q);if(!*query)continue;ModuleExitReason rr=browse(d,kinds[selected]);if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
 }
}
static ModuleExitReason discover(DisplayContext *d){
 const char *terms[]={"Vietnam pop","K-pop","J-pop","Mandopop","Thai pop","acoustic","jazz","piano","lofi","rock"};
 const char *rows[]={Nxt_text("Vietnamese music","Nhạc Việt"),"K-pop","J-pop",Nxt_text("Chinese pop","Nhạc Hoa"),Nxt_text("Thai pop","Nhạc Thái"),"Acoustic","Jazz","Piano","Lo-fi","Rock"};int selected=0;
 for(;;){NxtPick p=Nxt_pick(d,Nxt_text("Discover · themed searches","Khám phá · tìm theo chủ đề"),rows,10,&selected,Nxt_text("A Browse • B Back","A Xem • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==1){copy(query,sizeof(query),terms[selected]);ModuleExitReason rr=browse(d,"search");if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}}
}
static ModuleExitReason account_menu(DisplayContext *d){
 int selected=0;for(;;){char heading[340];snprintf(heading,sizeof(heading),"Spotify · %s",*account_name?account_name:status);
  const char *rows[]={Nxt_text("Log in / switch account by QR","Đăng nhập / đổi tài khoản bằng QR"),Nxt_text("Reconnect","Kết nối lại"),Nxt_text("Log out","Đăng xuất"),Nxt_text("Advanced · import existing PC login","Nâng cao · nhập đăng nhập PC cũ"),Nxt_text("Login guide","Hướng dẫn đăng nhập")};
  NxtPick p=Nxt_pickLive(d,heading,rows,5,&selected,Nxt_text("A Select • B Back","A Chọn • B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
  ModuleExitReason rr=MODULE_EXIT_TO_MENU;
  if(selected==0){if(start())rr=pair_dialog(d);}
  if(selected==1){if(child>0){stop_track();busy=true;command("login");}else start();}
  if(selected==2){NxtSpotify_quit();nxt.spotify_auth_source=0;Nxt_save();unlink(creds);unlink(playback_creds);char temp[1040];snprintf(temp,sizeof(temp),"%s.tmp",playback_creds);unlink(temp);unlink(cover_path);unlink(lyrics_path);*account_name=0;auth_required=true;auth_upgrade=false;copy(status,sizeof(status),Nxt_text("Logged out","Đã đăng xuất"));return MODULE_EXIT_TO_MENU;}
  if(selected==3&&import_credentials())start();
  if(selected==4)rr=login_help(d);
  if(rr==MODULE_EXIT_QUIT)return rr;
 }
}
static ModuleExitReason spotify_menu_run(DisplayContext *d){
 int selected=0;bool tried_pair=false;paths();if(!start())Toast_show(status,2500);
 for(;;){
  if(!busy&&!track_loaded&&(auth_required||auth_upgrade)&&!tried_pair){tried_pair=true;ModuleExitReason rr=pair_dialog(d);if(rr==MODULE_EXIT_QUIT)return rr;selected=0;}
  char heading[340];snprintf(heading,sizeof(heading),"Spotify · %s",connected&&*account_name?account_name:status);
  const char *online[]={Nxt_text("Search","Tìm kiếm"),Nxt_text("Suggested online tracks","Bài hát online gợi ý"),Nxt_text("Playable playlists · your account","Playlist nghe được · tài khoản của bạn"),Nxt_text("Liked songs","Bài hát đã thích"),Nxt_text("Saved albums","Album đã lưu"),Nxt_text("Now playing","Đang phát"),Nxt_text("Play Spotify track link","Phát link bài hát Spotify"),Nxt_text("Account","Tài khoản"),Nxt_text("Login guide","Hướng dẫn đăng nhập")};
  const char *offline[]={Nxt_text("Log in by QR","Đăng nhập bằng QR"),Nxt_text("Login guide","Hướng dẫn đăng nhập"),Nxt_text("Account & tools","Tài khoản & công cụ")};
  NxtPick p=Nxt_pickLive(d,heading,connected?online:offline,connected?9:3,&selected,Nxt_text("A Open • B Back","A Mở • B Quay lại"));
  if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
  ModuleExitReason rr=MODULE_EXIT_TO_MENU;
  if(!connected){if(selected==0){tried_pair=true;if(!busy&&start())rr=pair_dialog(d);}else if(selected==1)rr=login_help(d);else rr=account_menu(d);}
  else if(selected==0)rr=search_menu(d);
  else if(selected==1)rr=NxtSpotify_recommendations(d,false);
  else if(selected>=2&&selected<=4)rr=browse(d,selected==2?"playlists":selected==3?"saved_tracks":"saved_albums");
  else if(selected==5)rr=NxtSpotify_nowPlaying(d);
  else if(selected==6)rr=play_link(d);
  else if(selected==7)rr=account_menu(d);
  else rr=login_help(d);
  if(rr==MODULE_EXIT_QUIT)return rr;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
 }
}
ModuleExitReason NxtSpotify_run(DisplayContext *d){
 NxtSkinSource previous=NxtSkin_setSource(NXT_SOURCE_SPOTIFY);
 ModuleExitReason result=spotify_menu_run(d);
 NxtSkin_setSource(previous);return result;
}

ModuleExitReason NxtSpotify_playSaved(DisplayContext *d,const char *uri,const char *title,const char *artist){
 if(!uri||!*uri||!start()){NxtLog_step("Spotify","history-start",false,"Cannot start source");return MODULE_EXIT_TO_MENU;}
 uint32_t began=SDL_GetTicks();int selected=0;const char *waiting[]={Nxt_text("Connecting to source…","Đang kết nối nguồn nhạc…")};
 while(!connected&&SDL_GetTicks()-began<30000){NxtSpotify_tick();NxtPick p=Nxt_pickLive(d,"Spotify",waiting,1,&selected,Nxt_text("B Cancel","B Hủy"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}
 if(!connected){Toast_show(Nxt_text("Open source and reconnect your account","Mở nguồn nhạc và kết nối lại tài khoản"),3000);NxtLog_step("Spotify","history-login",false,"Connection timed out");return MODULE_EXIT_TO_MENU;}
 Track track={0};copy(track.uri,sizeof(track.uri),uri);copy(track.title,sizeof(track.title),title);copy(track.artist,sizeof(track.artist),artist);queue_count=0;queue_more=queue_request=queue_waiting=false;queue_retry=0;
 if(!append_track(&queue,&queue_count,&queue_capacity,&track))return MODULE_EXIT_TO_MENU;select_track(0);return NxtSpotify_nowPlaying(d);
}

ModuleExitReason NxtSpotify_recommendations(DisplayContext *d,bool playlists){
 if(!start())return MODULE_EXIT_TO_MENU;
 if(!connected){uint32_t began=SDL_GetTicks();int select=0;const char *wait[]={Nxt_text("Connecting to music source…","Đang kết nối nguồn nhạc…")};while(!connected&&busy&&SDL_GetTicks()-began<30000){NxtPick p=Nxt_pickLive(d,"Spotify",wait,1,&select,Nxt_text("B Cancel","B Hủy"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}if(!connected)return NxtSpotify_run(d);}
 if(playlists){int mix=0;const char *rows[]={Nxt_text("NXT Mix · your genre","NXT Mix · theo thể loại"),Nxt_text("NXT Mix · your mood","NXT Mix · theo tâm trạng"),Nxt_text("Account playlists · verified access","Playlist tài khoản · đã kiểm tra quyền")};for(;;){NxtPick p=Nxt_pick(d,Nxt_text("Suggested playlists","Playlist gợi ý"),rows,3,&mix,Nxt_text("A Open · B Back","A Mở · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;NxtCustom_query(query,sizeof(query),mix==1?2:1);ModuleExitReason r=browse(d,mix==2?"playlists":"search");if(r==MODULE_EXIT_QUIT)return r;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}}
 int selected=0;for(;;){char preference[200];snprintf(preference,sizeof(preference),"%s · %s",NxtCustom_genre(nxt.genre),NxtCustom_mood(nxt.mood));const char *rows[]={preference,Nxt_text("By your genre","Theo thể loại của bạn"),Nxt_text("By your mood","Theo tâm trạng"),Nxt_text("Acoustic versions","Phiên bản acoustic"),Nxt_text("Live performances","Bản biểu diễn live"),Nxt_text("Playable playlists","Playlist nghe được"),Nxt_text("More genres","Thêm thể loại")};NxtPick p=Nxt_pick(d,Nxt_text("Suggested online music","Nhạc online gợi ý"),rows,7,&selected,Nxt_text("A Browse · B Back","A Xem · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;ModuleExitReason r;if(selected==6)r=discover(d);else{NxtCustom_query(query,sizeof(query),selected);r=selected==5?NxtSpotify_recommendations(d,true):browse(d,"search");}if(r==MODULE_EXIT_QUIT)return r;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}
}
