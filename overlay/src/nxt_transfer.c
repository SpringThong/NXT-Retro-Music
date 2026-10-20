/* SPDX-License-Identifier: GPL-3.0-only. On-demand LAN transfer; bounded streaming. */
#include "nxt_transfer.h"
#include "nxt_text.h"
#include "nxt_skin.h"
#include <utf8proc.h>
#include "toast.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_t worker;static int server=-1,client=-1,folder=-1;static bool running,started,copying;
static char pin[7],address[128],message[256];static unsigned long long received,total;
bool NxtTransfer_isBusy(void){pthread_mutex_lock(&lock);bool value=copying;pthread_mutex_unlock(&lock);return value;}
static bool live(void){pthread_mutex_lock(&lock);bool v=running;pthread_mutex_unlock(&lock);return v;}
static void status(const char *s,unsigned long long done,unsigned long long size){pthread_mutex_lock(&lock);snprintf(message,sizeof(message),"%s",s);received=done;total=size;pthread_mutex_unlock(&lock);}
static bool sendall(int fd,const void *p,size_t n){while(n){ssize_t k=send(fd,p,n,MSG_NOSIGNAL);if(k<0&&errno==EINTR)continue;if(k<=0)return false;p=(const char*)p+k;n-=k;}return true;}
static void reply(int fd,int code,const char *type,const char *body){char head[256];int n=snprintf(head,sizeof(head),"HTTP/1.1 %d Result\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",code,type,strlen(body));if(sendall(fd,head,n))sendall(fd,body,strlen(body));}
static const char page[]=
 "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width'><title>NXT File Transfer</title><style>body{background:#142d32;color:#e0ece8;font:18px system-ui;max-width:600px;margin:50px auto;padding:24px}input,button{font:inherit;max-width:100%;margin:14px 0;padding:12px;border-radius:10px}progress{width:100%}</style><h1>NXT · File transfer</h1><p>Enter the PIN shown on your device. Files are copied into its music folder.</p><input id=p placeholder='6-digit PIN' inputmode=numeric maxlength=6><input id=f type=file multiple><button id=u>Upload files</button><button id=c>Cancel</button><progress id=g max=100 value=0></progress><p id=s></p><script>let xhr;const q=x=>document.getElementById(x);q('c').onclick=()=>xhr&&xhr.abort();q('u').onclick=async()=>{q('u').disabled=true;for(const file of q('f').files){try{await new Promise((ok,no)=>{xhr=new XMLHttpRequest;xhr.open('PUT','/upload/'+encodeURIComponent(file.name));xhr.setRequestHeader('X-NXT-PIN',q('p').value);xhr.upload.onprogress=e=>{if(e.lengthComputable)q('g').value=e.loaded/e.total*100};xhr.onload=()=>{q('s').textContent=file.name+': '+xhr.responseText;xhr.status===201?ok():no()};xhr.onerror=xhr.onabort=no;xhr.send(file)})}catch(e){break}}q('u').disabled=false}</script>";
static bool filename(char *out,const char *url){size_t n=0;for(;*url;url++){unsigned char c=*url;if(c=='%'){if(!isxdigit((unsigned char)url[1])||!url[2]||!isxdigit((unsigned char)url[2]))return false;char hex[3]={url[1],url[2],0};c=strtoul(hex,NULL,16);url+=2;}if(c<32||c==127||c=='/'||c=='\\'||n>=200)return false;out[n++]=c;}out[n]=0;if(!n||out[0]=='.'||strstr(out,".."))return false;const char *ext=strrchr(out,'.');if(!ext)return false;const char *allow[]={".mp3",".dsf",".dff",".flac",".wav",".m4a",".aac",".ogg",".opus",".alac",".lrc",".txt",".jpg",".jpeg",".png",".m3u",".m3u8"};bool supported=false;for(unsigned i=0;i<sizeof(allow)/sizeof(*allow);i++)if(!strcasecmp(ext,allow[i]))supported=true;if(!supported)return false;const unsigned char *p=(const unsigned char*)out;while(*p){utf8proc_int32_t cp;ssize_t k=utf8proc_iterate(p,-1,&cp);if(k<=0)return false;p+=k;}return true;}
/* FAT/exFAT have no hard links. Use atomic no-replace rename on supported
 * kernels; otherwise reserve the destination exclusively and stream-copy. */
