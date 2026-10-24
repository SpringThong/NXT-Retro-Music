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
#include "nxt_stock.h"
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
#include <errno.h>
static bool enabled,visible=true;static int server=-1,lockfd=-1;
static char socket_path[100],status_path[128];
static bool attach;
static bool background_choice;
static void poll_client(void) {
    if(server<0)return;int client=accept(server,NULL,NULL);if(client<0)return;
    struct timeval timeout={0,200000};setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    char request[128]={0};int n=read(client,request,sizeof(request)-1);
    /* A legacy server does not understand OPEN and cannot attach to it.
     * The server checks the build key atomically with accepting the attach. */
    if(n>0&&!strncmp(request,"OPEN ",5)){
        if(!strcmp(request,"OPEN " NXT_SESSION_BUILD_KEY "\n")){attach=true;send(client,"OK\n",3,MSG_NOSIGNAL);}
        else send(client,"UPGRADE\n",8,MSG_NOSIGNAL);
    }
    else if(n>0&&!strncmp(request,"ATTACH",6)){attach=true;send(client,"OK\n",3,MSG_NOSIGNAL);}
    else if(n>0&&!strncmp(request,"STATUS",6)){char state[1200];int length=snprintf(state,sizeof(state),"%s pid=%ld playback=%d position_ms=%d codec=%s track=%s\n",visible?"VISIBLE":"HIDDEN",(long)getpid(),NxtPlayback_state(),NxtPlayback_position(),NxtPlayback_info()->codec,NxtPlayback_path());if(length>0)send(client,state,(size_t)length<sizeof(state)?(size_t)length:sizeof(state)-1,MSG_NOSIGNAL);}
    else send(client,visible?"VISIBLE\n":"HIDDEN\n",visible?8:7,MSG_NOSIGNAL);close(client);
}
/* A helper from a dead frontend may have inherited its listening socket.
 * Recover only a dead peer owned by this user, with a free app lock. Never
 * signal a PID from a state file or terminate a live music session. */
