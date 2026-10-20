/* SPDX-License-Identifier: GPL-3.0-only
 * Production session client/server with file transport, audio and UI fixtures.
 * Managed runtime denies sockets; no kernel IPC or physical device claim. */
#include "nxt.h"
#ifdef NXT_FIXTURE_BUILD_KEY
#undef NXT_SESSION_BUILD_KEY
#define NXT_SESSION_BUILD_KEY NXT_FIXTURE_BUILD_KEY
#endif
#include "nxt_playback.h"
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
static bool fixture_server,direct;
static int direct_fd=-1;
static char direct_reply[128];
static int fixture_socket(int family,int type,int protocol){
    (void)family;(void)type;(void)protocol;
    char path[]="/tmp/nxt-session-reply-XXXXXX";int fd=mkstemp(path);unlink(path);return fd;
}
static int fixture_connect(int fd,const struct sockaddr *address,socklen_t size){
    (void)fd;(void)size;
    const char *path=((const struct sockaddr_un *)address)->sun_path;
    FILE *f=fopen(path,"r");if(!f){errno=ENOENT;return -1;}
    long pid=0;int n=fscanf(f,"%ld",&pid);fclose(f);
    if(n!=1||pid<=0){errno=ECONNREFUSED;return -1;}return 0;
}
static int fixture_accept(int fd,struct sockaddr *address,socklen_t *size){
    (void)fd;(void)address;(void)size;
    if(direct){int result=direct_fd;direct_fd=-1;return result;}
    char path[180];snprintf(path,sizeof(path),"%s.request",getenv("NXT_SESSION_SOCKET"));
    int result=open(path,O_RDONLY|O_CLOEXEC);if(result>=0)unlink(path);return result;
}
static ssize_t fixture_send(int fd,const void *buffer,size_t length,int flags){
    (void)flags;
    if(direct){size_t n=length<sizeof(direct_reply)-1?length:sizeof(direct_reply)-1;memcpy(direct_reply,buffer,n);direct_reply[n]=0;return (ssize_t)length;}
    char temp[180],target[180];const char *endpoint=getenv("NXT_SESSION_SOCKET");
    snprintf(temp,sizeof(temp),"%s.%s.tmp",endpoint,fixture_server?"reply":"request");
    snprintf(target,sizeof(target),"%s.%s",endpoint,fixture_server?"reply":"request");
    char response_path[180];snprintf(response_path,sizeof(response_path),"%s.reply",endpoint);if(!fixture_server)unlink(response_path);
    FILE *f=fopen(temp,"wb");if(!f)return -1;size_t count=fwrite(buffer,1,length,f);fclose(f);if(count!=length||rename(temp,target))return -1;
    if(fixture_server)return (ssize_t)length;
    for(int attempt=0;attempt<300;attempt++){
        f=fopen(response_path,"rb");if(f){char reply[1200];size_t n=fread(reply,1,sizeof(reply),f);fclose(f);unlink(response_path);ftruncate(fd,0);lseek(fd,0,SEEK_SET);if(write(fd,reply,n)!=(ssize_t)n)return -1;lseek(fd,0,SEEK_SET);return (ssize_t)length;}
        usleep(10000);
    }
    return -1;
}
#define socket fixture_socket
#define connect fixture_connect
#define accept fixture_accept
#define send fixture_send
#include NXT_SESSION_SOURCE
#undef socket
#undef connect
#undef accept
#undef send

void NxtStock_visible(bool shown){(void)shown;}
static volatile sig_atomic_t stopped;
static const TrackInfo info={.codec="QA",.title="Session fixture"};
bool NxtSoundCloud_hasMetadata(void){return false;}
bool NxtSoundCloud_active(void){return false;}
bool NxtSoundCloud_isPlaying(void){return false;}
const TrackInfo *NxtSoundCloud_info(void){return &info;}
int NxtSoundCloud_position(void){return 0;}
const char *NxtSoundCloud_uri(void){return "";}
bool NxtSpotify_hasMetadata(void){return false;}
bool NxtSpotify_active(void){return false;}
bool NxtSpotify_isPlaying(void){return false;}
const TrackInfo *NxtSpotify_info(void){return &info;}
int NxtSpotify_position(void){return 0;}
const char *NxtSpotify_uri(void){return "";}
PlayerState Player_getState(void){return PLAYER_STATE_PLAYING;}
int Player_getPosition(void){return 12345;}
const TrackInfo *Player_getTrackInfo(void){return &info;}
const char *Player_getCurrentFile(void){return "fixture://same-track";}

