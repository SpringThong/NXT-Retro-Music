#include "nxt_lyric_preview.h"
#include "nxt_auralis.h"
#include "nxt_volume.h"
#include "nxt_performance.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * Native full-layout players: physical controls and geometry, no web runtime. */
#include "nxt_theme.h"
#include "nxt_custom.h"
#include "nxt_playback.h"
#include "nxt_record.h"
#include "nxt_wallpaper_store.h"
#include "player.h"
#include "album_art.h"
#include "lyrics.h"
#include "spectrum.h"
#include "settings.h"
#include "msettings.h"
#include "api.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nxt_v18_palette.inc"
static SDL_Color rgb(unsigned v){return(SDL_Color){v>>16,(v>>8)&255,v&255,255};}
const char *NxtTheme_name(int i){return names[i>=0&&i<NXT_THEME_COUNT?i:0];}
static bool header_palette;
void NxtTheme_header(bool active){header_palette=active;}
bool NxtTheme_color(NxtColor n,SDL_Color *out){int i=nxt.interface_style;if(i<=0||i>=NXT_THEME_COUNT)return false;*out=rgb(colors[i][n]);return true;}

int NxtTheme_radius(int px){int i=nxt.interface_style;return i==8||i==15||i==20||i==22?1:i==7?px*2:i==1||i==3||i==12?px/3:px;}
bool NxtTheme_linearHome(void){int i=nxt.interface_style;return i==2||i==5||i==8||i==10||i==14||i==15||i==18||i==20||i==22||i==38||i==41||i==43||i==46||i==48;}
static int u(SDL_Surface *s,int v){return(v*s->w+512)/1024;}
static void rect(SDL_Surface *s,int x,int y,int w,int h,SDL_Color c,int round){NxtSkin_rect(s,(SDL_Rect){u(s,x),u(s,y),u(s,w),u(s,h)},c,u(s,round));}
static void text(SDL_Surface *s,int px,const char *t,int x,int y,int w,SDL_Color c){NxtSkin_text(s,px,t,u(s,x),u(s,y),u(s,w),c);}
static void circle(SDL_Surface *s,int cx,int cy,int rad,SDL_Color c){int r=u(s,rad);NxtSkin_rect(s,(SDL_Rect){u(s,cx)-r,u(s,cy)-r,2*r,2*r},c,r);}
static void line(SDL_Surface *s,int x,int y,int ex,int ey,SDL_Color c,int thick){NxtSkin_line(s,u(s,x),u(s,y),u(s,ex),u(s,ey),c,u(s,thick));}
static void grille(SDL_Surface *s,int x,int y,int w,int h,int gap){for(int i=0;i<h;i+=gap)rect(s,x,y+i,w,2,NxtSkin_color(NXT_LINE),0);}
static void cover(SDL_Surface *s,SDL_Surface *art,int x,int y,int edge){SDL_Rect d={u(s,x),u(s,y),u(s,edge),u(s,edge)};NxtSkin_rect(s,d,NxtSkin_color(NXT_PANEL),u(s,8));if(art&&art->w>0&&art->h>0){int w=d.w,h=d.h;if(art->w>art->h)h=(int)((long long)w*art->h/art->w);else w=(int)((long long)h*art->w/art->h);d.x+=(d.w-w)/2;d.y+=(d.h-h)/2;d.w=w;d.h=h;NxtSkin_cachedArt(s,art,d);}else NxtSkin_icon(s,"music-2",u(s,x+edge/3),u(s,y+edge/3),u(s,edge/3),NxtSkin_color(NXT_ACCENT));}
static float angle;static uint32_t at;static bool advancing;
static float phase(bool playing){uint32_t now=SDL_GetTicks();if(at&&advancing&&!PWR_getDisplayOff()&&!NxtSession_isHidden()){uint32_t dt=now-at;if(dt<400)angle=fmodf(angle+dt*.09f,360);}at=now;advancing=playing&&nxt.animations;return angle*(float)M_PI/180.f;}
static void reel(SDL_Surface *s,int x,int y,int radius,float p,bool tape){SDL_Color ink=NxtSkin_color(NXT_INK),muted=NxtSkin_color(NXT_MUTED),panel=NxtSkin_color(NXT_PANEL);circle(s,x,y,radius,muted);circle(s,x,y,radius-4,panel);if(tape){circle(s,x,y,radius-11,rgb(0x231e1a));circle(s,x,y,radius/2,muted);}for(int i=0;i<3;i++){float a=p+i*(float)M_PI*2/3;line(s,x,y,x+cosf(a)*(radius-9),y+sinf(a)*(radius-9),ink,7);}circle(s,x,y,9,panel);circle(s,x,y,3,ink);}
static void dial(SDL_Surface *s,int x,int y,int radius,float a){circle(s,x,y,radius,NxtSkin_color(NXT_LINE));circle(s,x,y,radius-4,NxtSkin_color(NXT_PANEL));line(s,x,y,x+cosf(a)*(radius-10),y+sinf(a)*(radius-10),NxtSkin_color(NXT_ACCENT),3);}
static void keys(SDL_Surface *s,int x,int y,int w,int h,int count){const char *symbols[]={"skip-back","play","pause","skip-forward","square"};for(int i=0;i<count;i++){int width=w/count;rect(s,x+i*width,y,width-5,h,NxtSkin_color(NXT_LINE),3);rect(s,x+i*width+2,y+2,width-9,h-6,NxtSkin_color(NXT_PANEL),3);NxtSkin_icon(s,symbols[i%5],u(s,x+i*width+width/2-10),u(s,y+h/2-10),u(s,20),NxtSkin_color(NXT_INK));}}
void NxtTheme_chrome(SDL_Surface *s,bool home){int i=nxt.interface_style;if(!i)return;SDL_Color linec=NxtSkin_color(NXT_LINE),accent=NxtSkin_color(NXT_ACCENT);int h=s->h*1024/s->w;rect(s,12,12,1000,h-24,linec,NxtTheme_radius(20));rect(s,17,17,990,h-34,NxtSkin_color(NXT_BG),NxtTheme_radius(18));
 if(i==1||i==3||i==4||i==9||i==12||i==13||i==16||i==17){for(int k=0;k<4;k++){int x=k%2?989:35,y=k/2?h-40:36;circle(s,x,y,4,NxtSkin_color(NXT_MUTED));line(s,x-2,y,x+2,y,linec,1);}}
 if(i==2||i==12){grille(s,240,38,210,27,6);line(s,67,83,76,30,accent,2);}
 if(i==5||i==16){for(int k=0;k<22;k++)rect(s,230+k*8,42,3,19,linec,0);}
 if(i==7){rect(s,17,17,990,6,accent,2);rect(s,17,h-22,990,5,accent,2);}
 if(i==8||i==20){rect(s,20,77,984,2,accent,0);text(s,15,i==8?"NXT:// AUDIO TERMINAL":"TRACE / AUDIO",238,45,200,accent);}
 if(i==15){rect(s,18,77,988,3,linec,0);rect(s,18,h-96,988,3,linec,0);}
 if(i==19){for(int k=0;k<5;k++)line(s,850+k*22,129,985-k*20,80,linec,1);}
 if(i==23){for(int k=0;k<3;k++)rect(s,20+k*3,80+k*4,200,1,accent,0);}
 (void)home;
}
static const char *label(int i){static const char *en[]={"Library","Playlists","Favorites","Online music","Personal","Settings","About & updates"},*vi[]={"Thư viện","Playlist","Yêu thích","Nhạc online","Cá nhân","Cài đặt","Thông tin & cập nhật"};return Nxt_text(en[i],vi[i]);}
bool NxtTheme_home(SDL_Surface *s,int selected){int style=nxt.interface_style;if(!style)return false;SDL_Color panel=NxtSkin_color(NXT_PANEL),accent=NxtSkin_color(NXT_ACCENT),ink=NxtSkin_color(NXT_INK),muted=NxtSkin_color(NXT_MUTED);int top=149,h=s->h*1024/s->w;
 if(NxtTheme_linearHome()){
  /* Linear menus have matching up/down input semantics. */
  int left=style==2?392:style==18?374:style==5?340:style==14?293:style==10?210:style==22?230:style==8||style==20?274:244;
  rect(s,32,top,left-48,h-338,panel,NxtTheme_radius(17));NxtSkin_icon(s,NxtCustom_menuIcon(selected),u(s,62),u(s,210),u(s,left-109>130?130:left-109),accent);text(s,22,NxtTheme_name(style),52,383,left-72,ink);
  if(style==2){grille(s,52,439,left-87,129,7);dial(s,left/2,520,31,-1);}
  if(style==5){for(int k=0;k<4;k++){rect(s,50,448+k*33,left-82,25,NxtSkin_color(NXT_LINE),3);circle(s,left-60,460+k*33,7,accent);}}
  if(style==8||style==20){text(s,17,"> session: ready",49,459,left-76,accent);text(s,15,"source / output / eq",49,501,left-76,muted);}
  for(int i=0;i<7;i++){int row=(h-331)/7,y=top+i*row;rect(s,left,y,992-left,row-7,NxtSkin_color(i==selected?NXT_ACCENT:NXT_PANEL),NxtTheme_radius(style==2?20:8));SDL_Color c=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);NxtSkin_icon(s,NxtCustom_menuIcon(i),u(s,left+17),u(s,y+12),u(s,27),c);char t[128];snprintf(t,sizeof(t),style==8?"> %02d  %s":style==15?"[%d]  %s":"%02d   %s",i+1,label(i));text(s,24,t,left+60,y+8,904-left,c);}
 }else{
  /* Seven controls: six paired + a final full-width key, as on Original. */
  int base=top+84,gap=style==7?18:12,row=(h-331-84-3*gap)/4;
  if(style==1){rect(s,32,top,960,72,panel,6);reel(s,80,top+36,25,0,true);reel(s,944,top+36,25,0,true);text(s,24,"NXT • PERSONAL MIXTAPE",128,top+16,747,accent);}
  else if(style==4){reel(s,93,top+37,33,0,true);reel(s,932,top+37,33,0,true);line(s,125,top+62,900,top+62,muted,2);text(s,24,NxtTheme_name(style),160,top+8,700,ink);}
  else if(style==6||style==9||style==13){circle(s,73,top+35,31,muted);circle(s,73,top+35,14,panel);text(s,26,NxtTheme_name(style),127,top+7,800,ink);}
  else if(style==12){grille(s,32,top,138,63,5);grille(s,855,top,138,63,5);text(s,27,"NXT  •  BOOMBOX",221,top+8,617,accent);}
  else if(style==16||style==17){for(int k=0;k<4;k++){rect(s,39+k*53,top+4,30,54,accent,12);rect(s,45+k*53,top+10,18,40,panel,8);}text(s,26,NxtTheme_name(style),292,top+10,660,ink);}
  else if(style==19){for(int k=0;k<6;k++)line(s,32+k*192,top+63,512,top+10,NxtSkin_color(NXT_LINE),1);text(s,27,"N X T  /  SYNTH STUDIO",184,top+6,690,accent);}
  else if(style==23){rect(s,52,top+9,902,58,NxtSkin_color(NXT_LINE),24);rect(s,62,top,902,58,panel,24);text(s,24,NxtTheme_name(style),95,top+8,780,accent);}
  else text(s,27,NxtTheme_name(style),49,top+8,922,accent);
  for(int i=0;i<7;i++){int x=i==6?32:32+(i%2)*488,y=base+(i==6?3:i/2)*(row+gap),w=i==6?960:472;int round=NxtTheme_radius(style==7?21:style==11?22:style==21?17:10);rect(s,x,y,w,row-1,i==selected?accent:panel,round);if(style==1||style==3||style==12)rect(s,x+4,y+row-6,w-8,3,NxtSkin_color(NXT_LINE),1);SDL_Color c=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);NxtSkin_icon(s,NxtCustom_menuIcon(i),u(s,x+20),u(s,y+row/2-15),u(s,30),c);text(s,25,label(i),x+68,y+row/2-19,w-88,c);}
 }
 NxtSkin_mini(s);return true;
}
#include "nxt_theme_v18.inc"
ModuleExitReason NxtTheme_gallery(DisplayContext *d){int original=nxt.interface_style,selected=original,show=0;bool preview=false;uint32_t drawn=0;for(;;){ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);bool dirty=g.dirty;int old=selected;if(g.should_quit){nxt.interface_style=original;return MODULE_EXIT_QUIT;}if(!g.input_consumed&&!Nxt_lockInput()){if(NxtPad_justPressed(BTN_B)){if(preview){preview=false;drawn=0;}else{nxt.interface_style=original;return MODULE_EXIT_TO_MENU;}}if(NxtPad_justPressed(BTN_A)){nxt.interface_style=selected;Nxt_save();return MODULE_EXIT_TO_MENU;}if(NxtPad_justPressed(BTN_Y)){preview=!preview;drawn=0;}if(NxtPad_justRepeated(BTN_LEFT))selected=(selected+NXT_THEME_COUNT-1)%NXT_THEME_COUNT;if(NxtPad_justRepeated(BTN_RIGHT))selected=(selected+1)%NXT_THEME_COUNT;if(NxtPad_justRepeated(BTN_UP))selected=selected>=4?selected-4:NXT_THEME_COUNT-1-(NXT_THEME_COUNT-1-selected)%4;if(NxtPad_justRepeated(BTN_DOWN))selected=selected+4<NXT_THEME_COUNT?selected+4:selected%4;}
  if(NxtSkin_shouldDraw(drawn,dirty||old!=selected||(preview&&nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING&&(uint32_t)(SDL_GetTicks()-drawn)>=NxtPerformance_previewMS()))){nxt.interface_style=selected;if(preview)NxtSkin_playing(s,1,1,nxt.shuffle,nxt.repeat_mode);else{char title[160];snprintf(title,sizeof(title),"%s · %d / %d",NxtTheme_name(selected),selected+1,NXT_THEME_COUNT);NxtSkin_begin(s,Nxt_text("Theme collection","Bộ sưu tập giao diện"),title,false);int first=selected/12*12;for(int i=first;i<first+12&&i<NXT_THEME_COUNT;i++){int col=i%4,row=(i-first)/4,x=32+col*244,y=151+row*158;nxt.interface_style=i;SDL_Color c=NxtSkin_color(NXT_PANEL);rect(s,x,y,230,146,c,NxtTheme_radius(12));SDL_Color ac=NxtSkin_color(NXT_ACCENT);rect(s,x+8,y+8,214,6,ac,2);const char *sym=i==1||i==3||i==12?"cassette-tape":i==2?"radio":i==4||i==6||i==9||i==13||i==17?"disc-3":i==8||i==15||i==20?"monitor":i==14?"smartphone":i==5||i==16?"sliders-horizontal":i==18?"gauge":i==19?"moon":i==21?"layout-grid":i==22?"book-open":i==23?"sparkles":"headphones";NxtSkin_icon(s,sym,u(s,x+87),u(s,y+25),u(s,52),ac);bounded(s,19,NxtTheme_name(i),(Box){x+12,y+88,207,25},NxtSkin_color(NXT_INK));if(i==selected){rect(s,x,y+139,230,5,ac,1);bounded(s,14,Nxt_text("Selected","Đang chọn"),(Box){x+12,y+116,206,19},ac);}}nxt.interface_style=selected;NxtSkin_footer(s,Nxt_text("D-pad Browse · Y Preview · A Apply · B Back","D-pad Chọn · Y Xem trước · A Áp dụng · B Về"),false);}ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(s);
 }
}