static bool recover_orphan(pid_t peer,uid_t owner){
 if(peer<=1||owner!=getuid()||kill(peer,0)==0||errno!=ESRCH)return false;
 char path[128];snprintf(path,sizeof(path),"%s.lock",socket_path);int fd=open(path,O_RDWR|O_CLOEXEC|O_NOFOLLOW);if(fd<0)return false;
 struct stat st;bool ok=!fstat(fd,&st)&&st.st_uid==getuid()&&S_ISREG(st.st_mode)&&!flock(fd,LOCK_EX|LOCK_NB);
 if(ok){unlink(socket_path);unlink(status_path);fprintf(stderr,"NXT: recovered orphan endpoint; starting a fresh session.\n");flock(fd,LOCK_UN);}close(fd);return ok;
}
int NxtSession_start(int argc,char **argv) {
    const char *custom=getenv("NXT_SESSION_SOCKET");
    /* Separate branches keep varargs types exact. */
    if(custom)snprintf(socket_path,sizeof(socket_path),"%s",custom);else snprintf(socket_path,sizeof(socket_path),"/tmp/nxt-retro-music-%lu.sock",(unsigned long)getuid());
    snprintf(status_path,sizeof(status_path),"%s.state",socket_path);
    bool client_mode=false,current_only=false;for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--session"))enabled=true;if(!strcmp(argv[i],"--attach"))client_mode=true;if(!strcmp(argv[i],"--attach-current"))client_mode=current_only=true;}
    if(client_mode){
        int fd=socket(AF_UNIX,SOCK_STREAM,0);struct sockaddr_un address={.sun_family=AF_UNIX};snprintf(address.sun_path,sizeof(address.sun_path),"%s",socket_path);
        if(fd<0)return -2;
        if(connect(fd,(struct sockaddr*)&address,sizeof(address))<0){int error=errno;close(fd);return error==ENOENT||error==ECONNREFUSED?-1:-2;}
        struct timeval timeout={3,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
        const char *request=current_only?"OPEN " NXT_SESSION_BUILD_KEY "\n":"ATTACH\n";
        size_t length=strlen(request);char reply[16]={0};ssize_t n=-1;
        if(send(fd,request,length,MSG_NOSIGNAL)==(ssize_t)length)n=read(fd,reply,sizeof(reply)-1);
        struct ucred peer={0};socklen_t peer_size=sizeof(peer);bool known=!getsockopt(fd,SOL_SOCKET,SO_PEERCRED,&peer,&peer_size);
        close(fd);if(n<=0&&known&&recover_orphan(peer.pid,peer.uid))return -1;
        if(n==3&&!memcmp(reply,"OK\n",3))return 1;
        fprintf(stderr,"NXT: background frontend is older or unavailable; reboot before opening %s.\n",NXT_BUILD_REVISION);return -2;
    }
    if(!enabled)return 0;
    char lockpath[128];snprintf(lockpath,sizeof(lockpath),"%s.lock",socket_path);lockfd=open(lockpath,O_CREAT|O_RDWR|O_CLOEXEC,0600);
    if(lockfd<0||flock(lockfd,LOCK_EX|LOCK_NB)<0){perror("NXT session lock");return -1;}
    unlink(socket_path);server=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0);struct sockaddr_un address={.sun_family=AF_UNIX};snprintf(address.sun_path,sizeof(address.sun_path),"%s",socket_path);
    if(server<0||bind(server,(struct sockaddr*)&address,sizeof(address))<0||listen(server,4)<0){perror("NXT session socket");return -1;}
    chmod(socket_path,0600);fcntl(server,F_SETFL,O_NONBLOCK);fprintf(stderr,"NXT frontend: %s\n",NXT_BUILD_REVISION);NxtSession_status(true);return 0;
}
void NxtSession_status(bool shown) {
    visible=shown;NxtStock_visible(shown);if(!enabled)return;char temp[160];snprintf(temp,sizeof(temp),"%s.tmp",status_path);FILE *f=fopen(temp,"w");if(f){fprintf(f,"%s %ld\n",shown?"VISIBLE":"HIDDEN",(long)getpid());fclose(f);rename(temp,status_path);}if(shown)poll_client();
}
bool NxtSession_wantsBackground(void){return background_choice||nxt.background;}
void NxtSession_exitPrompt(DisplayContext *display){
 bool audible=NxtSpotify_isPlaying()||NxtSoundCloud_isPlaying()||Player_getState()==PLAYER_STATE_PLAYING||Radio_getState()==RADIO_STATE_PLAYING;
 background_choice=false;
 if(!audible){ModuleCommon_signalStop();ModuleCommon_requestQuit();return;}
 int selected=0;const char *rows[]={Nxt_text("Keep playing in background","Tiếp tục nghe nhạc dưới nền"),Nxt_text("Stop music and close app","Dừng nhạc và thoát hẳn ứng dụng")};
 for(;;){NxtPick p=Nxt_pick(display,Nxt_text("Leave music player?","Thoát trình nghe nhạc?"),rows,2,&selected,Nxt_text("A Choose · B Close app","A Chọn · B Thoát hẳn"));
  if(p.action==1&&selected==0){background_choice=true;ModuleCommon_requestQuit();return;}
  if((p.action==1&&selected==1)||p.action==2||p.action==9){ModuleCommon_signalStop();ModuleCommon_requestQuit();return;}
 }
}
bool NxtSession_background(DisplayContext *display) {
    if(!enabled||!NxtSession_wantsBackground()||ModuleCommon_stopSignalled())return false;
    background_choice=false;
    if(Background_getActive()==BG_NONE){if(NxtSoundCloud_active())Background_setActive(BG_SOUNDCLOUD);else if(NxtSpotify_active())Background_setActive(BG_SPOTIFY);else if(PlayerModule_isActive())Background_setActive(BG_MUSIC);else if(Radio_isActive())Background_setActive(BG_RADIO);else if(Podcast_isActive())Background_setActive(BG_PODCAST);}
    if(!NxtSoundCloud_active()&&!NxtSpotify_active()&&!PlayerModule_isActive()&&!Radio_isActive()&&!Podcast_isActive())return false;
    if(!NxtSoundCloud_active()&&!NxtSpotify_active())Resume_updatePosition(Player_getPosition());MusicPlaying_leave();Spectrum_quit();DisplayHelper_prepareForExternal();PAD_quit();
    attach=false;NxtSession_status(false);
    while(!ModuleCommon_stopSignalled()){
        poll_client();Nxt_tick();
        if(ModuleCommon_stopSignalled())break;
        Background_tick();
        if(attach){NxtStock_visible(true);DisplayHelper_recoverDisplay();PAD_init();PAD_reset();ModuleCommon_init();ModuleCommon_setAutosleepDisabled(true);NxtSession_status(true);return true;}
        if(!NxtSoundCloud_active()&&!NxtSpotify_active()&&!PlayerModule_isActive()&&!Radio_isActive()&&!Podcast_isActive())break;
        usleep(20000);
    }
    /* Stay detached on EOF/timer/shutdown. Never take the game's framebuffer. */
    return false;
}
void NxtSession_tick(void){if(enabled&&visible)poll_client();}
bool NxtSession_isHidden(void){return enabled&&!visible;}
void NxtSession_quit(void){if(server>=0)close(server);if(enabled){unlink(socket_path);unlink(status_path);}if(lockfd>=0)close(lockfd);}
