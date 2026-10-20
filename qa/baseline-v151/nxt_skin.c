#include "nxt_appearance.h"
#include "nxt_personal.h"
#include "nxt_controls.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * Native rendering of the approved charcoal / mint NXT design. */
#include "nxt_skin.h"
#include "nxt_drive.h"
#include "nxt_playback.h"
#include "nxt_record.h"
#include "ui_theme.h"
#include "ui_layers.h"
#include "ui_fonts.h"
#include "config.h"
#include "api.h"
#include "player.h"
#include "module_player.h"
#include "album_art.h"
#include "lyrics.h"
#include "spectrum.h"
#include "settings.h"
#include "background.h"
#include "resume.h"
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int list_top;
static int unit(SDL_Surface *s,int n){return (n*s->w+512)/1024;}
SDL_Color NxtSkin_color(NxtColor n){
 SDL_Color system=uintToColour(CFG_getColor(COLOR_BACKGROUND));bool light=nxt.theme==2||(nxt.theme==0&&(system.r*299+system.g*587+system.b*114)>160000);
 static const SDL_Color dark[]={{17,26,24,255},{26,40,36,255},{234,244,237,255},{160,183,172,255},{48,66,58,255}};
 static const SDL_Color pale[]={{245,246,242,255},{232,236,230,255},{23,39,32,255},{81,102,91,255},{198,211,201,255}};
 const SDL_Color *palette=light?pale:dark;if(n<=NXT_LINE)return palette[n];
 unsigned rgb=NxtAppearance_accent();SDL_Color raw={(rgb>>16)&255,(rgb>>8)&255,rgb&255,255},target=light?(SDL_Color){0,0,0,255}:(SDL_Color){255,255,255,255},accent=raw;
 for(int percent=0;percent<=100;percent++){accent=Theme_mix(raw,target,percent);if(Theme_contrast(accent,palette[NXT_BG])>=4.5&&Theme_contrast(accent,palette[NXT_PANEL])>=4.5)break;}
 if(n==NXT_ACCENT)return accent;SDL_Color black={0,0,0,255},white={255,255,255,255};return Theme_contrast(accent,black)>=Theme_contrast(accent,white)?black:white;
}
typedef struct {SDL_Surface *image;SDL_Color color;int w,h,radius;unsigned used;} PanelCache;
static PanelCache panels[8];static unsigned panel_clock;
static void solid_rect(SDL_Surface *s,SDL_Rect r,SDL_Color color,int radius){
    Uint32 c=SDL_MapRGBA(s->format,color.r,color.g,color.b,color.a);
    radius=radius<r.h/2?radius:r.h/2;radius=radius<r.w/2?radius:r.w/2;
    if(radius<1){SDL_FillRect(s,&r,c);return;}
    SDL_FillRect(s,&(SDL_Rect){r.x,r.y+radius,r.w,r.h-2*radius},c);
    for(int y=0;y<radius;y++){int inset=radius-(int)sqrt((double)radius*radius-(double)(radius-y-1)*(radius-y-1));SDL_FillRect(s,&(SDL_Rect){r.x+inset,r.y+y,r.w-2*inset,1},c);SDL_FillRect(s,&(SDL_Rect){r.x+inset,r.y+r.h-y-1,r.w-2*inset,1},c);}
}
void NxtSkin_rect(SDL_Surface *s,SDL_Rect r,SDL_Color color,int radius){
    if(!s||r.w<=0||r.h<=0||!color.a)return;
    if(color.a==255){solid_rect(s,r,color,radius);return;}
    PanelCache *cache=NULL,*oldest=&panels[0];
    for(int i=0;i<8;i++){PanelCache *p=&panels[i];if(p->image&&p->w==r.w&&p->h==r.h&&p->radius==radius&&!memcmp(&p->color,&color,sizeof(color))){cache=p;break;}if(!p->image||p->used<oldest->used)oldest=p;}
    if(!cache){cache=oldest;if(cache->image)SDL_FreeSurface(cache->image);memset(cache,0,sizeof(*cache));
        cache->image=SDL_CreateRGBSurfaceWithFormat(0,r.w,r.h,32,SDL_PIXELFORMAT_ARGB8888);if(!cache->image)return;
        cache->w=r.w;cache->h=r.h;cache->radius=radius;cache->color=color;
        SDL_FillRect(cache->image,NULL,0);solid_rect(cache->image,(SDL_Rect){0,0,r.w,r.h},color,radius);SDL_SetSurfaceBlendMode(cache->image,SDL_BLENDMODE_BLEND);
    }
    cache->used=++panel_clock;SDL_BlitSurface(cache->image,NULL,s,&r);
}
void NxtSkin_panel(SDL_Surface *s,SDL_Rect r,SDL_Color color,int radius){
    if(nxt.wallpaper_mode)color.a=nxt.panel_opacity*255/100;
    NxtSkin_rect(s,r,color,radius);
}
/* Render white glyphs once, tint the outline and then the actual text. This
 * protects text over photos without placing an opaque card over the photo. */
