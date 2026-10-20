/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "nxt_playback.h"
#include "api.h"
#include "display_helper.h"
#include "background.h"
#include "module_player.h"
#include "player.h"
#include "radio.h"
#include "podcast.h"
#include "resume.h"
#include "ui_music_playing.h"
#include "spectrum.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static bool enabled,visible=true;static int server=-1,lockfd=-1;
static char socket_path[100],status_path[128];
static bool attach;
static void poll_client(void) {
    if(server<0)return;int client=accept(server,NULL,NULL);if(client<0)return;
    struct timeval timeout={0,200000};setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    char request[32]={0};int n=read(client,request,sizeof(request)-1);
    if(n>0&&!strncmp(request,"ATTACH",6)){attach=true;send(client,"OK\n",3,MSG_NOSIGNAL);}
    else if(n>0&&!strncmp(request,"STATUS",6)){char state[1200];int length=snprintf(state,sizeof(state),"%s pid=%ld playback=%d position_ms=%d codec=%s track=%s\n",visible?"VISIBLE":"HIDDEN",(long)getpid(),NxtPlayback_state(),NxtPlayback_position(),NxtPlayback_info()->codec,NxtPlayback_path());send(client,state,length,MSG_NOSIGNAL);}
    else send(client,visible?"VISIBLE\n":"HIDDEN\n",visible?8:7,MSG_NOSIGNAL);close(client);
}
int NxtSession_start(int argc,char **argv) {
    const char *custom=getenv("NXT_SESSION_SOCKET");
    /* Separate branches keep varargs types exact. */
    if(custom)snprintf(socket_path,sizeof(socket_path),"%s",custom);else snprintf(socket_path,sizeof(socket_path),"/tmp/nxt-retro-music-%lu.sock",(unsigned long)getuid());
    snprintf(status_path,sizeof(status_path),"%s.state",socket_path);
    bool client_mode=false;for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--session"))enabled=true;if(!strcmp(argv[i],"--attach"))client_mode=true;}
    if(client_mode){int fd=socket(AF_UNIX,SOCK_STREAM,0);struct sockaddr_un address={.sun_family=AF_UNIX};snprintf(address.sun_path,sizeof(address.sun_path),"%s",socket_path);if(fd<0||connect(fd,(struct sockaddr*)&address,sizeof(address))<0){if(fd>=0)close(fd);return -1;}struct timeval timeout={3,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));send(fd,"ATTACH\n",7,MSG_NOSIGNAL);char reply[8];int n=read(fd,reply,sizeof(reply));close(fd);return n>0?1:-1;}
    if(!enabled)return 0;
    char lockpath[128];snprintf(lockpath,sizeof(lockpath),"%s.lock",socket_path);lockfd=open(lockpath,O_CREAT|O_RDWR|O_CLOEXEC,0600);
    if(lockfd<0||flock(lockfd,LOCK_EX|LOCK_NB)<0){perror("NXT session lock");return -1;}
    unlink(socket_path);server=socket(AF_UNIX,SOCK_STREAM,0);struct sockaddr_un address={.sun_family=AF_UNIX};snprintf(address.sun_path,sizeof(address.sun_path),"%s",socket_path);
    if(server<0||bind(server,(struct sockaddr*)&address,sizeof(address))<0||listen(server,4)<0){perror("NXT session socket");return -1;}
    chmod(socket_path,0600);fcntl(server,F_SETFL,O_NONBLOCK);NxtSession_status(true);return 0;
}
void NxtSession_status(bool shown) {
    visible=shown;if(!enabled)return;char temp[160];snprintf(temp,sizeof(temp),"%s.tmp",status_path);FILE *f=fopen(temp,"w");if(f){fprintf(f,"%s %ld\n",shown?"VISIBLE":"HIDDEN",(long)getpid());fclose(f);rename(temp,status_path);}if(shown)poll_client();
}
bool NxtSession_background(DisplayContext *display) {
    if(!enabled||!nxt.background||ModuleCommon_stopSignalled())return false;
    if(Background_getActive()==BG_NONE){if(NxtSoundCloud_active())Background_setActive(BG_SOUNDCLOUD);else if(NxtSpotify_active())Background_setActive(BG_SPOTIFY);else if(PlayerModule_isActive())Background_setActive(BG_MUSIC);else if(Radio_isActive())Background_setActive(BG_RADIO);else if(Podcast_isActive())Background_setActive(BG_PODCAST);}
    if(!NxtSoundCloud_active()&&!NxtSpotify_active()&&!PlayerModule_isActive()&&!Radio_isActive()&&!Podcast_isActive())return false;
    if(!NxtSoundCloud_active()&&!NxtSpotify_active())Resume_updatePosition(Player_getPosition());MusicPlaying_leave();Spectrum_quit();DisplayHelper_prepareForExternal();PAD_quit();
    attach=false;NxtSession_status(false);
    while(!ModuleCommon_stopSignalled()){
        poll_client();Nxt_tick();Background_tick();
        if(attach){DisplayHelper_recoverDisplay();PAD_init();PAD_reset();ModuleCommon_init();ModuleCommon_setAutosleepDisabled(true);NxtSession_status(true);return true;}
        if(!NxtSoundCloud_active()&&!NxtSpotify_active()&&!PlayerModule_isActive()&&!Radio_isActive()&&!Podcast_isActive())break;
        usleep(20000);
    }
    /* Stay detached on EOF/timer/shutdown. Never take the game's framebuffer. */
    return false;
}
void NxtSession_tick(void){if(enabled&&visible)poll_client();}
bool NxtSession_isHidden(void){return enabled&&!visible;}
void NxtSession_quit(void){if(server>=0)close(server);if(enabled){unlink(socket_path);unlink(status_path);}if(lockfd>=0)close(lockfd);}
