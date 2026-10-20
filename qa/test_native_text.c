#include "nxt_text.h"
#include <SDL2/SDL_image.h>
#include <utf8proc.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static const char *samples[]={
 "Tiếng Việt: Đường về nhà – Nguyễn Xuân Thông",
 "한국어 · 사랑의 노래 · 별빛 아래에서",
 "日本語 · 夜に駆ける · 漢字とひらがなカタカナ",
 "简体中文 · 月亮代表我的心 · 音乐与梦想",
 "繁體中文 · 愛與夢想 · 風中的聲音",
 "ภาษาไทย · เพลงรัก · น้ำตาและความฝัน",
 "ພາສາລາວ · ເພງຮັກ",
 "ខ្មែរ · ចម្រៀងស្នេហា",
 "हिन्दी · प्यार का गीत",
 "العربية · أغنية الحب",
 "漢字拡張 · 𠀀 𠮷 · ★ ♫ →",
 "Việt · 한국어 · 日本語 · 简体/繁體 · น้ำตา · العربية"};
int main(int argc,char **argv){assert(TTF_Init()==0);int tests=0;for(int pixels=24;pixels<=48;pixels+=12)for(int font=0;font<16;font++){
 TTF_Font *f=NxtText_open(NxtText_fontPath(font),pixels);assert(f);
 SDL_Surface *canvas=SDL_CreateRGBSurfaceWithFormat(0,2200,1100,32,SDL_PIXELFORMAT_ARGB8888);assert(canvas);SDL_FillRect(canvas,NULL,SDL_MapRGB(canvas->format,17,26,24));
 for(int i=0;i<12;i++){
  const unsigned char *p=(const unsigned char*)samples[i];while(*p){utf8proc_int32_t cp;int n=utf8proc_iterate(p,-1,&cp);assert(n>0);if(cp>32)assert(NxtText_hasGlyph(f,cp));p+=n;}
  int w=0,h=0;assert(NxtText_size(f,samples[i],&w,&h)==0&&w>0&&h>15&&h<pixels*3);
  SDL_Surface *text=NxtText_render(f,samples[i],(SDL_Color){157,228,182,255});assert(text&&text->w==w&&text->h==h);
  bool ink=false;for(int y=0;y<text->h;y++)for(int x=0;x<text->w;x++){Uint32 pixel=*(Uint32*)((char*)text->pixels+y*text->pitch+x*4);if(pixel>>24)ink=true;}assert(ink);
  SDL_BlitSurface(text,NULL,canvas,&(SDL_Rect){20,30+i*85,0,0});SDL_FreeSurface(text);tests++;
 }
 SDL_Surface *a=NxtText_render(f,"Tiếng Việt",(SDL_Color){255,255,255,255}),*b=NxtText_render(f,"Tiếng Việt",(SDL_Color){255,255,255,255});assert(a&&b&&a->w==b->w&&a->h==b->h);for(int y=0;y<a->h;y++)assert(!memcmp((char*)a->pixels+y*a->pitch,(char*)b->pixels+y*b->pitch,a->w*4));SDL_FreeSurface(a);SDL_FreeSurface(b);
 const char *cluster="ก้ข";assert(NxtText_nextCluster(cluster)==cluster+6);char *key=NxtText_searchKey("Đường VỀ NHÀ");assert(key&&!strcmp(key,"duong ve nha"));free(key);
 char file[256];snprintf(file,sizeof(file),"%s/font-%02d-%d.png",argc>1?argv[1]:"../qa/screens",font,pixels);assert(IMG_SavePNG(canvas,file)==0);SDL_FreeSurface(canvas);NxtText_close(f);
 }
 TTF_Quit();printf("PASS: %d font/script renders; 48 NFC pixel comparisons; glyph coverage, Thai clusters, Vietnamese search.\n",tests);return 0;}
