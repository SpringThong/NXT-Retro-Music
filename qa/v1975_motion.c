#include "nxt.h"
#include "nxt_motion.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
NxtConfig nxt;static Uint32 ticks;static bool off,hidden;
Uint32 SDL_GetTicks(void){return ticks;}int PWR_getDisplayOff(void){return off;}bool NxtSession_isHidden(void){return hidden;}
int main(void){assert(!SDL_Init(0));
 for(int size=0;size<2;size++){
  SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,size?640:1024,size?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(s);size_t n=s->pitch*s->h;void*copy=malloc(n);assert(copy);
  NxtMotion_release();
  for(int theme=0;theme<51;theme++)for(int page=0;page<8;page++){
   nxt.interface_style=theme;nxt.animations=1;off=hidden=false;SDL_SetClipRect(s,NULL);
   SDL_FillRect(s,NULL,SDL_MapRGBA(s->format,(page*47)%256,(theme*17)%256,89,page&1?127:255));
   SDL_FillRect(s,&(SDL_Rect){19,31,67,43},SDL_MapRGBA(s->format,210,90,43,255));memcpy(copy,s->pixels,n);
   const char *names[]={"home","settings","lyrics","queue","profile","about","library","now-playing"};NxtMotion_page(s,names[page],page!=2&&page!=7);
   for(int frame=0;frame<10;frame++){ticks+=17;assert(!NxtMotion_frame(s,frame==0));assert(!memcmp(copy,s->pixels,n));}
   SDL_SetClipRect(s,&(SDL_Rect){17,29,33,41});NxtMotion_input(s);NxtMotion_setMenu(false);off=hidden=true;ticks=UINT32_MAX-7;assert(!NxtMotion_frame(s,false));ticks+=100;assert(!NxtMotion_frame(s,false));assert(!memcmp(copy,s->pixels,n));
  }
  free(copy);SDL_FreeSurface(s);
 }
 NxtMotion_release();SDL_Quit();puts("PASS no full-page displacement/snapshot/restore for 51 themes, 8 page classes, repeated frames, clipped/translucent surfaces, hidden/off, and tick wrap at both native sizes.");
}
