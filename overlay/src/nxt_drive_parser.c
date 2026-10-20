/* SPDX-License-Identifier: GPL-3.0-only
 * Public embeddedfolderview anchors, also used by the maintained gdown project.
 * No Google API keys, account cookies or private-file access are requested. */
#include "nxt_drive_parser.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

static bool identifier(const char *p,size_t n) {
    if(!n||n>=128)return false;
    for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)p[i])&&p[i]!='-'&&p[i]!='_')return false;
    return true;
}
static bool parameter(const char *url,const char *key,char out[128]) {
    const char *p=strchr(url,'?');if(!p)return false;p++;
    size_t k=strlen(key);
    while(*p&&*p!='#') {
        const char *end=p+strcspn(p,"&#");
        if((size_t)(end-p)>k&&!memcmp(p,key,k)&&p[k]=='=') {
            size_t n=end-(p+k+1);if(!identifier(p+k+1,n))return false;
            memcpy(out,p+k+1,n);out[n]=0;return true;
        }
        p=*end=='&'?end+1:end;
    }
    return false;
}
bool NxtDrive_parseURL(const char *url,NxtDriveEntry *out) {
    if(!url||!out||strlen(url)>=512||strncmp(url,"https://",8))return false;
    memset(out,0,sizeof(*out));
    const char *host=url+8,*path=strchr(host,'/');if(!path)return false;
    size_t h=path-host;
    bool drive=h==16&&!strncasecmp(host,"drive.google.com",h);
    bool docs=h==15&&!strncasecmp(host,"docs.google.com",h);
    bool download=h==28&&!strncasecmp(host,"drive.usercontent.google.com",h);
    if(!drive&&!docs&&!download)return false;
    const char *id=NULL;
    if(!strncmp(path,"/file/d/",8))id=path+8;
    else if(drive) {
        const char *folders=NULL;
        if(!strncmp(path,"/drive/folders/",15))folders=path+15;
        else if(!strncmp(path,"/drive/u/",9)) {
            const char *p=path+9;while(isdigit((unsigned char)*p))p++;
            if(!strncmp(p,"/folders/",9))folders=p+9;
        }
        if(folders){id=folders;out->folder=true;}
        else if(!strncmp(path,"/embeddedfolderview?",20))out->folder=true;
        else if(strncmp(path,"/open?",6)&&strncmp(path,"/uc?",4)&&strncmp(path,"/download?",10))return false;
    }else if(download){if(strncmp(path,"/download?",10))return false;}
    else return false;
    if(id) {
        size_t n=strcspn(id,"/?#");if(n<10||!identifier(id,n))return false;
        memcpy(out->id,id,n);out->id[n]=0;
    }else if(!parameter(url,"id",out->id)||strlen(out->id)<10)return false;
    if(strstr(url,"resourcekey=")&&!parameter(url,"resourcekey",out->resource_key))return false;
    return true;
}
bool NxtDrive_url(const NxtDriveEntry *entry,bool download,char *out,size_t capacity) {
    if(!identifier(entry->id,strlen(entry->id))||
       (entry->resource_key[0]&&!identifier(entry->resource_key,strlen(entry->resource_key))))return false;
    int n;
    if(download)n=snprintf(out,capacity,"https://drive.usercontent.google.com/download?id=%s&export=download&confirm=t%s%s",entry->id,entry->resource_key[0]?"&resourcekey=":"",entry->resource_key);
    else n=snprintf(out,capacity,"https://drive.google.com/%s%s%s%s%s",entry->folder?"drive/folders/":"file/d/",entry->id,entry->folder?"":"/view",entry->resource_key[0]?"?resourcekey=":"",entry->resource_key);
    return n>=0&&(size_t)n<capacity;
}
static void put_utf8(char *out,size_t size,size_t *n,uint32_t cp) {
    unsigned char s[4];int bytes=0;
    if(cp<128){s[0]=cp;bytes=1;}
    else if(cp<2048){s[0]=0xc0|(cp>>6);s[1]=0x80|(cp&63);bytes=2;}
    else if(cp<65536){s[0]=0xe0|(cp>>12);s[1]=0x80|((cp>>6)&63);s[2]=0x80|(cp&63);bytes=3;}
    else if(cp<=0x10ffff){s[0]=0xf0|(cp>>18);s[1]=0x80|((cp>>12)&63);s[2]=0x80|((cp>>6)&63);s[3]=0x80|(cp&63);bytes=4;}
    if(bytes&&*n+bytes<size){memcpy(out+*n,s,bytes);*n+=bytes;out[*n]=0;}
}
static void decode(const char *p,const char *end,char *out,size_t size,bool tags) {
    size_t used=0;if(!size)return;out[0]=0;
    while(p<end&&*p&&used+1<size) {
        if(tags&&*p=='<'){const char *close=memchr(p,'>',end-p);if(!close)break;p=close+1;continue;}
        if(*p=='&') {
            const char *semi=memchr(p,';',end-p<16?end-p:16);uint32_t cp=0;
            if(semi){size_t n=semi-p;
                if(n==4&&!memcmp(p,"&amp",4))cp='&';
                else if(n==3&&!memcmp(p,"&lt",3))cp='<';
                else if(n==3&&!memcmp(p,"&gt",3))cp='>';
                else if(n==5&&!memcmp(p,"&quot",5))cp='"';
                else if(n==5&&!memcmp(p,"&apos",5))cp='\'';
                else if(n==5&&!memcmp(p,"&nbsp",5))cp=' ';
                else if(n>2&&p[1]=='#'){char number[16];memcpy(number,p+2,n-2);number[n-2]=0;char *last=NULL;cp=strtoul(number+((number[0]=='x'||number[0]=='X')?1:0),&last,(number[0]=='x'||number[0]=='X')?16:10);if(!last||*last)cp=0;}
                if(cp&&cp<=0x10ffff&&!(cp>=0xd800&&cp<=0xdfff)){put_utf8(out,size,&used,cp);p=semi+1;continue;}
            }
        }
        if(tags&&isspace((unsigned char)*p)){if(used&&out[used-1]!=' ')out[used++]=' ';p++;}
        else {int bytes=1;unsigned char c=*p;if(c>=0xf0)bytes=4;else if(c>=0xe0)bytes=3;else if(c>=0xc0)bytes=2;
            if(p+bytes>end||used+bytes>=size)break;memcpy(out+used,p,bytes);used+=bytes;p+=bytes;}
        out[used]=0;
    }
    while(used&&isspace((unsigned char)out[used-1]))out[--used]=0;
}
static bool attr(const char *tag,const char *end,const char *name,char *out,size_t size) {
    const char *p=tag;size_t k=strlen(name);
    while(p<end){
        while(p<end&&(isspace((unsigned char)*p)||*p=='<'||*p=='/'))p++;
        const char *key=p;while(p<end&&!isspace((unsigned char)*p)&&*p!='='&&*p!='>')p++;
        size_t n=p-key;while(p<end&&isspace((unsigned char)*p))p++;
        if(p>=end)break;
        if(*p!='='){if(p==key)p++;continue;}p++;
        while(p<end&&isspace((unsigned char)*p))p++;
        char quote=(*p=='\''||*p=='"')?*p++:0;const char *v=p;
        if(quote){while(p<end&&*p!=quote)p++;}
        else while(p<end&&!isspace((unsigned char)*p)&&*p!='>')p++;
        if(n==k&&!strncasecmp(key,name,k)){decode(v,p,out,size,false);return true;}
        if(quote&&p<end)p++;
    }
    return false;
}
static bool page_title(const char *html,char *out,size_t size) {
    const char *start=strcasestr(html,"<title");if(!start)return false;
    start=strchr(start,'>');if(!start)return false;start++;
    const char *end=strcasestr(start,"</title>");if(!end)return false;
    decode(start,end,out,size,true);
    char *suffix=strstr(out," - Google Drive");if(suffix)*suffix=0;
    return *out!=0;
}
bool NxtDrive_fileName(const char *html,char *out,size_t size) {
    return !strstr(html,"ServiceLogin")&&!strstr(html,"accounts.google.com/signin")&&
           !strcasestr(html,"You need access")&&page_title(html,out,size)&&strcmp(out,"Google Drive")&&strcmp(out,"Sign in");
}
int NxtDrive_parseFolder(const char *html,NxtDriveEntry *entries,int maximum,char *title,size_t title_size) {
    if(!html||strstr(html,"ServiceLogin")||strstr(html,"accounts.google.com/signin")||strcasestr(html,"You need access")||!page_title(html,title,title_size))return -1;
    int count=0;const char *p=html;
    while((p=strcasestr(p,"<a"))) {
        p+=2;if(!isspace((unsigned char)*p)&&*p!='>')continue;
        const char *end=strchr(p,'>');if(!end)break;
        const char *close=strcasestr(end+1,"</a>");if(!close)break;
        char url[512];NxtDriveEntry entry;
        if(attr(p,end,"href",url,sizeof(url))&&NxtDrive_parseURL(url,&entry)) {
            decode(end+1,close,entry.name,sizeof(entry.name),true);
            bool duplicate=false;for(int i=0;i<count;i++)if(!strcmp(entries[i].id,entry.id))duplicate=true;
            if(entry.name[0]&&!duplicate){if(count==maximum)return -2;entries[count++]=entry;}
        }
        p=close+4;
    }
    if(!count&&!strstr(html,"flip-entries")&&!strstr(html,"folderview"))return -1;
    return count;
}
bool NxtDrive_confirmation(const char *html,const NxtDriveEntry *entry,char *out,size_t size) {
    const char *form=strcasestr(html,"<form");if(!form)return false;
    const char *end=strchr(form,'>');if(!end)return false;
    const char *close=strcasestr(end,"</form>");if(!close)return false;
    char action[256];if(!attr(form,end,"action",action,sizeof(action))||strcmp(action,"https://drive.usercontent.google.com/download"))return false;
    char id[128]="",confirm[128]="",uuid[128]="";const char *p=end;
    while((p=strcasestr(p,"<input"))&&p<close){end=strchr(p,'>');if(!end||end>close)break;
        char name[64],value[128];if(attr(p,end,"name",name,sizeof(name))&&attr(p,end,"value",value,sizeof(value))&&identifier(value,strlen(value))){
            if(!strcmp(name,"id"))snprintf(id,sizeof(id),"%s",value);
            if(!strcmp(name,"confirm"))snprintf(confirm,sizeof(confirm),"%s",value);
            if(!strcmp(name,"uuid"))snprintf(uuid,sizeof(uuid),"%s",value);
        }p=end+1;
    }
    if(strcmp(id,entry->id)||!*confirm)return false;
    int n=snprintf(out,size,"%s?id=%s&export=download&confirm=%s%s%s%s%s",action,id,confirm,*uuid?"&uuid=":"",uuid,entry->resource_key[0]?"&resourcekey=":"",entry->resource_key);
    return n>=0&&(size_t)n<size;
}
