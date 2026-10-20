/* Production NextUI discovery, route writer and shared audio reopen path.
 * D-Bus messages, hardware files and SDL device are explicit test inputs. */
#define config_path qa_nextui_config_path
#include "nxt_nextui.c"
#undef config_path
#ifdef NXT_QA_BASELINE_HARDWARE
#include "baseline-hardware.c"
#else
#include "nxt_hardware.c"
#endif
#include <alsa/asoundlib.h>
static int qa_reset(void);
#define snd_config_update_free_global qa_reset
#include "nxt_player_core.c"
#undef snd_config_update_free_global
#include <assert.h>
#include <stdarg.h>
NxtConfig nxt;
static Uint32 qa_ticks=100;
static int qa_sink,qa_volume=8,qa_opens,qa_resets,qa_source,qa_fail;
static bool qa_device;
#ifdef NXT_ROM_NXREDUX
static int qa_music=20,qa_mute;
int GetMusicVolume(void){return qa_music;}int GetSpeakerMute(void){return qa_mute;}
static bool qa_audio_started=true;
Uint32 SDL_WasInit(Uint32 flags){return qa_audio_started?SDL_INIT_AUDIO:0;}
void SDL_QuitSubSystem(Uint32 flags){assert(!qa_device);qa_audio_started=false;}
int SDL_InitSubSystem(Uint32 flags){assert(!qa_device);qa_audio_started=true;return 0;}
#endif
static char qa_data[1024],qa_peer[18];
void LOG_note(int level,const char *fmt,...){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}
const char *Nxt_text(const char *en,const char *vi){return en;}
int InitializedSettings(void){return 1;}int GetJack(void){return 0;}
int GetAudioSink(void){return qa_sink;}void SetAudioSink(int v){qa_sink=v;}
int GetVolume(void){return qa_volume;}void SetVolume(int v){qa_volume=v;}
void Nxt_profileSave(int s){}void Nxt_profileLoad(int s){}
bool Nxt_save(void){return true;}
int userdata_snpath(const char *s,char *out,size_t n){return snprintf(out,n,"%s/%s",qa_data,s);}
bool Radio_isActive(void){return false;}
int Radio_getAudioSamples(int16_t *p,int n){return 0;}
int src_reset(SRC_STATE *s){return 0;}int src_process(SRC_STATE*s,SRC_DATA*d){return -1;}
int NxtSoundCloud_audioSamples(float*p,int n,int rate){if(qa_source!=2)return 0;for(int i=0;i<n*2;i++)p[i]=.25f;return n;}
int NxtSpotify_audioSamples(float*p,int n,int rate){if(qa_source!=1)return 0;for(int i=0;i<n*2;i++)p[i]=.25f;return n;}
bool NxtSoundCloud_isPlaying(void){return qa_source==2;}bool NxtSpotify_isPlaying(void){return qa_source==1;}
void NxtDSP_process(float*p,int n,float gain){}void NxtDSP_configure(const NxtConfig*c,int rate){}
float NxtDSP_peak(void){return 0;}
void NxtDecoder_cancelStreams(void){}
const char *SDL_GetError(void){return "QA BlueALSA transport not ready";}
Uint32 SDL_GetTicks(void){return qa_ticks;}
int SDL_setenv(const char*n,const char*v,int overwrite){return setenv(n,v,overwrite);}
void SDL_PauseAudioDevice(SDL_AudioDeviceID id,int pause){}
void SDL_CloseAudioDevice(SDL_AudioDeviceID id){assert(qa_device);qa_device=false;}
static int qa_reset(void){assert(!qa_device);qa_resets++;return snd_config_update_free_global();}
static const char *node_text(snd_config_t *tree,const char *path){snd_config_t*n;const char *v;assert(snd_config_search(tree,path,&n)>=0&&snd_config_get_string(n,&v)>=0);return v;}
static void check_alsa(void){
 snd_config_t *tree=NULL;assert(snd_config_update_ref(&tree)>=0);
 assert(!strcmp(node_text(tree,"pcm.nxt_music_output.type"),"plug"));
 if(qa_sink==1){
  assert(!strcmp(node_text(tree,"pcm.nxt_music_output.slave.pcm.type"),"bluealsa"));
  assert(!strcmp(node_text(tree,"pcm.nxt_music_output.slave.pcm.device"),qa_peer));
  assert(!strcmp(node_text(tree,"pcm.nxt_music_output.slave.pcm.profile"),"a2dp"));
  assert(!strcmp(node_text(tree,"ctl.default.device"),qa_peer));
 }else{
  assert(!strcmp(node_text(tree,"pcm.nxt_music_output.slave.pcm"),"nxt_music_mix"));
  assert(!strcmp(node_text(tree,"pcm.nxt_music_mix.type"),"dmix"));
  assert(!strcmp(node_text(tree,"pcm.nxt_music_mix.slave.pcm"),qa_sink==2?"hw:3,0":"hw:0,0"));
#ifdef NXT_ROM_NXREDUX
  long key,rate; snd_config_t *entry;
  assert(snd_config_search(tree,"pcm.nxt_music_mix.ipc_key",&entry)>=0&&snd_config_get_integer(entry,&key)>=0);
  assert(key==(qa_sink==2?0x4e5858:1111));
  assert(snd_config_search(tree,"pcm.nxt_music_mix.slave.rate",&entry)>=0&&snd_config_get_integer(entry,&rate)>=0);
  assert(rate==(qa_sink==2?96000:48000));
  assert(!strcmp(node_text(tree,"pcm.nxt_music_mix.slave.format"),"S16_LE"));
#endif
 }
 snd_config_unref(tree);
}
SDL_AudioDeviceID SDL_OpenAudioDevice(const char *name,int capture,const SDL_AudioSpec *want,SDL_AudioSpec *have,int changes){
 assert(!qa_device&&!name&&!capture&&!changes&&qa_resets>0);
 assert(!strcmp(getenv("AUDIODEV"),"nxt_music_output"));
 assert(!strcmp(getenv("ALSA_CONFIG_PATH"),qa_nextui_config_path));
 qa_opens++;check_alsa();if(qa_fail){qa_fail--;return 0;}
 *have=*want;qa_device=true;return 2;
}
static void property(DBusMessageIter *props,const char *key,int type,void *value,const char *signature){
 DBusMessageIter entry,variant;assert(dbus_message_iter_open_container(props,DBUS_TYPE_DICT_ENTRY,NULL,&entry));
 assert(dbus_message_iter_append_basic(&entry,DBUS_TYPE_STRING,&key));
 assert(dbus_message_iter_open_container(&entry,DBUS_TYPE_VARIANT,signature,&variant));
 assert(dbus_message_iter_append_basic(&variant,type,value));
 assert(dbus_message_iter_close_container(&entry,&variant));assert(dbus_message_iter_close_container(props,&entry));
}
static void device(DBusMessageIter *objects,const char *path,const char *address,bool connected,bool a2dp){
 DBusMessageIter object,interfaces,entry,props,p,variant,uuids;
 const char *iface="org.bluez.Device1",*key="UUIDs",*uuid=a2dp?AUDIO_SINK_UUID:"00001124-0000-1000-8000-00805f9b34fb";
 dbus_bool_t on=connected;
 assert(dbus_message_iter_open_container(objects,DBUS_TYPE_DICT_ENTRY,NULL,&object));
 assert(dbus_message_iter_append_basic(&object,DBUS_TYPE_OBJECT_PATH,&path));
 assert(dbus_message_iter_open_container(&object,DBUS_TYPE_ARRAY,"{sa{sv}}",&interfaces));
 assert(dbus_message_iter_open_container(&interfaces,DBUS_TYPE_DICT_ENTRY,NULL,&entry));
 assert(dbus_message_iter_append_basic(&entry,DBUS_TYPE_STRING,&iface));
 assert(dbus_message_iter_open_container(&entry,DBUS_TYPE_ARRAY,"{sv}",&props));
 property(&props,"Address",DBUS_TYPE_STRING,&address,"s");property(&props,"Connected",DBUS_TYPE_BOOLEAN,&on,"b");
 assert(dbus_message_iter_open_container(&props,DBUS_TYPE_DICT_ENTRY,NULL,&p));assert(dbus_message_iter_append_basic(&p,DBUS_TYPE_STRING,&key));
 assert(dbus_message_iter_open_container(&p,DBUS_TYPE_VARIANT,"as",&variant));assert(dbus_message_iter_open_container(&variant,DBUS_TYPE_ARRAY,"s",&uuids));
 assert(dbus_message_iter_append_basic(&uuids,DBUS_TYPE_STRING,&uuid));
 assert(dbus_message_iter_close_container(&variant,&uuids));assert(dbus_message_iter_close_container(&p,&variant));assert(dbus_message_iter_close_container(&props,&p));
 assert(dbus_message_iter_close_container(&entry,&props));assert(dbus_message_iter_close_container(&interfaces,&entry));
 assert(dbus_message_iter_close_container(&object,&interfaces));assert(dbus_message_iter_close_container(objects,&object));
}
static void discover(bool connected,const char *peer){
 DBusMessage *reply=dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);assert(reply);
 DBusMessageIter root,objects;dbus_message_iter_init_append(reply,&root);
 assert(dbus_message_iter_open_container(&root,DBUS_TYPE_ARRAY,"{oa{sa{sv}}}",&objects));
 device(&objects,"/org/bluez/hci0/dev_10","10:11:12:13:14:15",true,false);
 device(&objects,"/org/bluez/hci0/dev_20",peer,connected,true);
 assert(dbus_message_iter_close_container(&root,&objects));
 audio_sink(reply,"",bt_address);dbus_message_unref(reply);
 if(connected){assert(!strcmp(bt_address,peer));snprintf(qa_peer,sizeof(qa_peer),"%s",peer);}else assert(!bt_address[0]);
}
static void reopen(void){Player_handleAudioSinkChange();}
static void wave_check(void){
 for(qa_source=0;qa_source<3;qa_source++){
  e.read=e.write=0;e.queued=1;e.ring[0].count=32;e.ring[0].used=0;e.ring[0].speed=1;e.state=PLAYER_STATE_PLAYING;
  for(int i=0;i<64;i++)e.ring[0].pcm[i]=.25f;
  float pcm[64];output_audio(NULL,(Uint8*)pcm,sizeof(pcm));for(int i=0;i<64;i++)assert(fabsf(pcm[i]-.25f)<.00001f);
#ifdef NXT_ROM_NXREDUX
  /* All sources observe the live firmware music fader and pure OSD mute. */
  for(int mode=0;mode<2;mode++){
   qa_music=10;qa_mute=mode;
   e.read=e.write=0;e.queued=1;e.ring[0].count=32;e.ring[0].used=0;
   output_audio(NULL,(Uint8*)pcm,sizeof(pcm));
   for(int i=0;i<64;i++)assert(fabsf(pcm[i]-(mode?0.f:.125f))<.00001f);
  }
  qa_music=20;qa_mute=0;
#endif
 }qa_source=0;
}
static void tick(Uint32 now){qa_ticks=now;NxtHardware_tick();reopen();}
int main(int argc,char **argv){
 assert(argc==2);snprintf(qa_data,sizeof(qa_data),"%s",argv[1]);
#ifdef NXT_QA_BASELINE_HARDWARE
 /* The user log has a working PCM, requested Bluetooth, but sink still 0.
  * Reproduce it with the unchanged v1.8.1 selector and a stale speaker file. */
 nxt.output=1;qa_sink=0;e.device=2;
 snprintf(config_path,sizeof(config_path),"%s/.asoundrc",qa_data);
 char previous[8192];assert(readtext(config_path,previous,sizeof(previous))&&strstr(previous,"# NXT Retro Music shared PCM")&&!strstr(previous,"bluealsa"));
 assert(!NxtHardware_select(1));
 assert(nxt.output==1&&GetAudioSink()==0&&route_pending&&!last_route_ok&&Player_outputReady());
 puts("PASS reproduced v1.8.1 user log: requested=1, selected=0, route write failed/pending, old speaker PCM still ready.");
 return 0;
#else
 snprintf(qa_nextui_config_path,sizeof(qa_nextui_config_path),"%s/private.conf",qa_data);
 snprintf(base_config,sizeof(base_config),"%s/base.conf",qa_data);
 nxt.output=0;nxt.auto_bt=1;for(int i=0;i<3;i++)nxt.volume[i]=8;
 e.state=PLAYER_STATE_PLAYING;e.volume=e.speed=1;e.options=nxt;
 tick(100);assert(qa_sink==0&&Player_outputReady());int initial=qa_resets;
 nxt.output=1;route_pending=true;last_route_ok=false;
 assert(qa_sink==0&&Player_outputReady());
 discover(true,"AA:BB:CC:DD:EE:FF");tick(700);
 assert(qa_sink==1&&nxt.output==1&&Player_isBluetoothActive()&&qa_resets==initial+1);
 assert(last_route_ok&&!route_pending);
 assert(!strcmp(NxtHardware_output(),"Bluetooth"));wave_check();
 assert(NxtHardware_select(0));reopen();assert(qa_sink==0&&nxt.output==0&&!Player_isBluetoothActive());
 assert(NxtHardware_select(1));reopen();assert(qa_sink==1&&nxt.output==1&&Player_isBluetoothActive());wave_check();
 /* A stale firmware settings update must not silently switch app output. */
 qa_sink=0;tick(1300);assert(qa_sink==1&&nxt.output==1&&Player_isBluetoothActive());
 discover(true,"11:22:33:44:55:66");int before=qa_resets;tick(1900);assert(qa_sink==1&&qa_resets==before+1);wave_check();
 before=qa_opens;tick(2500);tick(3100);assert(qa_opens==before);
 /* A delayed A2DP transport must retry without another connect event. */
 qa_fail=2;Player_requestOutputRefresh();reopen();assert(!Player_outputReady()&&Player_outputRetryPending()&&*Player_lastOutputError());
 qa_ticks+=301;reopen();assert(Player_outputReady()&&!Player_outputRetryPending()&&!*Player_lastOutputError());wave_check();
 assert(NxtHardware_select(2));reopen();assert(qa_sink==2&&Player_isUSBDACActive()&&!Player_isBluetoothActive());wave_check();
 assert(NxtHardware_select(1));reopen();assert(qa_sink==1&&Player_isBluetoothActive()&&!Player_isUSBDACActive());
 discover(false,"11:22:33:44:55:66");tick(4000);assert(qa_sink==0&&nxt.output==0&&!Player_isBluetoothActive());
 SDL_CloseAudioDevice(e.device);e.device=0;NxtHardware_release();snd_config_update_free_global();
 puts("PASS NextUI production: connected A2DP detection ignores gamepad; private ALSA route parses without merge conflicts; speaker/Bluetooth/DAC and headset changes reopen named PCM after closing/resetting cache; stale firmware sink is corrected; transport retry recovers; local, Spotify and SoundCloud PCM remains nonzero; idle does not reopen audio. No physical headset test.");
#endif
}

#include "v184_bit_stub.inc"
