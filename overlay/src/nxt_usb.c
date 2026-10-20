/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_usb.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
static bool read_node(const char *base,const char *name,char *text,size_t size){
    char path[1024];if(!base||!text||size<2)return false;
    int n=snprintf(path,sizeof(path),"%s/%s",base,name);if(n<0||(size_t)n>=sizeof(path))return false;
    int fd=open(path,O_RDONLY);if(fd<0){fprintf(stderr,"Bottom USB: open %s: %s\n",path,strerror(errno));return false;}
    /* Vendor action nodes can perform the transition on each read. Read once;
       stdio fread may issue a second read after a short vendor response. */
    ssize_t got=read(fd,text,size-1);int err=errno;close(fd);
    if(got<0){fprintf(stderr,"Bottom USB: read %s: %s\n",path,strerror(err));return false;}
    size_t len=(size_t)got;text[len]=0;
    while(len&&isspace((unsigned char)text[len-1]))text[--len]=0;
    size_t start=0;while(text[start]&&isspace((unsigned char)text[start]))start++;
    if(start)memmove(text,text+start,len-start+1);
    return true;
}
int NxtUSB_role(const char *base){
    char role[64];if(!read_node(base,"otg_role",role,sizeof(role)))return 0;
    if(!strcmp(role,"usb_host"))return 1;if(!strcmp(role,"usb_device"))return 2;return 0;
}
int NxtUSB_switch(const char *base,bool host){
    int wanted=host?1:2,current=NxtUSB_role(base);if(current==wanted)return 1;
    char reply[128];
    /* Only an explicit user request reads the action node, even when the
       initial role is unknown. Status polling never touches action nodes. */
    if(!read_node(base,host?"usb_host":"usb_device",reply,sizeof(reply)))return 0;
    int after=NxtUSB_role(base);
    fprintf(stderr,"Bottom USB: action=%s before=%d reply=%s after=%d\n",host?"usb_host":"usb_device",current,reply,after);
    /* Kernel role changes can finish asynchronously. Do not claim success
       until confirmed, or mislabel a pending transition as firmware failure. */
    return after==wanted?2:3;
}