bool NxtStartup_preview(void){return false;}
const TrackInfo *NxtStartup_info(void){return &info;}
int NxtStartup_position(void){return 0;}
const char *NxtStartup_path(void){return "";}
static long clock_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000+t.tv_nsec/1000000;}
static void event(const char *name){
    const char *path=getenv("NXT_QA_EVENTS");if(!path)return;
    FILE *f=fopen(path,"a");if(f){fprintf(f,"%s %s %ld\n",NXT_FIXTURE_TAG,name,(long)getpid());fclose(f);}
}
static void terminate(int signal_number){(void)signal_number;stopped=1;}
int main(int argc,char **argv){
    #ifdef NXT_FIXTURE_HAS_ORPHAN
    if(argc>1&&!strcmp(argv[1],"--qa-orphan")){
     const char *path=getenv("NXT_SESSION_SOCKET");snprintf(socket_path,sizeof(socket_path),"%s",path);snprintf(status_path,sizeof(status_path),"%s.state",path);char lp[128];snprintf(lp,sizeof(lp),"%s.lock",path);
     FILE *f=fopen(path,"w");assert(f);fclose(f);f=fopen(status_path,"w");assert(f);fclose(f);
     int held=open(lp,O_CREAT|O_RDWR|O_CLOEXEC,0600);assert(held>=0&&!flock(held,LOCK_EX|LOCK_NB));
     pid_t dead=fork();assert(dead>=0);if(!dead)_exit(0);assert(waitpid(dead,NULL,0)==dead);
     assert(!recover_orphan(dead,getuid())&&!access(path,F_OK));close(held);
     assert(!recover_orphan(getpid(),getuid())&&!access(path,F_OK));assert(!recover_orphan(dead,getuid()+1)&&!access(path,F_OK));
     assert(recover_orphan(dead,getuid())&&access(path,F_OK)&&access(status_path,F_OK));puts("PASS orphan recovery: dead same-UID peer/free lock only; live and wrong-UID peers or held locks untouched.");return 0;
    }
    #endif
    if(argc>1&&!strcmp(argv[1],"--qa-build-key")){puts(NXT_SESSION_BUILD_KEY);return 0;}
    bool fixture_session=false;
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--build-info")){printf("%s\n%s\n",NXT_APP_NAME,NXT_BUILD_REVISION);return 0;}
        if(!strcmp(argv[i],"--session"))fixture_session=true;
        if(!strcmp(argv[i],"--qa-poll")&&i+1<argc){
            direct=fixture_server=true;direct_fd=fixture_socket(0,0,0);write(direct_fd,argv[i+1],strlen(argv[i+1]));lseek(direct_fd,0,SEEK_SET);server=1;poll_client();printf("%s",direct_reply);fprintf(stderr,"ATTACHED=%d\n",attach);return 0;
        }
    }
    if(!fixture_session){int result=NxtSession_start(argc,argv);return result>0?0:result==-2?2:1;}
    fixture_server=true;enabled=true;server=open("/dev/null",O_RDONLY);
    snprintf(socket_path,sizeof(socket_path),"%s",getenv("NXT_SESSION_SOCKET"));snprintf(status_path,sizeof(status_path),"%s.state",socket_path);
    FILE *endpoint=fopen(socket_path,"w");if(!endpoint)return 1;fprintf(endpoint,"%ld\n",(long)getpid());fclose(endpoint);NxtSession_status(true);
    signal(SIGTERM,terminate);signal(SIGINT,terminate);
    event("start");long hide_at=clock_ms()+180;
    while(!stopped){
        poll_client();
        if(attach){attach=false;event("attach");NxtSession_status(true);hide_at=clock_ms()+180;}
        if(visible&&clock_ms()>=hide_at){NxtSession_status(false);event("hide");}
        usleep(10000);
    }
    event("stop");NxtSession_quit();event("exit");return 0;
}
