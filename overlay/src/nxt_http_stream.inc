/* SPDX-License-Identifier: GPL-3.0-only
 * Curl streams public Google Drive bytes into a bounded pipe. No audio file is
 * written. HTTP Range backs FFmpeg seeking, including M4A moov atoms at EOF. */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define HEADER_LIMIT 32768
#define READ_TIMEOUT_MS 15000
typedef struct NetStream NetStream;
struct NetStream {const bool *stop;const unsigned *reader_epoch;
    NxtDriveEntry entry;
    char url[768];
    pid_t child;
    int body,headers;
    int64_t position,size;
    bool seekable,html;
    unsigned epoch;
};
static bool cancelled(const NetStream *s){return (s->stop&&__atomic_load_n(s->stop,__ATOMIC_RELAXED))||(s->reader_epoch&&*s->reader_epoch!=NxtStream_epoch());}
static void NetStream_close(NetStream *s);
static int64_t millis(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return(int64_t)t.tv_sec*1000+t.tv_nsec/1000000;}
static void end_child(NetStream *s){
    if(s->body>=0){close(s->body);s->body=-1;}
    if(s->headers>=0){close(s->headers);s->headers=-1;}
    if(s->child<=0)return;
    kill(s->child,SIGTERM);int status;
    for(int i=0;i<20;i++){pid_t p=waitpid(s->child,&status,WNOHANG);if(p==s->child||(p<0&&errno!=EINTR)){s->child=0;return;}usleep(10000);}
    kill(s->child,SIGKILL);while(waitpid(s->child,&status,0)<0&&errno==EINTR){}s->child=0;
}
static bool decimal(const char *p,int64_t *value){
    if(*p<'0'||*p>'9')return false;uint64_t n=0;
    while(*p>='0'&&*p<='9'){if(n>((uint64_t)INT64_MAX-(*p-'0'))/10)return false;n=n*10+*p++-'0';}
    *value=(int64_t)n;return true;
}
/* Only the last complete response is relevant (proxy CONNECT and redirects
 * also write headers). Reject errors, misaligned ranges and malformed sizes. */
