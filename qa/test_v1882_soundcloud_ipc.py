#!/usr/bin/env python3
"""Execute production native SoundCloud import, metadata, control and lifecycle."""
from pathlib import Path
import subprocess,tempfile,os,json
r=Path(__file__).resolve().parents[1];src=r/'upstream/src';q=r/'.cache/v1882-soundcloud-qa';q.mkdir(parents=True,exist_ok=True)
sdk=r/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include';h=q/'include';h.mkdir(exist_ok=True)
for name in ['SDL2','GLES2','EGL','KHR']:
    if not (h/name).exists():(h/name).symlink_to(sdk/name,target_is_directory=True)
if not (h/'samplerate.h').exists():(h/'samplerate.h').symlink_to(r/'.cache/nextui18-build/headers/samplerate/samplerate.h')
c=q/'ipc.c';c.write_text(r'''
#include "nxt_soundcloud.c"
#include "nxt_playback.h"
#include "spotify_stub.inc"
#include <assert.h>
NxtConfig nxt;
const char *Nxt_text(const char *en,const char *vi){return vi;}
void ModuleCommon_setAutosleepDisabled(bool x){}
void SDL_FreeSurface(SDL_Surface *s){}
Uint32 SDL_GetTicks(void){return 1234;}
void LOG_note(int level,const char *fmt,...){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}
int userdata_snpath(const char *rel,char *out,size_t size){return snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
static int sink=2;static bool dac=true;
int GetAudioSink(void){return sink;}
bool NxtHardware_hasDAC(void){return dac;}
bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
static int stopped;
void Background_stopAll(void){stopped++;}
void Player_stop(void){}
void Player_resumeAudio(void){}
void PlayerModule_markStopped(void){}
bool PlayerModule_isActive(void){return false;}
int Player_getPosition(void){return 0;}
void Resume_updatePosition(int ms){}
static BackgroundPlayerType bg;
BackgroundPlayerType Background_getActive(void){return bg;}
void Background_setActive(BackgroundPlayerType v){bg=v;}
void Lyrics_clear(void){}
bool Lyrics_loadTextFile(const char *path,int duration){return true;}
bool Lyrics_isSynced(void){return true;}
ToastToken Toast_show(const char *message,uint32_t duration){return 0;}
SDL_Surface *IMG_Load(const char *path){return NULL;}
const TrackInfo *Player_getTrackInfo(void){static TrackInfo i;return &i;}
const char *Player_getCurrentFile(void){return "";}
bool Nxt_isFavorite(const char *path){return false;}
int Player_getDuration(void){return 0;}

static void catalog_page(int page,int count,bool more,bool duplicate){
 char json[18000];size_t n=snprintf(json,sizeof(json),"{\"event\":\"results\",\"kind\":\"search\",\"offset\":%d,\"next_offset\":%d,\"has_next\":%s,\"items\":[",page,page+count,more?"true":"false");
 for(int i=0;i<count;i++){int id=page+i-(duplicate?1:0);n+=snprintf(json+n,sizeof(json)-n,"%s{\"title\":\"Track %d\",\"uri\":\"catalog:track:%d\",\"id\":\"%d\"}",i?",":"",id,id,id);assert(n<sizeof(json)-3);}
 strcpy(json+n,"]}");event(json);
}
static void catalog_regression(void){
 queue_request=false;result_count=0;for(int page=0;page<80;page+=20){catalog_page(page,20,true,false);assert(result_count==page+20);}
 capture_queue("search");assert(queue_count==80&&queue_more&&queue_offset==80);
 nxt.repeat_mode=2;queue_index=79;assert(adjacent(1)==-1);
 queue_request=true;catalog_page(80,2,false,true);assert(queue_count==81&&!queue_more&&adjacent(1)==80);
 queue_index=80;assert(adjacent(1)==0);nxt.repeat_mode=0;
 free(queue);queue=NULL;queue_count=queue_capacity=0;free(results);results=NULL;result_count=result_capacity=0;queue_more=has_next=false;
 puts("PASS append 80 catalog results, listening queue grows to 81 with deduplication, no early repeat while more pages remain, final repeat-all wraps only at actual catalog end.");
}

static void poll(void){for(int i=0;i<100;i++){usleep(5000);NxtSoundCloud_tick();}}

#include "nxt_bitperfect.h"
bool NxtStartup_preview(void){return false;}int NxtStartup_position(void){return 0;}const TrackInfo *NxtStartup_info(void){static TrackInfo i;return &i;}int NxtStartup_takeSeek(const char*s,const char*p){return -1;}
static int mode_changes;
void NxtVolume_release(void){}void NxtVolume_routeChanged(void){}bool NxtVolume_hardware(void){return false;}
int NxtHardware_dacCard(void){return 3;}

PlayerState Player_getState(void){return PLAYER_STATE_PAUSED;}
bool NxtDrive_isPath(const char*p){return p&&!strncmp(p,"nxtdrive://",11);}
void Player_switchBitMode(bool bit,bool reload,int state){mode_changes++;nxt.bit_perfect=bit;}
PlayerState NxtBit_state(void){return PLAYER_STATE_PAUSED;}bool NxtBit_ready(void){return false;}int NxtBit_loadStatus(void){return 1;}const char *NxtBit_error(void){return "";}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t n){memset(i,0,sizeof(*i));snprintf(p,n,"local.flac");return 1;}

int main(void){
 assert(start());assert(access(creds,F_OK));pid_t owned=child;poll();assert(connected);
 event("{\"event\":\"results\",\"kind\":\"search\",\"items\":[{\"title\":\"日本語 ไทย 한국어 中文 Việt\",\"artist\":\"Uploader\",\"album\":\"SoundCloud · Uploader\",\"uri\":\"soundcloud:tracks:1\",\"duration_ms\":90000}],\"has_next\":true}");
 assert(result_count==1&&has_next);capture_queue("search");queue_more=false;nxt.audio_mode=2;nxt.bit_perfect=1;int before_mode_changes=mode_changes;select_track(0);assert(!nxt.bit_perfect&&nxt.audio_mode==2&&mode_changes==before_mode_changes+1);poll();assert(stopped==1&&bg==BG_SOUNDCLOUD&&NxtSoundCloud_active()&&playing);
 assert(NxtPlayback_info()==&track_info&&strstr(track_info.title,"日本語"));assert(track_info.sample_rate==44100&&track_info.bitrate==160000);assert(NxtPlayback_duration()==90000);
 assert(NxtSoundCloud_position()==0);float audio[2048];NxtSoundCloud_togglePause();assert(!playing&&!pcm_enabled);assert(NxtSoundCloud_audioSamples(audio,1024,48000)==0);assert(NxtSoundCloud_position()==0);
 NxtSoundCloud_togglePause();assert(playing);int frames=NxtSoundCloud_audioSamples(audio,1024,48000);assert(frames>950);assert(NxtSoundCloud_position()>=19&&NxtSoundCloud_position()<=21);
 unsigned epoch=pcm_epoch;NxtSoundCloud_seek(42000);poll();assert(pcm_epoch!=epoch&&playing);assert(NxtPlayback_position()==42000);assert(NxtSoundCloud_audioSamples(audio,1024,48000)>950);assert(NxtPlayback_position()>=42019);
 char stale[300];snprintf(stale,sizeof(stale),"{\"event\":\"error\",\"reason\":\"track_restricted\",\"operation\":\"play\",\"uri\":\"soundcloud:tracks:1\",\"stream_id\":%u}",epoch);event(stale);assert(track_loaded&&playing);
 event("{\"event\":\"favorite\",\"uri\":\"soundcloud:tracks:1\",\"stream_id\":0,\"saved\":true}");assert(NxtPlayback_favorite());
 expected_serial=9;event("{\"event\":\"results\",\"request_id\":8,\"items\":[]}");assert(result_count==1);
 event("{\"event\":\"source_link\",\"qr\":[\"111\",\"101\",\"111\"]}");assert(source_qr_count==3);event("{\"event\":\"source_link\",\"qr\":[\"10\",\"1x\"]}");assert(source_qr_count==0);
 nxt.unplug_pause=true;dac=false;NxtSoundCloud_tick();assert(track_loaded&&!playing);dac=true;NxtSoundCloud_togglePause();assert(playing);sink=1;NxtSoundCloud_tick();assert(playing);sink=2;NxtSoundCloud_tick();assert(playing);
 event("{\"event\":\"error\",\"operation\":\"play\",\"reason\":\"track_restricted\",\"uri\":\"soundcloud:tracks:1\"}");assert(connected&&!playing&&!track_loaded&&strstr(status,"giới hạn"));
 char utf[8];copy(utf,sizeof(utf),"日本語日本語");assert(!strcmp(utf,"日本"));
 catalog_regression();NxtSoundCloud_quit();assert(child==-1&&channel==-1&&!NxtSoundCloud_hasMetadata());assert(kill(owned,0)==-1&&errno==ESRCH);assert(waitpid(owned,NULL,WNOHANG)==-1&&errno==ECHILD);return 0;
}
''')
flags=['gcc','-O1','-g','-no-pie','-fsanitize=address,undefined','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-I'+str(h),'-I'+str(src),'-I'+str(r/'qa'),'-I'+str(src/'include'),'-I'+str(r/'upstream/NextUI/workspace/all/common'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/libmsettings')]
subprocess.run(flags+[str(c),str(src/'include/parson/parson.c'),str(src/'nxt_audio_mode.c'),'-Wl,--gc-sections','-l:libsamplerate.so.0','-lm','-lpthread','-o',str(q/'ipc')],check=True)
with tempfile.TemporaryDirectory() as td:
    t=Path(td);(t/'bin/tg5040').mkdir(parents=True);data=t/'userdata';data.mkdir()
    f=t/'bin/tg5040/nxt-soundcloud-backend';f.write_text('''#!/usr/bin/env python3
import sys,json,os,socket,struct,time
pcm=socket.socket(fileno=int(os.environ['NXT_SOUNDCLOUD_PCM_FD']))
assert pcm.getsockopt(socket.SOL_SOCKET,socket.SO_TYPE)==socket.SOCK_SEQPACKET
assert os.environ['NXT_SOUNDCLOUD_CREDENTIALS'].endswith('/soundcloud/credentials.json')
assert os.environ['NXT_SOUNDCLOUD_COVER'].endswith('/soundcloud/cover.jpg')
assert os.environ['NXT_SOUNDCLOUD_LYRICS'].endswith('/soundcloud/lyrics.txt')
for line in sys.stdin:
 v=json.loads(line)
 if v['cmd']=='login':print(json.dumps({'event':'logged_in'}),flush=True)
 if v['cmd'] in ('play','seek'):
  pcm.send(struct.pack('<IIII',v['stream_id'],1764,44100,0)+struct.pack('<ff',.25,-.5)*882)
  print(json.dumps({'event':'playing','uri':v['uri'],'stream_id':v['stream_id'],'sample_rate':44100,'codec':'AAC','bitrate_kbps':160}),flush=True)
 if v['cmd']=='check_favorite':print(json.dumps({'event':'favorite','uri':v['uri'],'stream_id':v['stream_id'],'saved':False}),flush=True)
 if v['cmd']=='quit':time.sleep(10)
''');f.chmod(0o755)
    (t/'soundcloud-credentials.json').write_text(json.dumps({'client_id':'FAKE-ID','client_secret':'FAKE-SECRET','access_token':'IGNORED-USER-TOKEN'}))
    subprocess.run([str(q/'ipc')],cwd=t,env={**os.environ,'QA_DATA':str(data),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},check=True,timeout=15)
    assert not (data/'soundcloud/credentials.json').exists()
    assert (t/'soundcloud-credentials.json').exists()
    log=(t/'NXT_SoundCloud_Log.txt').read_text();assert 'FAKE-SECRET' not in log and 'IGNORED-USER-TOKEN' not in log
print('PASS production SoundCloud frontend: key-free startup, no credential import, UTF-8 metadata, shared player, pause/seek PCM clock, stale replies, local bookmark status, QR validation, DAC unplug/route pause, restricted-track cleanup, helper quit/kill/reap, token-free logs.')

(r/'logs'/'v1882_soundcloud_ipc-test.json').write_text(json.dumps({"exit":0,"production_provider_C":True,"production_audio_policy":True,"raw_to_Normal_keeps_helper_and_queue":True,"local_IPC_fixture":True,"ASan_UBSan":True,"live_account_playback_tested":False},indent=2)+"\n")
