/* Actual timer/config/session C, with explicit clocks and playback fixtures. */
#include "nxt.h"
#include "player.h"
#include "background.h"
#include "radio.h"
#include "podcast.h"
#include "nxt_spotify.h"
#include "nxt_soundcloud.h"
#include "file_utils.h"
#include "parson/parson.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
/* This build host prohibits all sockets. Keep the real session state machine
 * and cleanup, with explicit transport fixtures instead of kernel sockets. */
static int qa_socket(int domain,int type,int protocol){return open("/dev/null",O_RDWR);}
static int qa_bind(int fd,const struct sockaddr *address,socklen_t size){
 const char *path=getenv("NXT_SESSION_SOCKET");assert(path&&path[0]=='/');
 int file=open(path,O_CREAT|O_WRONLY,0600);assert(file>=0);close(file);return 0;
}
static int qa_listen(int fd,int backlog){return 0;}
static int qa_accept(int fd,struct sockaddr *address,socklen_t *size){errno=EAGAIN;return -1;}
#define socket qa_socket
#define bind qa_bind
#define listen qa_listen
#define accept qa_accept
#include "../upstream/src/nxt_session.c"
#undef socket
#undef bind
#undef listen
#undef accept
static Uint32 ticks;
static bool stopping,quit_requested,online,radio_on,podcast_on,display_off;
static PlayerState player_state=PLAYER_STATE_PLAYING;
static BackgroundPlayerType bg;
static int saved_position,personal_flushes,preferences_saved,radio_stops,podcast_stops;
static int auto_ticks,hardware_ticks;
Uint32 SDL_GetTicks(void){return ticks;}
void ModuleCommon_signalStop(void){stopping=true;}
bool ModuleCommon_stopSignalled(void){return stopping;}
void ModuleCommon_requestQuit(void){quit_requested=true;}
void ModuleCommon_setAutosleepDisabled(bool disabled){}
void ModuleCommon_init(void){}
void NxtPersonal_flush(void){personal_flushes++;}
void NxtPersonal_savePreferences(void){preferences_saved++;}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void NxtPersonal_tick(void){}
void NxtSpotify_tick(void){}void NxtSoundCloud_tick(void){}
void NxtSpotify_quit(void){online=false;}void NxtSoundCloud_quit(void){}
bool NxtSpotify_active(void){return online;}bool NxtSoundCloud_active(void){return false;}
bool NxtSpotify_hasMetadata(void){return online;}bool NxtSoundCloud_hasMetadata(void){return false;}
bool NxtSpotify_isPlaying(void){return online;}bool NxtSoundCloud_isPlaying(void){return false;}
int NxtSpotify_position(void){return 123456;}int NxtSoundCloud_position(void){return 0;}
const char *NxtSpotify_uri(void){return "spotify:track:fixture";}const char *NxtSoundCloud_uri(void){return "";}
static const TrackInfo track={.duration_ms=180000};
const TrackInfo *NxtSpotify_info(void){return &track;}const TrackInfo *NxtSoundCloud_info(void){return &track;}
const TrackInfo *Player_getTrackInfo(void){return &track;}const char *Player_getCurrentFile(void){return "/Music/test.flac";}
void NxtMonitor_tick(void){}void NxtOTA_tick(void){}
void NxtLog_step(const char*c,const char*s,bool ok,const char*fmt,...){}
void Player_update(void){}void PlayerModule_syncQueue(void){}
PlayerState Player_getState(void){return player_state;}
int Player_getPosition(void){return 123456;}
int Player_getOutputRate(void){return 48000;}
void Player_refreshOptions(void){}
void Player_stop(void){player_state=PLAYER_STATE_STOPPED;}
void PlayerModule_markStopped(void){}
bool PlayerModule_isActive(void){return player_state!=PLAYER_STATE_STOPPED;}
void PlayerModule_backgroundTick(void){auto_ticks++;}
void Player_handleAudioSinkChange(void){}
void Resume_updatePosition(int position){saved_position=position;}
bool Radio_isActive(void){return radio_on;}
void Radio_stop(void){radio_on=false;radio_stops++;}
bool Podcast_isActive(void){return podcast_on;}
void Podcast_stop(void){podcast_on=false;podcast_stops++;}
void Podcast_flushProgress(void){}
BackgroundPlayerType Background_getActive(void){return bg;}
void Background_setActive(BackgroundPlayerType v){bg=v;}
void Background_stopAll(void){bg=BG_NONE;}
void Background_tick(void){auto_ticks++;}
void NxtHardware_tick(void){hardware_ticks++;}
void NxtDSP_configure(const NxtConfig*c,int rate){}
void Nxt_profileSave(int sink){}
int PWR_getDisplayOff(void){return display_off;}
void MusicPlaying_leave(void){}void Spectrum_quit(void){}
void DisplayHelper_prepareForExternal(void){}void DisplayHelper_recoverDisplay(void){}
void PLAT_initInput(void){}void PLAT_quitInput(void){}void PAD_reset(void){}
#ifdef NXT_ROM_STOCK
void NxtStock_tick(void){}void NxtStock_visible(bool v){}
bool NxtStock_panelActive(void){return false;}
USBHIDEvent Player_pollUSBHID(void){return USB_HID_EVENT_NONE;}
bool ModuleCommon_handleHIDVolume(USBHIDEvent event){return false;}
#endif
static void reset(void){
 stopping=quit_requested=online=radio_on=podcast_on=display_off=false;
 bg=BG_NONE;player_state=PLAYER_STATE_PLAYING;saved_position=0;auto_ticks=hardware_ticks=0;
 memset(&nxt,0,sizeof(nxt));nxt.background=1;nxt.interface_style=17;nxt.led_rgb=0x123456;
 Nxt_sleepTimer(0);
}
static void expire(void){
 int before=personal_flushes;Nxt_tick();assert(Nxt_timerRemaining()==0&&personal_flushes==before+1);
#ifdef NXT_ROM_NXREDUX
 assert(stopping&&quit_requested&&auto_ticks==0&&hardware_ticks==0);
 char path[1024];userdata_snpath("nxt.json",path,sizeof(path));JSON_Value*v=json_parse_file(path);assert(v);
 JSON_Object *o=json_value_get_object(v);assert(json_object_get_number(o,"background")==1&&json_object_get_number(o,"interface_style")==17&&json_object_get_number(o,"led_rgb")==0x123456);json_value_free(v);
#else
 assert(!stopping&&!quit_requested);
#endif
 Nxt_tick();assert(personal_flushes==before+1); /* exactly once */
}
int main(void){
 reset();ticks=100;Nxt_sleepTimer(1);ticks=60099;Nxt_tick();assert(!stopping&&Nxt_timerRemaining()==1);
 auto_ticks=hardware_ticks=0;ticks=60100;expire();assert(saved_position==123456&&player_state==PLAYER_STATE_STOPPED);
 reset();ticks=100;Nxt_sleepTimer(1);Nxt_sleepTimer(0);ticks=100000;Nxt_tick();assert(!stopping&&player_state==PLAYER_STATE_PLAYING);
 reset();ticks=100;Nxt_sleepTimer(1);ticks=500;Nxt_sleepTimer(2);ticks=60100;Nxt_tick();assert(!stopping&&Nxt_timerRemaining()>0);
 reset();ticks=UINT32_MAX-1000;Nxt_sleepTimer(1);ticks+=60000;expire();assert(saved_position==123456);
 reset();ticks=100;Nxt_sleepTimer(1);player_state=PLAYER_STATE_PAUSED;display_off=true;ticks+=60000;expire();assert(saved_position==123456);
 reset();ticks=100;Nxt_sleepTimer(1);radio_on=true;bg=BG_RADIO;ticks+=60000;expire();assert(radio_stops>0&&!radio_on);
 reset();ticks=100;Nxt_sleepTimer(1);podcast_on=true;bg=BG_PODCAST;ticks+=60000;expire();assert(podcast_stops>0&&!podcast_on);
#ifdef NXT_ROM_NXREDUX
 reset();ticks=100;Nxt_sleepTimer(1);online=true;ticks+=60000;
 char *args[]={"timer-test","--session",NULL};assert(NxtSession_start(2,args)==0);
 assert(!NxtSession_background(NULL));assert(stopping&&quit_requested&&!online&&NxtSession_isHidden()&&auto_ticks==0);
 NxtSession_quit();const char *s=getenv("NXT_SESSION_SOCKET");assert(s&&access(s,F_OK)<0);
 char status[128];snprintf(status,sizeof(status),"%s.state",s);assert(access(status,F_OK)<0);
 assert(preferences_saved>=5);
#endif
 puts("PASS timer: deadline, cancel, replace, wrap, pause/screen-off, radio/podcast; NX-only saved full exit and actual background/session cleanup; Stock/NextUI stay open.");
 return 0;
}