static bool install_file(const char *tmp,const char *name){
#ifdef SYS_renameat2
 if(syscall(SYS_renameat2,folder,tmp,folder,name,1)==0)return true;
 if(errno!=ENOSYS&&errno!=EINVAL&&errno!=EOPNOTSUPP)return false;
#endif
 int src=openat(folder,tmp,O_RDONLY|O_NOFOLLOW);if(src<0)return false;
 int dst=openat(folder,name,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);if(dst<0){close(src);return false;}
 char data[32768];bool ok=true;for(;;){ssize_t n=read(src,data,sizeof(data));if(n<0&&errno==EINTR)continue;if(n<0){ok=false;break;}if(!n)break;if(!live()){ok=false;break;}
  for(ssize_t i=0;i<n;){ssize_t k=write(dst,data+i,n-i);if(k<0&&errno==EINTR)continue;if(k<=0){ok=false;break;}i+=k;}if(!ok)break;}
 if(ok&&fsync(dst))ok=false;close(src);close(dst);if(!ok)unlinkat(folder,name,0);return ok;
}
static void request(int fd){
 char buffer[8193],header[8193];size_t used=0,body=0;for(;;){if(!live())return;ssize_t n=recv(fd,buffer+used,8192-used,0);if(n<0&&errno==EINTR)continue;if(n<=0)return;used+=n;buffer[used]=0;char *end=strstr(buffer,"\r\n\r\n");if(end){body=end-buffer+4;memcpy(header,buffer,body);header[body]=0;break;}if(used==8192){reply(fd,431,"text/plain","Header too large");return;}}
 char method[12],url[640],version[20];if(sscanf(header,"%11s %639s %19s",method,url,version)!=3){reply(fd,400,"text/plain","Bad request");return;}
 if(!strcmp(method,"GET")&&!strcmp(url,"/")){reply(fd,200,"text/html; charset=utf-8",page);return;}
 if(strcmp(method,"PUT")||strncmp(url,"/upload/",8)){reply(fd,404,"text/plain","Not found");return;}
 char *line=strstr(header,"\r\n")+2,auth[64]="";unsigned long long size=0;bool has_size=false,bad=false;int auth_count=0;
 while(*line){char *end=strstr(line,"\r\n");if(!end||end==line)break;*end=0;if(!strncasecmp(line,"X-NXT-PIN:",10)){auth_count++;char *p=line+10;while(*p==' ')p++;snprintf(auth,sizeof(auth),"%s",p);}if(!strncasecmp(line,"Content-Length:",15)){if(has_size)bad=true;char *p=line+15,*tail;while(*p==' ')p++;errno=0;size=strtoull(p,&tail,10);if(!*p||*tail||errno||*p=='-')bad=true;has_size=true;}if(!strncasecmp(line,"Transfer-Encoding:",18))bad=true;line=end+2;}
 if(auth_count!=1||strcmp(auth,pin)){reply(fd,403,"text/plain","Incorrect PIN");return;}
 char name[256];if(bad||!has_size||!size||size>2147483648ULL||!filename(name,url+8)){reply(fd,400,"text/plain","Unsupported file or size");return;}
 char tmp[256];snprintf(tmp,sizeof(tmp),".nxt-upload-%ld.part",(long)getpid());int out=openat(folder,tmp,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);if(out<0){reply(fd,409,"text/plain","Cannot create temporary file");return;}
 pthread_mutex_lock(&lock);copying=true;pthread_mutex_unlock(&lock);
 unsigned long long done=0;bool ok=true;status(name,0,size);char chunk[32768];size_t first=used-body;if(first>size)first=size;
 while(done<size&&live()){const char *p=buffer+body;size_t n=first;if(!n){ssize_t got=recv(fd,chunk,(size-done)<sizeof(chunk)?(size_t)(size-done):sizeof(chunk),0);if(got<0&&errno==EINTR)continue;if(got<=0){ok=false;break;}p=chunk;n=got;}first=0;size_t written=0;while(written<n){ssize_t k=write(out,p+written,n-written);if(k<0&&errno==EINTR)continue;if(k<=0){ok=false;break;}written+=k;}if(!ok)break;done+=n;status(name,done,size);}
 ok=ok&&done==size&&live();if(ok&&fsync(out))ok=false;close(out);char final[256];snprintf(final,sizeof(final),"%s",name);bool installed=false;
 if(ok)for(int i=0;i<1000;i++){if(i)snprintf(final,sizeof(final),"%d-%s",i,name);if(install_file(tmp,final)){installed=true;break;}if(errno!=EEXIST)break;}
 unlinkat(folder,tmp,0);if(installed){fsync(folder);NxtLibrary_notify();status(final,done,size);reply(fd,201,"text/plain; charset=utf-8",final);}else{status("Upload cancelled or failed",done,size);reply(fd,409,"text/plain","Upload failed");}
 pthread_mutex_lock(&lock);copying=false;pthread_mutex_unlock(&lock);
}
static void *serve(void *unused){while(live()){int fd=accept(server,NULL,NULL);if(fd<0){if(errno==EINTR)continue;break;}struct timeval timeout={20,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));pthread_mutex_lock(&lock);client=fd;pthread_mutex_unlock(&lock);request(fd);pthread_mutex_lock(&lock);close(fd);client=-1;pthread_mutex_unlock(&lock);}return NULL;}
bool NxtTransfer_start(const char *root){
 if(started)return true;folder=open(root,O_RDONLY|O_DIRECTORY|O_NOFOLLOW);if(folder<0)return false;unsigned value;int random=open("/dev/urandom",O_RDONLY);bool ok=random>=0&&read(random,&value,sizeof(value))==sizeof(value);if(random>=0)close(random);if(!ok){close(folder);folder=-1;return false;}snprintf(pin,sizeof(pin),"%06u",value%1000000);
 server=socket(AF_INET,SOCK_STREAM,0);struct sockaddr_in bindaddr={.sin_family=AF_INET,.sin_addr.s_addr=htonl(INADDR_ANY)};if(server<0||bind(server,(struct sockaddr*)&bindaddr,sizeof(bindaddr))||listen(server,2)){if(server>=0)close(server);close(folder);server=folder=-1;return false;}socklen_t len=sizeof(bindaddr);getsockname(server,(struct sockaddr*)&bindaddr,&len);
 char ip[64]="127.0.0.1";struct ifaddrs *ifs;if(!getifaddrs(&ifs)){for(struct ifaddrs *p=ifs;p;p=p->ifa_next)if(p->ifa_addr&&p->ifa_addr->sa_family==AF_INET&&strcmp(p->ifa_name,"lo")){inet_ntop(AF_INET,&((struct sockaddr_in*)p->ifa_addr)->sin_addr,ip,sizeof(ip));if(!strncmp(p->ifa_name,"wlan",4))break;}freeifaddrs(ifs);}snprintf(address,sizeof(address),"http://%s:%u",ip,ntohs(bindaddr.sin_port));running=true;status("Ready",0,0);
 if(pthread_create(&worker,NULL,serve,NULL)){running=false;close(server);close(folder);server=folder=-1;return false;}started=true;return true;
}
void NxtTransfer_stop(void){if(!started)return;pthread_mutex_lock(&lock);running=false;shutdown(server,SHUT_RDWR);if(client>=0)shutdown(client,SHUT_RDWR);pthread_mutex_unlock(&lock);pthread_join(worker,NULL);close(server);close(folder);server=folder=-1;started=false;}
void NxtTransfer_status(char *url,size_t u,char *code,size_t c,unsigned long long *done,unsigned long long *size){pthread_mutex_lock(&lock);if(url)snprintf(url,u,"%s",address);if(code)snprintf(code,c,"%s",pin);if(done)*done=received;if(size)*size=total;pthread_mutex_unlock(&lock);}
ModuleExitReason NxtTransfer_run(DisplayContext *d){
 if(!NxtTransfer_start(Nxt_musicRoot())){Toast_show(Nxt_text("Cannot start file transfer","Không mở được chuyển file"),2500);return MODULE_EXIT_TO_MENU;}ModuleCommon_setAutosleepDisabled(true);uint32_t drawn=0;int show=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;
 for(;;){ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit){result=MODULE_EXIT_QUIT;break;}if(!g.input_consumed&&!Nxt_lockInput()){if(NxtPad_justPressed(BTN_B))break;if(NxtPad_justPressed(BTN_X)){pthread_mutex_lock(&lock);if(client>=0)shutdown(client,SHUT_RDWR);pthread_mutex_unlock(&lock);}}
 if(NxtSkin_shouldDraw(drawn,g.dirty)){char url[128],code[7],progress[128],pinlabel[40],file[256];unsigned long long done,size;NxtTransfer_status(url,sizeof(url),code,sizeof(code),&done,&size);pthread_mutex_lock(&lock);snprintf(file,sizeof(file),"%s",message);pthread_mutex_unlock(&lock);snprintf(pinlabel,sizeof(pinlabel),"PIN · %s",code);snprintf(progress,sizeof(progress),"%llu / %llu MiB",done/1048576,size/1048576);const char *rows[]={url,pinlabel,file,progress,Nxt_text("Open this address on the same Wi-Fi","Mở địa chỉ này trên cùng Wi-Fi")};NxtSkin_begin(s,Nxt_text("Wi-Fi file transfer","Chuyển file qua Wi-Fi"),NULL,false);for(int i=0;i<5;i++)NxtSkin_text(s,i<2?28:22,rows[i],48*s->w/1024,(170+i*80)*s->w/1024,925*s->w/1024,NxtSkin_color(NXT_INK));NxtSkin_footer(s,Nxt_text("X Cancel file · B Stop transfer","X Hủy file · B Dừng chuyển"),false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(s);
 }NxtTransfer_stop();ModuleCommon_setAutosleepDisabled(false);return result;
}
