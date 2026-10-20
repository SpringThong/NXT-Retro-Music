/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_log.h"
#include "nxt.h"
#include "file_utils.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
void NxtLog_step(const char *component,const char *step,bool ok,const char *format,...){
    char path[1200],previous[1220],msg[768],stamp[64];
    va_list args;va_start(args,format);vsnprintf(msg,sizeof(msg),format,args);va_end(args);
    for(char *p=msg;*p;p++)if(*p=='\r'||*p=='\n')*p=' ';
    time_t now=time(NULL);struct tm tm;localtime_r(&now,&tm);strftime(stamp,sizeof(stamp),"%Y-%m-%d %H:%M:%S",&tm);
    pthread_mutex_lock(&lock);userdata_mkdir("");userdata_snpath("app-log.txt",path,sizeof(path));
    struct stat st;if(!stat(path,&st)&&st.st_size>1024*1024){snprintf(previous,sizeof(previous),"%s.previous",path);rename(path,previous);}
    FILE *f=fopen(path,"a");if(f){chmod(path,0600);fprintf(f,"%s v%s %s [%s/%s] %s\n",stamp,NXT_VERSION,ok?"OK":"ERROR",component,step,msg);fclose(f);}pthread_mutex_unlock(&lock);
}
bool NxtLog_export(void){
    char path[1200],buf[8192];userdata_snpath("app-log.txt",path,sizeof(path));bool ok=false;
    pthread_mutex_lock(&lock);FILE *in=fopen(path,"rb"),*out=in?fopen("NXT_App_Log.txt","wb"):NULL;
    if(in&&out){size_t n;ok=true;while((n=fread(buf,1,sizeof(buf),in)))if(fwrite(buf,1,n,out)!=n){ok=false;break;}if(ferror(in))ok=false;}
    if(in)fclose(in);if(out){if(fclose(out))ok=false;chmod("NXT_App_Log.txt",0600);}pthread_mutex_unlock(&lock);return ok;
}