void NxtSkin_blitText(SDL_Surface *s,SDL_Surface *text,SDL_Rect src,int x,int y,SDL_Color color){
    if(nxt.wallpaper_mode){int d=unit(s,1);if(d<1)d=1;int light=color.r*299+color.g*587+color.b*114>140000;Uint8 shadow=light?0:255;
        SDL_SetSurfaceColorMod(text,shadow,shadow,shadow);SDL_SetSurfaceAlphaMod(text,210);
        SDL_BlitSurface(text,&src,s,&(SDL_Rect){x-d,y,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x+d,y,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y-d,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y+d,0,0});
    }
    SDL_SetSurfaceColorMod(text,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(text,color.a);SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y,0,0});
}
typedef struct {char name[48];SDL_Surface *image;} Icon;
static Icon icons[80];static int nicons;
static NxtSkinSource screen_source=NXT_SOURCE_AUTO;
static SDL_Surface *source_logos[4];
NxtSkinSource NxtSkin_setSource(NxtSkinSource source){
    NxtSkinSource previous=screen_source;screen_source=source;return previous;
}
static NxtSkinSource playback_source(void){
    if(NxtDrive_isPath(Player_getCurrentFile())&&Player_getState()!=PLAYER_STATE_STOPPED)return NXT_SOURCE_DRIVE;
    if(NxtSoundCloud_hasMetadata())return NXT_SOURCE_SOUNDCLOUD;
    if(NxtSpotify_hasMetadata())return NXT_SOURCE_SPOTIFY;
    return NXT_SOURCE_AUTO;
}
static NxtSkinSource header_source(const char *title){
    /* The badge identifies the current track even when browsing another source. */
    NxtSkinSource playing=playback_source();
    if(playing!=NXT_SOURCE_AUTO)return playing;
    if(title&&strstr(title,"Google Drive"))return NXT_SOURCE_DRIVE;
    if(title&&strstr(title,"Spotify"))return NXT_SOURCE_SPOTIFY;
    if(title&&strstr(title,"SoundCloud"))return NXT_SOURCE_SOUNDCLOUD;
    return screen_source;
}
static void source_brand(SDL_Surface *s,NxtSkinSource source,int x,int y){
    if(source==NXT_SOURCE_DRIVE){
        int w=unit(s,152),h=unit(s,42),pad=unit(s,12);
        NxtSkin_rect(s,(SDL_Rect){x-pad,y-pad,w+pad*2,h+pad*2},(SDL_Color){18,18,18,255},unit(s,8));
        NxtSkin_text(s,22,"Google Drive",x,y,w,(SDL_Color){255,255,255,255});return;
    }
    const char *paths[]={NULL,"./res/icons/spotify-source.png","./res/icons/soundcloud-source.png"};
    if(source!=NXT_SOURCE_SPOTIFY&&source!=NXT_SOURCE_SOUNDCLOUD)return;
    if(!source_logos[source])source_logos[source]=IMG_Load(paths[source]);
    SDL_Surface *image=source_logos[source];if(!image)return;
    int w=unit(s,152),h=w*image->h/image->w,pad=unit(s,12);
    NxtSkin_rect(s,(SDL_Rect){x-pad,y-pad,w+pad*2,h+pad*2},(SDL_Color){18,18,18,255},unit(s,8));
    SDL_BlitScaled(image,NULL,s,&(SDL_Rect){x,y,w,h});
}
void NxtSkin_icon(SDL_Surface *s,const char *name,int x,int y,int size,SDL_Color color){
    if(!name||!size)return;SDL_Surface *image=NULL;
    for(int i=0;i<nicons;i++)if(!strcmp(icons[i].name,name)){image=icons[i].image;break;}
    if(!image&&nicons<80){char path[160];snprintf(path,sizeof(path),"./res/icons/%s.png",name);image=IMG_Load(path);if(image){snprintf(icons[nicons].name,48,"%s",name);icons[nicons++].image=image;}}
    if(!image)return;SDL_SetSurfaceColorMod(image,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(image,color.a);SDL_BlitScaled(image,NULL,s,&(SDL_Rect){x,y,size,size});
}
static TTF_Font *fonts[18];static int sizes[18],nf,signature;
TTF_Font *NxtSkin_font(SDL_Surface *s,int pixels){
    int sig=s->w*10000+nxt.font_index*200+nxt.font_size;
    if(sig!=signature){for(int i=0;i<nf;i++)TTF_CloseFont(fonts[i]);nf=0;signature=sig;}
    pixels=unit(s,pixels);if(pixels<12)pixels=12;
    for(int i=0;i<nf;i++)if(sizes[i]==pixels)return fonts[i];
    if(nf>=18)return fonts[0];TTF_Font *f=Fonts_open(pixels);if(f){sizes[nf]=pixels;fonts[nf++]=f;}return f;
}
void NxtSkin_text(SDL_Surface *s,int pixels,const char *text,int x,int y,int width,SDL_Color color){
    if(!text||width<=0)return;SDL_Surface *rendered=NxtText_render(NxtSkin_font(s,pixels),text,(SDL_Color){255,255,255,255});if(!rendered)return;
    int offset=0;if(rendered->w>width){int distance=rendered->w-width,speed=unit(s,35);if(speed<1)speed=1;int duration=distance*1000/speed,phase=SDL_GetTicks()%(duration+4000);if(phase>1800)offset=(phase-1800)*speed/1000;if(offset>distance)offset=distance;}
    NxtSkin_blitText(s,rendered,(SDL_Rect){offset,0,width<rendered->w?width:rendered->w,rendered->h},x,y,color);SDL_FreeSurface(rendered);
}
void NxtSkin_release(void){for(int i=0;i<8;i++){if(panels[i].image)SDL_FreeSurface(panels[i].image);memset(&panels[i],0,sizeof(panels[i]));}panel_clock=0;NxtAppearance_release();NxtRecord_release();for(int i=0;i<nf;i++)TTF_CloseFont(fonts[i]);nf=0;signature=0;for(int i=0;i<nicons;i++)SDL_FreeSurface(icons[i].image);nicons=0;for(int i=0;i<4;i++){if(source_logos[i])SDL_FreeSurface(source_logos[i]);source_logos[i]=NULL;}screen_source=NXT_SOURCE_AUTO;}
int NxtSkin_status(SDL_Surface *s){
    int p=unit(s,32),y=unit(s,24);
    int charge=0,battery=0;PLAT_getBatteryStatusFine(&charge,&battery);char pct[20],clock[20];snprintf(pct,20,"%d%%",battery);
    time_t now=time(NULL);struct tm local;localtime_r(&now,&local);strftime(clock,20,"%H:%M",&local);
    int right=s->w-p;

    if(battery<0)battery=0;if(battery>100)battery=100;snprintf(pct,sizeof(pct),"%d%%",battery);
    SDL_Rect box={right-unit(s,87),y+unit(s,8),unit(s,82),unit(s,32)};
    NxtSkin_rect(s,box,NxtSkin_color(NXT_MUTED),unit(s,3));NxtSkin_rect(s,(SDL_Rect){box.x+unit(s,2),box.y+unit(s,2),box.w-unit(s,4),box.h-unit(s,4)},NxtSkin_color(NXT_BG),unit(s,2));NxtSkin_rect(s,(SDL_Rect){box.x+box.w,box.y+unit(s,10),unit(s,5),unit(s,12)},NxtSkin_color(NXT_MUTED),unit(s,1));
    SDL_Surface *percent=NxtText_render(NxtSkin_font(s,18),pct,NxtSkin_color(NXT_INK));if(percent){SDL_BlitSurface(percent,NULL,s,&(SDL_Rect){box.x+(box.w-percent->w)/2,box.y+(box.h-percent->h)/2,0,0});SDL_FreeSurface(percent);}if(charge)NxtSkin_icon(s,"zap",box.x-unit(s,25),y+unit(s,10),unit(s,22),NxtSkin_color(NXT_ACCENT));
    int status_x=right-unit(s,charge?155:130),step=unit(s,41);
    if(Nxt_isLocked()){NxtSkin_icon(s,"lock-keyhole",status_x,y+unit(s,10),unit(s,26),NxtSkin_color(NXT_ACCENT));status_x-=step;}
    if(NxtHardware_hasDAC()){NxtSkin_icon(s,"usb",status_x,y+unit(s,10),unit(s,26),NxtSkin_color(NXT_ACCENT));status_x-=step;}
    if(NxtHardware_hasJack()){NxtSkin_icon(s,"headphones",status_x,y+unit(s,10),unit(s,26),NxtSkin_color(NXT_ACCENT));status_x-=step;}
    if(PLAT_btIsConnected()){NxtSkin_icon(s,"bluetooth",status_x,y+unit(s,10),unit(s,26),NxtSkin_color(NXT_ACCENT));status_x-=step;}
    NxtSkin_icon(s,PLAT_wifiConnected()?"wifi":"wifi-off",status_x,y+unit(s,10),unit(s,26),NxtSkin_color(NXT_MUTED));
    NxtSkin_text(s,20,clock,status_x-unit(s,83),y+unit(s,7),unit(s,70),NxtSkin_color(NXT_MUTED));
    return s->w-(status_x-unit(s,83))+p;
}
void NxtSkin_begin(SDL_Surface *s,const char *title,const char *detail,bool mini){
    list_top=0;NxtRecord_hide();
    GFX_clear(s);UiLayer_clear(UI_LAYER_STATUS);UiLayer_clear(UI_LAYER_ANIMATION);UiLayer_clear(UI_LAYER_SELECTION);
    NxtAppearance_background(s);
    NxtSkin_panel(s,(SDL_Rect){0,0,s->w,unit(s,132)},NxtSkin_color(NXT_BG),0);
    NxtSkin_panel(s,(SDL_Rect){0,s->h-unit(s,178),s->w,unit(s,178)},NxtSkin_color(NXT_BG),0);
    bool home=title&&(!strncmp(title,"Chào ",6)||!strncmp(title,"Hello ",6));
    if(!home){SDL_Color body=NxtSkin_color(NXT_BG);if(nxt.wallpaper_mode)body.a=nxt.panel_opacity*255/300;NxtSkin_rect(s,(SDL_Rect){unit(s,22),unit(s,142),s->w-unit(s,44),s->h-unit(s,242)},body,unit(s,12));}
    int p=unit(s,32),y=unit(s,24),logo=unit(s,46);
    NxtSkin_rect(s,(SDL_Rect){p,y,logo,logo},NxtSkin_color(NXT_PANEL),unit(s,12));
    NxtSkin_icon(s,home?NxtPersonal_avatar():"headphones",p+unit(s,6),y+unit(s,5),unit(s,34),NxtSkin_color(NXT_ACCENT));
    if(!home)for(int i=0;i<3;i++)NxtSkin_rect(s,(SDL_Rect){p+unit(s,17+i*5),y+unit(s,33-(i==1?13:i==0?7:10)),unit(s,3),unit(s,i==1?13:i==0?7:10)},NxtSkin_color(NXT_ACCENT),1);
    NxtSkin_text(s,24,NXT_APP_NAME,p+logo+unit(s,14),y+unit(s,2),s->w/2,NxtSkin_color(NXT_INK));
    NxtSkin_status(s);
    NxtSkinSource source=header_source(title);
    int brand_x=s->w-p-unit(s,164)-(detail&&*detail?unit(s,110):0);
    if(title)NxtSkin_text(s,32,title,p,unit(s,87),source!=NXT_SOURCE_AUTO?brand_x-p-unit(s,32):s->w-p*2,NxtSkin_color(NXT_INK));
    if(source!=NXT_SOURCE_AUTO)source_brand(s,source,brand_x,unit(s,88));
    if(detail&&*detail){int w=0;TTF_SizeUTF8(NxtSkin_font(s,19),detail,&w,NULL);NxtSkin_text(s,19,detail,s->w-p-w,unit(s,94),w,NxtSkin_color(NXT_MUTED));}
    if(mini)NxtSkin_mini(s);
}
int NxtSkin_top(SDL_Surface *s){return unit(s,145);}
int NxtSkin_bottom(SDL_Surface *s,bool mini){return s->h-unit(s,mini?178:101);}
/* Align the visible glyphs, including Vietnamese marks, rather than a font's
 * transparent ascent/descent padding. Compact hints must fit without scrolling. */
static SDL_Surface *footer_text(SDL_Surface *s,const char *text,int pixels,SDL_Color color,int width,int height,SDL_Rect *ink,int *ascent){
    for(;pixels>=12;pixels--){
        TTF_Font *font=NxtSkin_font(s,pixels);SDL_Surface *rendered=NxtText_render(font,text,color);if(!rendered)return NULL;
        if(SDL_LockSurface(rendered)<0){SDL_FreeSurface(rendered);return NULL;}
        int left=rendered->w,top=rendered->h,right=-1,bottom=-1;
        for(int y=0;y<rendered->h;y++){
            const Uint32 *row=(const Uint32 *)((const Uint8 *)rendered->pixels+y*rendered->pitch);
            for(int x=0;x<rendered->w;x++)if(row[x]&rendered->format->Amask){
                if(x<left)left=x;if(x>right)right=x;if(y<top)top=y;if(y>bottom)bottom=y;
            }
        }
        SDL_UnlockSurface(rendered);
        *ink=(SDL_Rect){left,top,right-left+1,bottom-top+1};
        if(ink->w>0&&ink->h>0&&ink->w<=width&&ink->h<=height){if(ascent)*ascent=TTF_FontAscent(font);return rendered;}
        SDL_FreeSurface(rendered);
    }
    return NULL;
}
void NxtSkin_footer(SDL_Surface *s,const char *hint,bool player){
    int p=unit(s,32),y=s->h-unit(s,91);NxtSkin_rect(s,(SDL_Rect){p,y,s->w-2*p,1},NxtSkin_color(NXT_LINE),0);
    if(!player){NxtSkin_text(s,18,"B  ←",p,y+unit(s,19),unit(s,66),NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,19,hint?hint:Nxt_text("A Select","A Chọn"),p+unit(s,80),y+unit(s,17),s->w-p*2-unit(s,80),NxtSkin_color(NXT_MUTED));return;}
    bool playing=NxtPlayback_state()==PLAYER_STATE_PLAYING;
    const int actions[]={NC_PREV,NC_PLAY,NC_NEXT,NC_QUEUE,NC_LYRICS,NC_EQ,NC_QUICK};
    const char *labels[]={"L1","A","R1","X","Y","SELECT","START"};
    const char *symbol[]={"skip-back",playing?"pause":"play","skip-forward","list-music","captions","sliders-horizontal","ellipsis"};
    const char *caption[]={Nxt_text("Previous","Trước"),playing?Nxt_text("Pause","Tạm dừng"):Nxt_text("Play","Phát"),Nxt_text("Next","Tiếp"),Nxt_text("Queue","Danh sách"),Nxt_text("Lyrics","Lời"),"EQ",Nxt_text("Options","Tùy chọn")};
    int gap=unit(s,10),available=s->w-2*p-6*gap,icon=unit(s,25),inside=unit(s,8);
    for(int i=0;i<7;i++){
        /* Integer partitions distribute every pixel without cumulative drift. */
        int x=p+i*gap+available*i/7,w=available*(i+1)/7-available*i/7,cx=x+w/2;
        NxtSkin_rect(s,(SDL_Rect){x,y+unit(s,13),w,unit(s,64)},NxtSkin_color(i==1?NXT_ACCENT:NXT_PANEL),unit(s,12));
        SDL_Color c=NxtSkin_color(i==1?NXT_ACCENT_INK:NXT_INK),muted=NxtSkin_color(i==1?NXT_ACCENT_INK:NXT_MUTED);
        const char *key="—";for(int k=0;k<16;k++)if(nxt.playing_keys[k]==actions[i]){key=Nxt_keyNames[k];break;}
        SDL_Rect ink;int key_y=y+unit(s,31),space=unit(s,9);
        SDL_Surface *text=footer_text(s,key,15,c,w-2*inside-icon-space,unit(s,22),&ink,NULL);
        int key_w=text?ink.w:0,group_x=cx-(key_w+space+icon)/2;
        if(text){SDL_BlitSurface(text,&ink,s,&(SDL_Rect){group_x,key_y-ink.h/2,0,0});SDL_FreeSurface(text);}
        NxtSkin_icon(s,symbol[i],group_x+key_w+space,key_y-icon/2,icon,c);
        int ascent=0;text=footer_text(s,caption[i],18,muted,w-2*inside,unit(s,25),&ink,&ascent);
        if(text){SDL_BlitSurface(text,&ink,s,&(SDL_Rect){cx-ink.w/2,y+unit(s,67)-ascent+ink.y,0,0});SDL_FreeSurface(text);}
    }
}
void NxtSkin_mini(SDL_Surface *s){
    int p=unit(s,32),y=s->h-unit(s,176),h=unit(s,73);
    NxtSkin_panel(s,(SDL_Rect){p,y,s->w-p*2,h},NxtSkin_color(NXT_PANEL),unit(s,12));
    NxtSkin_icon(s,"music-2",p+unit(s,18),y+unit(s,24),unit(s,28),NxtSkin_color(NXT_ACCENT));
    PlayerState state=NxtPlayback_state();bool active=state!=PLAYER_STATE_STOPPED;
    const TrackInfo *info=NxtPlayback_info();const ResumeState *resume=Resume_getState();
    const char *title=active&&info->title[0]?info->title:resume&&resume->track_name[0]?resume->track_name:Nxt_text("Choose a track in Library","Chọn bài hát trong Thư viện");NxtSkin_text(s,23,title,p+unit(s,72),y+unit(s,7),s->w-p*2-unit(s,220),NxtSkin_color(NXT_INK));
    NxtSkin_text(s,17,active&&info->artist[0]?info->artist:Nxt_text("Y · Now playing / resume","Y · Đang phát / nghe tiếp"),p+unit(s,72),y+unit(s,40),s->w-p*2-unit(s,220),NxtSkin_color(NXT_MUTED));
    int pos=(active?NxtPlayback_position():resume?resume->position_ms:0)/1000;char time[32];snprintf(time,32,"%02d:%02d",pos/60,pos%60);NxtSkin_text(s,19,time,s->w-p-unit(s,121),y+unit(s,23),unit(s,75),NxtSkin_color(NXT_MUTED));
    NxtSkin_icon(s,state==PLAYER_STATE_PLAYING?"pause":"play",s->w-p-unit(s,44),y+unit(s,23),unit(s,26),NxtSkin_color(NXT_INK));
}
void NxtSkin_list(SDL_Surface *s,const char *const *items,int count,int selected,bool mini){
    int titleheight=0,subheight=0;TTF_SizeUTF8(NxtSkin_font(s,25*nxt.font_size/100),"Tiếng Việt ไทย語 한국어",NULL,&titleheight);TTF_SizeUTF8(NxtSkin_font(s,18),"Tiếng Việt ไทย語 한국어",NULL,&subheight);int p=unit(s,32),top=list_top>0?list_top:NxtSkin_top(s),bottom=NxtSkin_bottom(s,mini),row=unit(s,78)*nxt.font_size/100;int minrow=titleheight+subheight+unit(s,21);if(row<minrow)row=minrow;list_top=0;
    int visible=(bottom-top)/row;if(visible<1)visible=1;int first=selected-visible/2;if(first<0)first=0;if(first>count-visible)first=count-visible;if(first<0)first=0;
    for(int i=first;i<count&&i<first+visible;i++){
        int y=top+(i-first)*row;bool chosen=i==selected;
        if(chosen){NxtSkin_panel(s,(SDL_Rect){p,y,s->w-p*2,row-unit(s,5)},NxtSkin_color(NXT_PANEL),unit(s,11));NxtSkin_rect(s,(SDL_Rect){p,y+unit(s,12),unit(s,3),row-unit(s,29)},NxtSkin_color(NXT_ACCENT),1);}
        const char *text=items[i]?items[i]:"";const char *line=strchr(text,'\n'),*value=strchr(text,'\t');char title[2048];size_t len=line?(size_t)(line-text):strlen(text);if(value&&(size_t)(value-text)<len)len=value-text;if(len>=sizeof(title))len=sizeof(title)-1;memcpy(title,text,len);title[len]=0;
        int texty=y+(line?unit(s,8):(row-titleheight)/2),labelwidth=value?(s->w-p*2)*55/100:s->w-p*2-unit(s,68);NxtSkin_text(s,25*nxt.font_size/100,title,p+unit(s,19),texty,labelwidth,NxtSkin_color(NXT_INK));
        if(value){char val[1024];snprintf(val,sizeof(val),"%s",value+1);char *newline=strchr(val,'\n');if(newline)*newline=0;int vw=0;TTF_SizeUTF8(NxtSkin_font(s,21),val,&vw,NULL);int limit=(s->w-p*2)*40/100;if(vw>limit)vw=limit;NxtSkin_text(s,21,val,s->w-p-unit(s,43)-vw,texty+unit(s,3),vw,NxtSkin_color(NXT_ACCENT));}
        if(line)NxtSkin_text(s,18,line+1,p+unit(s,19),y+unit(s,11)+titleheight,s->w-p*2-unit(s,68),NxtSkin_color(NXT_MUTED));
        NxtSkin_icon(s,"chevron-right",s->w-p-unit(s,34),y+row/2-unit(s,10),unit(s,21),NxtSkin_color(chosen?NXT_ACCENT_INK:NXT_MUTED));
        if(!chosen)NxtSkin_rect(s,(SDL_Rect){p+unit(s,14),y+row-unit(s,4),s->w-p*2-unit(s,28),1},NxtSkin_color(NXT_LINE),0);
    }
    if(count>visible){int trackh=bottom-top,thumb=trackh*visible/count;if(thumb<unit(s,14))thumb=unit(s,14);NxtSkin_rect(s,(SDL_Rect){s->w-p+unit(s,13),top,unit(s,3),trackh},NxtSkin_color(NXT_PANEL),1);NxtSkin_rect(s,(SDL_Rect){s->w-p+unit(s,13),top+(trackh-thumb)*first/(count-visible),unit(s,3),thumb},NxtSkin_color(NXT_ACCENT),1);}
    if(!count)NxtSkin_text(s,25,Nxt_text("No items yet.","Chưa có nội dung."),p+unit(s,14),top+unit(s,20),s->w-p*2,NxtSkin_color(NXT_MUTED));
}
void NxtSkin_home(SDL_Surface *s,int selected){
 char greeting[180];snprintf(greeting,sizeof(greeting),"%s%s",Nxt_text("Hello ","Chào "),NxtPersonal_name());NxtSkin_begin(s,greeting,NULL,true);
 const char *labels[]={Nxt_text("Library","Thư viện"),"Playlist",Nxt_text("Favorites","Yêu thích"),Nxt_text("Online music","Nhạc online"),Nxt_text("Personal","Cá nhân"),Nxt_text("Settings","Cài đặt"),Nxt_text("About, help & updates","Thông tin, hướng dẫn & cập nhật")};
 const char *subs[]={Nxt_text("Folders · artists · albums","Thư mục · nghệ sĩ · album"),Nxt_text("Create and edit playlists","Tạo và sửa danh sách"),Nxt_text("Your favorite tracks","Những bài bạn yêu thích"),"Spotify · SoundCloud · Drive",Nxt_text("For you · statistics · history · profile","Dành cho bạn · thống kê · hồ sơ"),Nxt_text("Audio · playback · controls · LEDs","Âm thanh · phát nhạc · nút · LED"),Nxt_text("Usage guide · changes · OTA · logs","Hướng dẫn sử dụng · thay đổi · OTA · log")};
 const char *symbols[]={"library","list-music","heart","radio","headphones","settings-2","info"};
 int p=unit(s,32),gap=unit(s,12),w=(s->w-p*2-gap)/2,h=unit(s,90),top=unit(s,149);
 for(int i=0;i<7;i++){
  int x=i==6?p:p+(i%2)*(w+gap),y=top+(i==6?3:i/2)*(h+gap),cw=i==6?s->w-p*2:w;
  if(i==selected)NxtSkin_rect(s,(SDL_Rect){x,y,cw,h},NxtSkin_color(NXT_ACCENT),unit(s,13));else NxtSkin_panel(s,(SDL_Rect){x,y,cw,h},NxtSkin_color(NXT_PANEL),unit(s,13));
  SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK),muted=i==selected?ink:NxtSkin_color(NXT_MUTED);
  NxtSkin_icon(s,symbols[i],x+unit(s,19),y+unit(s,29),unit(s,28),ink);NxtSkin_text(s,25,labels[i],x+unit(s,60),y+unit(s,9),cw-unit(s,72),ink);NxtSkin_text(s,18,subs[i],x+unit(s,60),y+unit(s,49),cw-unit(s,72),muted);
 }
 NxtSkin_mini(s);NxtSkin_footer(s,Nxt_text("D-pad Select · A Open · Y Now playing · START Options","D-pad Chọn · A Mở · Y Đang phát · START Tùy chọn"),false);
}
void NxtSkin_tabs(SDL_Surface *s,const char *const *labels,int count,int selected,int y){int x=unit(s,32);for(int i=0;i<count;i++){int w=0;TTF_SizeUTF8(NxtSkin_font(s,21),labels[i],&w,NULL);w+=unit(s,28);if(i==selected)NxtSkin_rect(s,(SDL_Rect){x,y,w,unit(s,43)},NxtSkin_color(NXT_ACCENT),unit(s,10));NxtSkin_text(s,21,labels[i],x+unit(s,14),y+unit(s,4),w-unit(s,24),NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK));x+=w+unit(s,8);}list_top=y+unit(s,58);}
void NxtSkin_playing(SDL_Surface *s,int track,int total,bool shuffle,int repeat){
    char counter[32];snprintf(counter,32,"%02d / %02d",track,total);
    NxtSkinSource previous=NxtSkin_setSource(playback_source());
    NxtSkin_begin(s,Nxt_text("Now playing","Đang phát"),counter,false);
    NxtSkin_setSource(previous);
    int p=unit(s,32),top=unit(s,147),aw=unit(s,242),x=p+aw+unit(s,32),w=s->w-p-x;
    SDL_Surface *cover=NxtPlayback_art();if(!cover&&!NxtSoundCloud_hasMetadata()&&!NxtSpotify_hasMetadata())cover=album_art_get();
    NxtRecord_draw(s,(SDL_Rect){p,top,aw,aw},cover,NxtPlayback_generation(),NxtPlayback_state()==PLAYER_STATE_PLAYING);
    const TrackInfo *info=NxtPlayback_info();int titlepx=36,tw=0;TTF_SizeUTF8(NxtSkin_font(s,titlepx),info->title,&tw,NULL);
    bool compact=tw>w&&tw<=w*2;bool scroll_title=tw>w*2;if(compact)titlepx=30;else if(scroll_title)titlepx=32;
    int y=top;TTF_Font *titlefont=NxtSkin_font(s,titlepx);
    if(scroll_title){int th=0;TTF_SizeUTF8(titlefont,info->title,NULL,&th);NxtSkin_text(s,titlepx,info->title,x,y,w,NxtSkin_color(NXT_INK));y+=th+unit(s,5);}
    else y+=Nxt_drawWrapped(s,titlefont,info->title,x,y,w,unit(s,110),NxtSkin_color(NXT_INK))+unit(s,5);
    NxtSkin_text(s,compact?25:27,info->artist[0]?info->artist:Nxt_text("Unknown artist","Chưa rõ nghệ sĩ"),x,y,w,NxtSkin_color(NXT_INK));y+=unit(s,compact?33:42);
    NxtSkin_text(s,compact?20:23,info->album,x,y,w,NxtSkin_color(NXT_MUTED));y+=unit(s,compact?31:40);
    char format[256],bits[48]="";if(info->bit_depth)snprintf(bits,48," · %d-bit",info->bit_depth);
    if(NxtSoundCloud_hasMetadata()&&!info->sample_rate)snprintf(format,256,"%s",Nxt_text("SoundCloud · Buffering…","SoundCloud · Đang tải âm thanh…"));
    else if(info->bitrate)snprintf(format,256,"%s%s · %.1f kHz · %d kbps",info->codec,bits,info->sample_rate/1000.f,info->bitrate/1000);else snprintf(format,256,"%s%s · %.1f kHz",info->codec,bits,info->sample_rate/1000.f);
    NxtSkin_text(s,compact?17:18,format,x,y,w,NxtSkin_color(NXT_MUTED));y+=unit(s,compact?28:33);
    NxtSkin_icon(s,Player_isUSBDACActive()?"usb":nxt.output==1?"bluetooth":NxtHardware_hasJack()?"headphones":"volume-2",x,y+unit(s,3),unit(s,23),NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,20,NxtHardware_output(),x+unit(s,34),y,w-unit(s,34),NxtSkin_color(NXT_ACCENT));
    if(Player_isUSBDACActive()){
        y+=unit(s,compact?26:32);
        NxtSkin_text(s,18,NxtHardware_pcm(),x,y,w,NxtSkin_color(NXT_MUTED));
    }
    int lyric_y=top+aw+unit(s,17);if(y+unit(s,compact?29:45)>lyric_y)lyric_y=y+unit(s,compact?29:45);int spectrum_y=s->h-unit(s,266),spectrum_h=unit(s,49);
    if(Settings_getBool(&SETTING_LYRICS_ENABLED)&&lyric_y+unit(s,44)>spectrum_y){int end=spectrum_y+spectrum_h;spectrum_y=lyric_y+unit(s,44);spectrum_h=end-spectrum_y;if(spectrum_h<unit(s,16)){spectrum_h=unit(s,16);spectrum_y=end-spectrum_h;}}
    if(Settings_getBool(&SETTING_LYRICS_ENABLED)&&lyric_y+unit(s,40)<spectrum_y){int current=Lyrics_isSynced()?Lyrics_currentIndex(NxtPlayback_position()):0;if(current<0)current=0;const char *line=Lyrics_lineText(current);if(line)Nxt_drawWrapped(s,NxtSkin_font(s,nxt.lyric_size),line,p,lyric_y,s->w-p*2,spectrum_y-lyric_y-unit(s,7),NxtSkin_color(NXT_ACCENT));else if(NxtSoundCloud_active())NxtSkin_text(s,20,NxtSoundCloud_lyricsStatus(),p,lyric_y,s->w-p*2,NxtSkin_color(NXT_MUTED));else if(NxtSpotify_active())NxtSkin_text(s,20,NxtSpotify_lyricsStatus(),p,lyric_y,s->w-p*2,NxtSkin_color(NXT_MUTED));}
    Spectrum_setPosition(p,spectrum_y,s->w-p*2,spectrum_h);
    int elapsed=NxtPlayback_position()/1000,duration=NxtPlayback_duration()/1000,left=duration>elapsed?duration-elapsed:0;
    NxtSkin_rect(s,(SDL_Rect){p,s->h-unit(s,203),s->w-p*2,unit(s,6)},NxtSkin_color(NXT_PANEL),unit(s,3));
    if(duration>0)NxtSkin_rect(s,(SDL_Rect){p,s->h-unit(s,203),(s->w-p*2)*elapsed/duration,unit(s,6)},NxtSkin_color(NXT_ACCENT),unit(s,3));
    char time[40];snprintf(time,40,"%02d:%02d",elapsed/60,elapsed%60);NxtSkin_text(s,20,time,p,s->h-unit(s,189),unit(s,120),NxtSkin_color(NXT_INK));snprintf(time,40,"−%02d:%02d",left/60,left%60);NxtSkin_text(s,20,time,s->w-p-unit(s,86),s->h-unit(s,189),unit(s,90),NxtSkin_color(NXT_MUTED));
    int mode_y=s->h-unit(s,136);NxtSkin_icon(s,"shuffle",p,mode_y,unit(s,25),NxtSkin_color(shuffle?NXT_ACCENT:NXT_MUTED));NxtSkin_icon(s,repeat==1?"repeat-1":"repeat",p+unit(s,53),mode_y,unit(s,25),NxtSkin_color(repeat?NXT_ACCENT:NXT_MUTED));NxtSkin_icon(s,"heart",p+unit(s,106),mode_y,unit(s,25),NxtSkin_color(NxtPlayback_favorite()?NXT_ACCENT:NXT_MUTED));
    int remaining=Nxt_timerRemaining();NxtSkin_icon(s,"timer",s->w-p-unit(s,140),mode_y,unit(s,25),NxtSkin_color(remaining?NXT_ACCENT:NXT_MUTED));if(remaining)snprintf(time,40,"%02d:%02d",remaining/60,remaining%60);else snprintf(time,40,"--:--");NxtSkin_text(s,19,time,s->w-p-unit(s,99),mode_y-unit(s,4),unit(s,100),NxtSkin_color(NXT_MUTED));
    if(Nxt_isLocked())NxtSkin_icon(s,"lock-keyhole",s->w/2,mode_y,unit(s,25),NxtSkin_color(NXT_ACCENT));
    NxtSkin_footer(s,NULL,true);
}
