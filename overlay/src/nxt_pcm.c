/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_pcm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int transport_bits(const char *format){
    const char *names[]={"S8","U8","S16_LE","S16_BE","U16_LE","U16_BE","S18_3LE","S18_3BE","S20_3LE","S20_3BE","S24_LE","S24_BE","S24_3LE","S24_3BE","U24_LE","U24_BE","S32_LE","S32_BE","U32_LE","U32_BE","FLOAT_LE","FLOAT_BE","FLOAT64_LE","FLOAT64_BE"};
    const int bits[]={8,8,16,16,16,16,18,18,20,20,24,24,24,24,24,24,32,32,32,32,32,32,64,64};
    for(size_t i=0;i<sizeof(bits)/sizeof(bits[0]);i++)if(!strcmp(format,names[i]))return bits[i];
    return 0;
}
bool NxtPCM_parse(const char *text,NxtPCM *out){
    NxtPCM pcm={0};if(!out)return false;*out=pcm;if(!text)return false;
    char copy[2048];if(strlen(text)>=sizeof(copy))return false;strcpy(copy,text);
    char *save=NULL;
    for(char *line=strtok_r(copy,"\r\n",&save);line;line=strtok_r(NULL,"\r\n",&save)){
        if(!strncmp(line,"format:",7))sscanf(line+7,"%31s",pcm.format);
        else if(!strncmp(line,"rate:",5)){char *end;long v=strtol(line+5,&end,10);if(end!=line+5&&v>0&&v<=1536000)pcm.rate=(int)v;}
        else if(!strncmp(line,"channels:",9)){char *end;long v=strtol(line+9,&end,10);if(end!=line+9&&v>0&&v<=256)pcm.channels=(int)v;}
    }
    pcm.bits=transport_bits(pcm.format);
    if(!pcm.rate||!pcm.bits||!pcm.channels)return false;
    *out=pcm;return true;
}
bool NxtPCM_read(const char *root,int card,NxtPCM *out){
    if(!out)return false;memset(out,0,sizeof(*out));if(!root||card<0||card>=32)return false;
    /* App route explicitly targets hw:card,0. Do not read another card/device. */
    for(int sub=0;sub<32;sub++){
        char path[1024],text[2048];int n=snprintf(path,sizeof(path),"%s/card%d/pcm0p/sub%d/hw_params",root,card,sub);
        if(n<0||(size_t)n>=sizeof(path))return false;
        FILE *f=fopen(path,"r");if(!f)continue;size_t len=fread(text,1,sizeof(text)-1,f);bool complete=feof(f);fclose(f);text[len]=0;
        if(complete&&NxtPCM_parse(text,out))return true;
    }
    return false;
}
void NxtPCM_label(const NxtPCM *pcm,bool vi,char *out,size_t size){
    if(!out||!size)return;
    const char *prefix=vi?"DAC thực tế":"DAC actual";
    if(!pcm||!pcm->rate||!pcm->bits){snprintf(out,size,"%s: %s",prefix,vi?"chưa xác nhận":"unconfirmed");return;}
    char rate[32];if(pcm->rate%1000==0)snprintf(rate,sizeof(rate),"%d",pcm->rate/1000);else snprintf(rate,sizeof(rate),"%.3f",pcm->rate/1000.0);
    /* Trim fractional zeroes while retaining e.g. 44.1 and 88.2 kHz. */
    if(strchr(rate,'.')){size_t n=strlen(rate);while(n&&rate[n-1]=='0')rate[--n]=0;if(n&&rate[n-1]=='.')rate[--n]=0;}
    snprintf(out,size,"%s: %s kHz · %d-bit PCM",prefix,rate,pcm->bits);
}
