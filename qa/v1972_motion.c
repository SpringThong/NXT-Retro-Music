#include "nxt.h"
#include "nxt_motion.h"
#include <assert.h>
#include <stdio.h>
NxtConfig nxt;static Uint32 ticks;static bool off,hidden;
Uint32 SDL_GetTicks(void){return ticks;}
int PWR_getDisplayOff(void){return off;}
bool NxtSession_isHidden(void){return hidden;}
static unsigned red(SDL_Surface*s){Uint8 r,g,b;SDL_GetRGB(*(Uint32*)s->pixels,s->format,&r,&g,&b);return r;}
static void paint(SDL_Surface*s,int r){SDL_FillRect(s,NULL,SDL_MapRGB(s->format,r,0,0));}
int main(void){assert(!SDL_Init(0));SDL_Surface *s=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);assert(s);nxt.animations=1;
 paint(s,20);NxtMotion_page(s,"menu",true);assert(!NxtMotion_frame(s,true));ticks=1000;NxtMotion_page(s,"settings",true);paint(s,220);assert(NxtMotion_frame(s,true));assert(red(s)==220);ticks+=64;assert(NxtMotion_frame(s,false));ticks+=64;assert(NxtMotion_frame(s,false)||!NxtMotion_frame(s,false));assert(red(s)==220);
 NxtMotion_input(s);paint(s,50);assert(!NxtMotion_frame(s,true));assert(red(s)==50); /* Immediate selection, never two labels. */
 NxtMotion_page(s,"other",true);paint(s,140);assert(NxtMotion_frame(s,true));NxtMotion_input(s);paint(s,70);assert(!NxtMotion_frame(s,true)&&red(s)==70);assert(!NxtMotion_frame(s,false));
 NxtMotion_page(s,"off",false);paint(s,180);assert(NxtMotion_frame(s,true));off=true;assert(NxtMotion_frame(s,false)&&red(s)==180);assert(!NxtMotion_frame(s,false));off=false;
 hidden=true;NxtMotion_page(s,"hidden",true);paint(s,80);assert(!NxtMotion_frame(s,true));hidden=false;ticks=UINT32_MAX-50;NxtMotion_page(s,"wrap",true);paint(s,99);assert(NxtMotion_frame(s,true));ticks+=130;assert(NxtMotion_frame(s,false)&&red(s)==99);assert(!NxtMotion_frame(s,false));
 NxtMotion_release();SDL_FreeSurface(s);SDL_Quit();puts("PASS opaque page entrance, immediate selection cancellation, no idle frames, screen-off restore, hidden and tick wrap.");}
