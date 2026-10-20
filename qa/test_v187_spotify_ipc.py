#!/usr/bin/env python3
"""Execute production C parsing/import/lifecycle with a fake local backend."""
from pathlib import Path
import subprocess,tempfile,os,json
r=Path(__file__).resolve().parents[1];src=r/'upstream/src';q=r/'.cache/v187-spotify-qa';q.mkdir(parents=True,exist_ok=True)
sdk=r/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include';h=q/'include';h.mkdir(exist_ok=True)
if (h/'samplerate.h').is_symlink():(h/'samplerate.h').unlink()
(h/'samplerate.h').symlink_to(r/'.cache/nextui18-build/headers/samplerate/samplerate.h')
for name in ['SDL2','GLES2','EGL','KHR']:
 if not (h/name).exists():(h/name).symlink_to(sdk/name,target_is_directory=True)
code=q/'ipc.c'
code.write_text('''#include "soundcloud_stub.inc"\n#include "nxt_spotify.c"
#include <assert.h>
NxtConfig nxt;
static int saves;bool Nxt_save(void){saves++;return true;}
const char *Nxt_text(const char *en,const char *vi){return vi;}
void ModuleCommon_setAutosleepDisabled(bool x){}
void SDL_FreeSurface(SDL_Surface *s){}
Uint32 SDL_GetTicks(void){return 1234;}
void LOG_note(int level,const char *fmt,...){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}
int userdata_snpath(const char *rel,char *out,size_t size){return snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
static int audio_sink=2;static bool dac_present=true;
int GetAudioSink(void){return audio_sink;}
bool NxtHardware_hasDAC(void){return dac_present;}
bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
void Background_stopAll(void){}
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
#include "nxt_bitperfect.h"
bool NxtStartup_preview(void){return false;}int NxtStartup_position(void){return 0;}const TrackInfo *NxtStartup_info(void){static TrackInfo i;return &i;}int NxtStartup_takeSeek(const char*s,const char*p){return -1;}
static int mode_changes;
void NxtVolume_release(void){}void NxtVolume_routeChanged(void){}bool NxtVolume_hardware(void){return false;}
int NxtHardware_dacCard(void){return 3;}
const char *Player_getCurrentFile(void){return "local.flac";}
PlayerState Player_getState(void){return PLAYER_STATE_PAUSED;}
bool NxtDrive_isPath(const char*p){return p&&!strncmp(p,"nxtdrive://",11);}
void Player_switchBitMode(bool bit,bool reload,int state){mode_changes++;nxt.bit_perfect=bit;}
PlayerState NxtBit_state(void){return PLAYER_STATE_PAUSED;}bool NxtBit_ready(void){return false;}int NxtBit_loadStatus(void){return 1;}const char *NxtBit_error(void){return "";}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t n){memset(i,0,sizeof(*i));snprintf(p,n,"local.flac");return 1;}


int main(int argc,char **argv){
 assert(start());NxtSpotify_quit();assert(import_credentials());assert(nxt.spotify_auth_source==1&&saves>0);assert(!access("spotify-credentials.json",F_OK));
 assert(import_credentials());assert(nxt.spotify_auth_source==1&&saves>0);assert(!access("spotify-credentials.json",F_OK));
 char parsed[128];assert(spotify_track_link("https://open.spotify.com/track/0123456789012345678901?si=public",parsed,sizeof(parsed)));assert(!strcmp(parsed,"spotify:track:0123456789012345678901"));
 assert(spotify_track_link("https://open.spotify.com/intl-vi/track/0123456789012345678901",parsed,sizeof(parsed)));
 assert(spotify_track_link(" spotify:track:0123456789012345678901 ",parsed,sizeof(parsed)));
 assert(!spotify_track_link("https://open.spotify.com/playlist/0123456789012345678901",parsed,sizeof(parsed)));
 assert(!spotify_track_link("https://evil.example/track/0123456789012345678901",parsed,sizeof(parsed)));
 assert(!spotify_track_link("spotify:track:012345678901234567890!",parsed,sizeof(parsed)));
 FILE *bad=fopen("spotify-credentials.json","w");assert(bad);fputs("{}",bad);fclose(bad);assert(!import_credentials());assert(nxt.spotify_auth_source==1);assert(!access("spotify-credentials.json",F_OK));
 JSON_Value *kept=json_parse_file(creds);assert(kept&&json_object_get_string(json_value_get_object(kept),"access_token"));json_value_free(kept);
 assert(start());pid_t owned=child;
 for(int i=0;i<100&&!connected;i++){usleep(10000);NxtSpotify_tick();}assert(connected);
 assert(strstr(status,"Sẵn sàng"));
 pairing=pair_finishing=true;pair_done=pair_failed=false;
 event("{\\"event\\":\\"logged_in\\",\\"playback_ready\\":true,\\"library_ready\\":false,\\"library_warning\\":\\"rate_limited:37\\"}");
 assert(connected&&pair_done&&!pairing&&!login_retry_at&&strstr(status,"Thư viện"));
 track_loaded=playing=true;event("{\\"event\\":\\"error\\",\\"reason\\":\\"playback_auth_rejected\\",\\"operation\\":\\"play\\"}");assert(connected&&!playing&&!track_loaded);
 pairing=true;command("pair");for(int i=0;i<100&&!*pair_code;i++){usleep(10000);NxtSpotify_tick();}assert(!strcmp(pair_code,"ABCDEF")&&pair_qr_count==3);
 event("{\\"event\\":\\"pair_code\\",\\"code\\":\\"ABCDEF\\",\\"qr\\":[\\"10\\",\\"1x\\"]}");assert(pair_qr_count==0);
 command("cancel_pair");pair_clear();event("{\\"event\\":\\"pair_code\\",\\"code\\":\\"LATE\\",\\"qr\\":[\\"1\\"]}");assert(!*pair_code&&pair_qr_count==0);
 pairing=true;event("{\\"event\\":\\"error\\",\\"reason\\":\\"pair_approval_failed\\"}");assert(pair_failed&&!pairing&&!connected);
 pair_done=pair_failed=false;pairing=true;event("{\\"event\\":\\"paired\\"}");assert(!pair_done&&pairing&&pair_qr_count==0&&nxt.spotify_auth_source==2);event("{\\"event\\":\\"logged_in\\",\\"needs_upgrade\\":false}");assert(pair_done&&!pairing&&connected);
 event("{\\"event\\":\\"results\\",\\"kind\\":\\"search\\",\\"items\\":[{\\"title\\":\\"日本語 ไทย 한국어 中文\\",\\"artist\\":\\"Artist\\",\\"uri\\":\\"spotify:track:0123456789012345678901\\",\\"duration_ms\\":180000}],\\"has_next\\":true}");
 assert(result_count==1&&has_next);assert(!strcmp(results[0].title,"日本語 ไทย 한국어 中文"));capture_queue("search");nxt.audio_mode=2;nxt.bit_perfect=1;int before_mode_changes=mode_changes;select_track(0);assert(!nxt.bit_perfect&&nxt.audio_mode==2&&mode_changes==before_mode_changes+1);assert(bg==BG_SPOTIFY&&NxtSpotify_active());assert(NxtSpotify_info()->duration_ms==180000&&!strcmp(NxtSpotify_info()->title,"日本語 ไทย 한국어 中文"));unsigned generation=NxtSpotify_generation();select_track(0);assert(child==owned&&NxtSpotify_generation()==generation+1);
 expected_serial=22;event("{\\"event\\":\\"results\\",\\"request_id\\":21,\\"items\\":[]}");assert(result_count==1);
 event("{\\"event\\":\\"playing\\",\\"position_ms\\":42000}");assert(playing&&position_ms==42000);audio_sink=0;dac_present=false;NxtSpotify_tick();assert(playing&&track_loaded);audio_sink=2;dac_present=true;NxtSpotify_tick();assert(playing&&track_loaded);
 event("{\\"event\\":\\"paused\\",\\"position_ms\\":43000}");assert(!playing&&position_ms==43000);
 event("{\\"event\\":\\"error\\",\\"reason\\":\\"api_http_403\\"}");assert(strstr(status,"Spotify"));
 char utf[8];copy(utf,sizeof(utf),"日本語日本語");assert(!strcmp(utf,"日本"));
 queue_count=2;queue_index=0;stop_requested=true;track_loaded=true;
 event("{\\"event\\":\\"metadata\\",\\"title\\":\\"Đặng Thái\\",\\"artist\\":\\"Nghệ sĩ\\",\\"album\\":\\"Album\\",\\"duration_ms\\":321000}");
 assert(!strcmp(track_info.title,"Đặng Thái")&&duration_ms==321000);
 pairing=pair_finishing=true;event("{\\"event\\":\\"error\\",\\"operation\\":\\"login\\",\\"reason\\":\\"playback_auth_rejected\\"}");
 assert(!connected&&!busy&&!login_retry_at&&!pairing&&pair_failed&&!auth_required);
 event("{\\"event\\":\\"ended\\"}");assert(queue_index==0&&!track_loaded);
 NxtSpotify_quit();assert(child==-1&&channel==-1&&!connected&&!playing);assert(kill(owned,0)==-1&&errno==ESRCH);
 assert(waitpid(owned,NULL,WNOHANG)==-1&&errno==ECHILD);
 return 0;
}''')
flags=['gcc','-O1','-g','-no-pie','-fsanitize=address,undefined','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-I'+str(h),'-I'+str(src),'-I'+str(r/'qa'),'-I'+str(src/'include'),'-I'+str(r/'upstream/NextUI/workspace/all/common'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(r/'upstream/NextUI/workspace/tg5040/libmsettings')]
subprocess.run(flags+[str(code),str(src/'include/parson/parson.c'),str(src/'nxt_audio_mode.c'),'-Wl,--gc-sections','-l:libsamplerate.so.0','-lm','-lpthread','-o',str(q/'ipc')],check=True)
with tempfile.TemporaryDirectory() as td:
 t=Path(td);(t/'bin/tg5040').mkdir(parents=True);data=t/'userdata';data.mkdir()
 fixture=t/'bin/tg5040/nxt-spotify-backend'
 fixture.write_text('''#!/usr/bin/env python3
import json,sys,time,os,socket,signal
signal.signal(signal.SIGPIPE,signal.SIG_DFL)
pcm=socket.socket(fileno=int(os.environ['NXT_SPOTIFY_PCM_FD']))
assert pcm.getsockopt(socket.SOL_SOCKET,socket.SO_TYPE)==socket.SOCK_SEQPACKET
assert os.environ['NXT_SPOTIFY_PLAYBACK_CREDS'].endswith('/spotify/playback.json')
assert os.environ['NXT_SPOTIFY_DIAG'].endswith('/NXT_Spotify_Log.txt')
assert os.environ['NXT_SPOTIFY_LYRICS'].endswith('/spotify/lyrics.txt')
for line in sys.stdin:
 v=json.loads(line)
 if v['cmd']=='login':
  assert v.get('source') in ['auto','pc','qr']
  with open('login-sources.txt','a') as f:f.write(v['source']+"\\n")
  print('{"event":"logged_in"}',flush=True)
 if v['cmd']=='pair':print('{"event":"pair_code","code":"ABCDEF","qr":["111","101","111"]}',flush=True)
 if v['cmd']=='quit':time.sleep(10)
''');fixture.chmod(0o755)
 (t/'spotify-credentials.json').write_text(json.dumps({'client_id':'320f1ca055564990bcdf4de6fe58b397','access_token':'FAKE-DO-NOT-USE','refresh_token':'FAKE-DO-NOT-USE'}))
 subprocess.run([str(q/'ipc')],cwd=t,env={**os.environ,'QA_DATA':str(data),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},check=True,timeout=8)
 assert (data/'spotify/credentials.json').stat().st_mode&0o777==0o600
 sources=(t/'login-sources.txt').read_text().splitlines();assert sources[0]=='auto' and 'pc' in sources and 'qr' in sources
 diag=(t/'NXT_Spotify_Log.txt').read_text();assert diag.count('Frontend build:')>=2;assert 'Stock-v1.3-Hardware-Login-Fix2' in diag and 'pairing code displayed' in diag;assert 'FAKE-DO-NOT-USE' not in diag and 'ABCDEF' not in diag
print('PASS production C: QR-first start without PC credentials, native metadata provider, Spotify background ownership, track changes keep the helper, credential import, verified playback with a limited library, no endless retry, non-destructive PC import, explicit PC/QR selection and preserved login attempts, valid track links and playback metadata, QR/code event validation/cancel/approval/errors, UTF-8 metadata, stale replies, PCM/playback error cleanup, private diagnostic log, owned helper kill/reap; tokens never sent in argv/IPC.')

(r/'logs'/'v187_spotify_ipc-test.json').write_text(json.dumps({"exit":0,"production_provider_C":True,"production_audio_policy":True,"raw_to_Normal_keeps_helper_and_queue":True,"local_IPC_fixture":True,"ASan_UBSan":True,"live_account_playback_tested":False},indent=2)+"\n")
