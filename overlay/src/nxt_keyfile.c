/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "nxt_keyfile.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
static void wipe(void *ptr,size_t n){volatile unsigned char *p=ptr;while(n--)*p++=0;}
static char *trim(char *line){while(*line&&isspace((unsigned char)*line))line++;size_t n=strlen(line);while(n&&isspace((unsigned char)line[n-1]))line[--n]=0;return line;}
int NxtKeyFile_readGroq(const char *path,char *output,size_t capacity){
 if(output&&capacity)wipe(output,capacity);
 if(!path||!output||capacity<13)return 2;
 int fd=open(path,O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK);
 if(fd<0)return errno==ENOENT?1:2;
 struct stat st;unsigned char raw[4097];char text[4097];size_t n=0,z=0;int result=2;
 if(fstat(fd,&st)||!S_ISREG(st.st_mode)||st.st_size<=0||st.st_size>4096)goto done;
 while(n<sizeof(raw)){ssize_t got=read(fd,raw+n,sizeof(raw)-n);if(got<0){if(errno==EINTR)continue;goto done;}if(!got)break;n+=(size_t)got;}
 if(!n||n>4096)goto done;
 size_t start=0;
 if(n>=2&&((raw[0]==0xff&&raw[1]==0xfe)||(raw[0]==0xfe&&raw[1]==0xff))){
  int le=raw[0]==0xff;if((n-2)%2)goto done;
  for(size_t i=2;i<n;i+=2){unsigned value=le?raw[i]|raw[i+1]<<8:raw[i]<<8|raw[i+1];if(!value||value>127)goto done;text[z++]=(char)value;}
 }else{
  if(n>=3&&raw[0]==0xef&&raw[1]==0xbb&&raw[2]==0xbf)start=3;
  for(size_t i=start;i<n;i++){if(!raw[i]||raw[i]==127||(raw[i]<32&&raw[i]!='\r'&&raw[i]!='\n'&&raw[i]!='\t'))goto done;text[z++]=(char)raw[i];}
 }
 text[z]=0;int count=0;
 for(char *line=text;line;){
  char *next=strchr(line,'\n');if(next)*next++=0;char *key=trim(line);
  if(*key&&*key!='#'&&*key!=';'){
   if(!strncmp(key,"GROQ_API_KEY",12)){key=trim(key+12);if(*key++!='=')goto done;key=trim(key);}
   size_t len=strlen(key);if(len>=2&&((key[0]=='"'&&key[len-1]=='"')||(key[0]=='\''&&key[len-1]=='\''))){key[len-1]=0;key++;len-=2;}
   if(++count!=1||len<12||len>=capacity||len>=256||strncmp(key,"gsk_",4))goto done;
   for(size_t i=4;i<len;i++)if(!((key[i]>='a'&&key[i]<='z')||(key[i]>='A'&&key[i]<='Z')||(key[i]>='0'&&key[i]<='9')||key[i]=='_'||key[i]=='-'))goto done;
   memcpy(output,key,len+1);
  }
  line=next;
 }
 if(count==1)result=0;
done:
 close(fd);wipe(raw,sizeof(raw));wipe(text,sizeof(text));if(result)wipe(output,capacity);return result;
}