static int parse_headers(NetStream *s,char *headers,int64_t offset){
    int status=0;int64_t length=-1,total=-1,start=-1;bool html=false,ranges=false,complete=false;
    char *save=NULL;
    for(char *line=strtok_r(headers,"\n",&save);line;line=strtok_r(NULL,"\n",&save)){
        size_t n=strlen(line);if(n&&line[n-1]=='\r')line[--n]=0;
        if(!strncmp(line,"HTTP/",5)){
            char *space=strchr(line,' ');status=space?atoi(space+1):0;
            length=total=start=-1;html=ranges=complete=false;
        }else if(!*line){complete=true;}
        else if(!strncasecmp(line,"Content-Length:",15)){
            const char *p=line+15;while(*p==' ')p++;if(!decimal(p,&length))return-EIO;
        }else if(!strncasecmp(line,"Content-Range:",14)){
            long long a,b,t;if(sscanf(line+14," bytes %lld-%lld/%lld",&a,&b,&t)!=3||a<0||b<a||t<=b)return-EIO;
            start=a;total=t;ranges=true;
        }else if(!strncasecmp(line,"Accept-Ranges:",14)&&strcasestr(line+14,"bytes"))ranges=true;
        else if(!strncasecmp(line,"Content-Type:",13)&&(strcasestr(line+13,"text/html")||strcasestr(line+13,"application/xhtml")))html=true;
    }
    if(!complete||!(status==200||status==206))return-EIO;
    if(offset>0&&(status!=206||start!=offset))return-ESPIPE;
    if(status==206&&start!=offset)return-EIO;
    s->size=total>=0?total:status==200?length:-1;s->seekable=ranges&&s->size>=0;s->html=html;
    return 0;
}
static int connect_at(NetStream *s,int64_t offset){
    if(cancelled(s))return-ECANCELED;
    end_child(s);int body[2],headers[2];
    if(pipe2(body,O_CLOEXEC))return-errno;
    if(pipe2(headers,O_CLOEXEC)){close(body[0]);close(body[1]);return-errno;}
    char range[64];snprintf(range,sizeof(range),"%lld-",(long long)offset);
    const char *args[]={"curl","-sL","--fail","--proto","=https","--proto-redir","=https",
        "--max-redirs","5","--cacert","./res/cacert.pem","--connect-timeout","15",
        "--range",range,"--dump-header","/proc/self/fd/2","-A","NXT-Retro-Music","-o","-",s->url,NULL};
    pid_t child=fork();
    if(child<0){close(body[0]);close(body[1]);close(headers[0]);close(headers[1]);return-errno;}
    if(!child){
        if(dup2(body[1],STDOUT_FILENO)<0||dup2(headers[1],STDERR_FILENO)<0)_exit(127);
        close(body[0]);close(body[1]);close(headers[0]);close(headers[1]);
        execvp("curl",(char *const *)args);_exit(127);
    }
    close(body[1]);close(headers[1]);s->body=body[0];s->headers=headers[0];s->child=child;
    fcntl(s->body,F_SETFL,O_NONBLOCK);fcntl(s->headers,F_SETFL,O_NONBLOCK);
    char buffer[HEADER_LIMIT+1];size_t used=0;int64_t deadline=millis()+READ_TIMEOUT_MS;int result=-ETIMEDOUT;
    for(;;){
        if(cancelled(s)){result=-ECANCELED;break;}
        struct pollfd fds[]={{s->headers,POLLIN,0},{s->body,POLLIN,0}};
        int n=poll(fds,2,100);if(n<0){if(errno==EINTR)continue;result=-errno;break;}
        for(;;){ssize_t got=read(s->headers,buffer+used,HEADER_LIMIT-used);if(got>0){used+=got;if(used==HEADER_LIMIT){result=-E2BIG;goto finished;}continue;}if(got<0&&errno!=EAGAIN&&errno!=EINTR){result=-errno;goto finished;}break;}
        if(fds[1].revents&(POLLIN|POLLHUP|POLLERR)){
            buffer[used]=0;result=parse_headers(s,buffer,offset);break;
        }
        if(millis()>=deadline)break;
    }
finished:
    if(s->headers>=0){close(s->headers);s->headers=-1;}
    if(result<0){end_child(s);return result;}
    s->position=offset;return 0;
}
static int bytes(NetStream *s,void *out,size_t length){
    if(!length)return 0;if(s->body<0)return 0;
    int64_t deadline=millis()+READ_TIMEOUT_MS;
    for(;;){
        if(cancelled(s))return-ECANCELED;
        ssize_t n=read(s->body,out,length);
        if(n>0){s->position+=n;return(int)n;}
        if(!n){
            int status;pid_t p=waitpid(s->child,&status,WNOHANG);
            if(p==s->child){s->child=0;return WIFEXITED(status)&&WEXITSTATUS(status)==0?0:-EIO;}
            if(p<0&&errno!=EINTR)return-EIO;
        }else if(errno!=EAGAIN&&errno!=EINTR)return-errno;
        if(millis()>=deadline)return-ETIMEDOUT;
        struct pollfd p={s->body,POLLIN,0};if(poll(&p,1,100)<0&&errno!=EINTR)return-errno;
    }
}
static NetStream *NetStream_open(const NxtDriveEntry *entry,const bool *stop,const unsigned *reader_epoch){
    if(!entry||entry->folder)return NULL;
    NetStream *s=calloc(1,sizeof(*s));if(!s)return NULL;
    s->body=s->headers=-1;s->size=-1;s->entry=*entry;s->stop=stop;s->reader_epoch=reader_epoch;
    if(!NxtDrive_url(entry,true,s->url,sizeof(s->url))||connect_at(s,0)<0)goto fail;
    if(s->html){
        char *html=calloc(1,128*1024+1);if(!html)goto fail;size_t used=0;bool ok=false;
        while(used<128*1024){int n=bytes(s,html+used,128*1024-used);if(n<0)break;used+=n;html[used]=0;if(!n||strcasestr(html,"</form>")){ok=NxtDrive_confirmation(html,entry,s->url,sizeof(s->url));break;}}
        free(html);if(!ok||connect_at(s,0)<0||s->html)goto fail;
    }
    return s;
fail:
    NetStream_close(s);return NULL;
}
static int NetStream_read(NetStream *s,void *out,size_t length){
    int n=bytes(s,out,length);
    if(n<0&&n!=-ECANCELED&&s->seekable&&s->size>s->position){
        int64_t position=s->position;
        if(connect_at(s,position)==0)n=bytes(s,out,length);
    }
    return n;
}
static int64_t NetStream_seek(NetStream *s,int64_t offset){
    if(offset<0||(s->size>=0&&offset>s->size))return-EINVAL;
    if(offset==s->position)return offset;
    if(offset&& !s->seekable)return-ESPIPE;
    if(s->size>=0&&offset==s->size){end_child(s);s->position=offset;return offset;}
    int r=connect_at(s,offset);return r<0?r:offset;
}
static int64_t NetStream_size(const NetStream *s){return s->size;}
static int64_t NetStream_position(const NetStream *s){return s->position;}
static bool NetStream_seekable(const NetStream *s){return s->seekable;}
static void NetStream_close(NetStream *s){if(s){end_child(s);free(s);}}
