#include "nxt_write.h"
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void read_text(const char*p,char*out,size_t n){FILE*f=fopen(p,"r");assert(f);size_t z=fread(out,1,n-1,f);out[z]=0;assert(!ferror(f));fclose(f);}
int main(int argc,char**argv){assert(argc==2);char p[1500],other[1500],im[1500],bad[1500],text[64],out[64];snprintf(p,sizeof(p),"%s/listener-a.json",argv[1]);snprintf(other,sizeof(other),"%s/listener-b.json",argv[1]);snprintf(im,sizeof(im),"%s/album.png",argv[1]);snprintf(bad,sizeof(bad),"%s/missing/invalid.json",argv[1]);
 assert(!SDL_Init(0));assert(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG);
 for(int i=0;i<200;i++){snprintf(text,sizeof(text),"{\"position_ms\":%d}",i);assert(NxtWrite_text(p,text,true));}
 assert(NxtWrite_text(other,"profile B snapshot",true));NxtWrite_flush();read_text(p,out,sizeof(out));assert(!strcmp(out,"{\"position_ms\":199}"));read_text(other,out,sizeof(out));assert(!strcmp(out,"profile B snapshot"));
 SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_ARGB8888);assert(s);SDL_FillRect(s,NULL,SDL_MapRGB(s->format,12,34,250));assert(NxtWrite_image(im,s));SDL_FillRect(s,NULL,0);SDL_FreeSurface(s);NxtWrite_flush();s=IMG_Load(im);assert(s&&s->w==640&&s->h==480);Uint8 r,g,b;Uint32 pixel=*(Uint32*)s->pixels;SDL_GetRGB(pixel,s->format,&r,&g,&b);assert(r==12&&g==34&&b==250);SDL_FreeSurface(s);
 assert(NxtWrite_remove(im));NxtWrite_flush();assert(access(im,F_OK)<0);assert(!NxtWrite_failed());
 assert(NxtWrite_text(bad,"must fail",true));NxtWrite_flush();assert(NxtWrite_failed()&&!NxtWrite_failed());NxtWrite_quit();
 assert(NxtWrite_text(p,"quit drains final snapshot",true));NxtWrite_quit();read_text(p,out,sizeof(out));assert(!strcmp(out,"quit drains final snapshot"));IMG_Quit();SDL_Quit();
 puts("PASS actual background disk worker: path-coalescing latest snapshot, profile paths isolated, PNG owns pixels, ordered remove, SD failure surfaced, quit drains and restart works. No UI fsync/PNG compression.");
}
