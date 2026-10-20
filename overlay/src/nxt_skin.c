#include "nxt_lyric_preview.h"
#include "nxt_volume.h"
#include "nxt_motion.h"
#include "nxt_auralis.h"
#include "nxt_appearance.h"
#include "nxt_translate.h"
#include "nxt_theme.h"
#include "nxt_custom.h"
#include "nxt_monitor.h"
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
#include "msettings.h"
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
static bool scrolling_text;
static int unit(SDL_Surface *s,int n){return (n*s->w+512)/1024;}
SDL_Color NxtSkin_color(NxtColor n){
 SDL_Color full;if(NxtTheme_color(n,&full))return full;
 SDL_Color system=uintToColour(CFG_getColor(COLOR_BACKGROUND));bool light=nxt.theme==2||(nxt.theme==0&&(system.r*299+system.g*587+system.b*114)>160000);
 static const SDL_Color dark[]={{16,43,49,255},{28,65,72,255},{220,233,231,255},{170,196,189,255},{58,82,83,255}};
 static const SDL_Color pale[]={{228,223,209,255},{209,203,189,255},{50,61,55,255},{95,106,96,255},{178,175,158,255}};
 const SDL_Color *palette=light?pale:dark;if(n<=NXT_LINE)return palette[n];
 static unsigned saved_rgb;static int saved_light=-1;static SDL_Color saved_accent,saved_ink;
 unsigned rgb=NxtAppearance_accent();if(saved_light==(int)light&&saved_rgb==rgb)return n==NXT_ACCENT?saved_accent:saved_ink;
 SDL_Color raw={(rgb>>16)&255,(rgb>>8)&255,rgb&255,255},target=light?(SDL_Color){0,0,0,255}:(SDL_Color){255,255,255,255},accent=raw;
 for(int percent=0;percent<=100;percent++){accent=Theme_mix(raw,target,percent);if(Theme_contrast(accent,palette[NXT_BG])>=4.5&&Theme_contrast(accent,palette[NXT_PANEL])>=4.5)break;}
 SDL_Color black={0,0,0,255},white={255,255,255,255};saved_light=light;saved_rgb=rgb;saved_accent=accent;saved_ink=Theme_contrast(accent,black)>=Theme_contrast(accent,white)?black:white;return n==NXT_ACCENT?saved_accent:saved_ink;
}

/* Static pages poll status at 1 Hz; only clipped, scrolling text needs 10 Hz.
 * Input is never throttled. Sleeping/hidden sessions still run audio and OTA. */
bool NxtSkin_shouldDraw(uint32_t last,bool dirty){
    if(PWR_getDisplayOff()||NxtSession_isHidden())return false;
    return dirty||!last||(uint32_t)(SDL_GetTicks()-last)>=(scrolling_text?100:1000);
}

#define TEXT_CACHE_COUNT 96
typedef struct {SDL_Surface *source,*scaled;unsigned generation;uint64_t used;} ArtCache;
static ArtCache arts[3];static uint64_t art_clock;
static void forget_art(void){for(int i=0;i<3;i++){if(arts[i].scaled)SDL_FreeSurface(arts[i].scaled);memset(&arts[i],0,sizeof(arts[i]));}art_clock=0;}
void NxtSkin_forgetArt(void){forget_art();}
void NxtSkin_cachedArt(SDL_Surface *screen,SDL_Surface *source,SDL_Rect dst){
 SDL_BlendMode original;Uint8 alpha;Uint32 key;
 if(!screen||!source)return;
 SDL_GetSurfaceBlendMode(source,&original);SDL_GetSurfaceAlphaMod(source,&alpha);
 /* Colour-key sprites retain SDL semantics. Alpha covers are filtered and
  * cached too; their blend/colour/opacity settings remain live at draw time. */
 if(SDL_GetColorKey(source,&key)==0){SDL_BlitScaled(source,NULL,screen,&dst);return;}
 if(!screen||!source||dst.w<1||dst.h<1)return;unsigned gen=NxtPlayback_generation();ArtCache *hit=NULL,*old=&arts[0];
 for(int i=0;i<3;i++){ArtCache *p=&arts[i];if(p->source==source&&p->scaled&&p->generation==gen&&p->scaled->w==dst.w&&p->scaled->h==dst.h){hit=p;break;}if(!p->scaled||p->used<old->used)old=p;}
 if(!hit){if(old->scaled)SDL_FreeSurface(old->scaled);memset(old,0,sizeof(*old));old->scaled=SDL_CreateRGBSurfaceWithFormat(0,dst.w,dst.h,32,SDL_PIXELFORMAT_ARGB8888);
  if(!old->scaled){SDL_BlitScaled(source,NULL,screen,&dst);return;}
  SDL_BlendMode blend;SDL_GetSurfaceBlendMode(source,&blend);SDL_SetSurfaceBlendMode(source,SDL_BLENDMODE_NONE);if(!NxtSkin_smoothScale(source,NULL,old->scaled,NULL))SDL_BlitScaled(source,NULL,old->scaled,NULL);SDL_SetSurfaceBlendMode(source,blend);SDL_SetSurfaceBlendMode(old->scaled,blend);old->source=source;old->generation=gen;hit=old;
 }
 hit->used=++art_clock;Uint8 r,g,b;SDL_GetSurfaceColorMod(source,&r,&g,&b);SDL_SetSurfaceColorMod(hit->scaled,r,g,b);SDL_SetSurfaceAlphaMod(hit->scaled,alpha);SDL_SetSurfaceBlendMode(hit->scaled,original);SDL_BlitSurface(hit->scaled,NULL,screen,&dst);
}
#define TEXT_CACHE_BYTES (6u*1024u*1024u)
typedef struct {TTF_Font *font;char *text;SDL_Surface *image,*fitted,*stroke;int stroke_w,stroke_h,stroke_radius;SDL_Surface *stroke_source;int fitted_w,fitted_h;size_t bytes;uint64_t used;SDL_Rect ink;bool measured;uint32_t started;unsigned words_revision;int words_line,word_edges[LYRICS_MAX_WORDS+1];} TextCache;
static TextCache glyphs[TEXT_CACHE_COUNT];static size_t glyph_bytes;static uint64_t glyph_clock;
static void drop_glyph(TextCache *p){if(p->image)SDL_FreeSurface(p->image);if(p->fitted)SDL_FreeSurface(p->fitted);if(p->stroke)SDL_FreeSurface(p->stroke);free(p->text);glyph_bytes-=p->bytes;memset(p,0,sizeof(*p));}
void NxtSkin_forgetFont(TTF_Font *font){for(int i=0;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].image&&(!font||glyphs[i].font==font))drop_glyph(&glyphs[i]);}
SDL_Surface *NxtSkin_renderText(TTF_Font *font,const char *text){
    if(!font||!text)return NULL;TextCache *slot=NULL;
    for(int i=0;i<TEXT_CACHE_COUNT;i++){TextCache *p=&glyphs[i];if(p->image&&p->font==font&&!strcmp(p->text,text)){p->used=++glyph_clock;return p->image;}if(!p->image)slot=p;}
    if(!slot){slot=&glyphs[0];for(int i=1;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].used<slot->used)slot=&glyphs[i];drop_glyph(slot);}
    SDL_Surface *image=NxtText_render(font,text,(SDL_Color){255,255,255,255});if(!image)return NULL;size_t bytes=(size_t)image->pitch*image->h;
    if(bytes>TEXT_CACHE_BYTES){SDL_FreeSurface(image);return NULL;}
    while(glyph_bytes+bytes>TEXT_CACHE_BYTES){TextCache *oldest=NULL;for(int i=0;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].image&&(!oldest||glyphs[i].used<oldest->used))oldest=&glyphs[i];if(!oldest)break;drop_glyph(oldest);}
    char *copy=strdup(text);if(!copy){SDL_FreeSurface(image);return NULL;}*slot=(TextCache){.font=font,.text=copy,.image=image,.bytes=bytes,.used=++glyph_clock,.started=SDL_GetTicks()};glyph_bytes+=bytes;return image;
}
static uint32_t text_age(SDL_Surface *s){for(int i=0;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].image==s)return SDL_GetTicks()-glyphs[i].started;return 0;}
static bool text_ink(SDL_Surface *image,SDL_Rect *ink){
    TextCache *entry=NULL;for(int i=0;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].image==image){entry=&glyphs[i];break;}
    if(!entry)return false;if(entry->measured){*ink=entry->ink;return true;}
    if(SDL_LockSurface(image)<0)return false;int left=image->w,top=image->h,right=-1,bottom=-1;
    for(int y=0;y<image->h;y++){const Uint32 *row=(const Uint32 *)((const Uint8 *)image->pixels+y*image->pitch);
        for(int x=0;x<image->w;x++)if(row[x]&image->format->Amask){if(x<left)left=x;if(x>right)right=x;if(y<top)top=y;if(y>bottom)bottom=y;}}
    SDL_UnlockSurface(image);entry->ink=(SDL_Rect){left,top,right-left+1,bottom-top+1};entry->measured=true;*ink=entry->ink;return true;
}
/* Rounded masks are generated once, with native-pixel coverage at the edge.
 * Cache both opaque and glass panels, under a fixed 4 MiB budget. */
#define PANEL_COUNT 32
#define PANEL_BYTES (4u*1024u*1024u)
typedef struct {SDL_Surface *image;SDL_Color color;int w,h,radius;unsigned used;} PanelCache;
static PanelCache panels[PANEL_COUNT];static unsigned panel_clock;static size_t panel_bytes;
static void drop_panel(PanelCache *p){if(p->image){panel_bytes-=(size_t)p->image->pitch*p->image->h;SDL_FreeSurface(p->image);}memset(p,0,sizeof(*p));}
void NxtSkin_rect(SDL_Surface *s,SDL_Rect r,SDL_Color color,int radius){
 if(!s||r.w<=0||r.h<=0||!color.a)return;
 if(radius>r.w/2)radius=r.w/2;if(radius>r.h/2)radius=r.h/2;if(radius<0)radius=0;
 if(!radius&&color.a==255){SDL_FillRect(s,&r,SDL_MapRGB(s->format,color.r,color.g,color.b));return;}
 PanelCache *hit=NULL,*old=&panels[0];
 for(int i=0;i<PANEL_COUNT;i++){PanelCache *p=&panels[i];if(p->image&&p->w==r.w&&p->h==r.h&&p->radius==radius&&!memcmp(&p->color,&color,sizeof(color))){hit=p;break;}if(!p->image||p->used<old->used)old=p;}
 if(!hit){size_t bytes=(size_t)r.w*r.h*4;if(bytes>PANEL_BYTES)return;drop_panel(old);
  while(panel_bytes+bytes>PANEL_BYTES){PanelCache *victim=NULL;for(int i=0;i<PANEL_COUNT;i++)if(panels[i].image&&(!victim||panels[i].used<victim->used))victim=&panels[i];if(!victim)break;drop_panel(victim);}
  SDL_Surface *mask=SDL_CreateRGBSurfaceWithFormat(0,r.w,r.h,32,SDL_PIXELFORMAT_ARGB8888);if(!mask)return;
  SDL_FillRect(mask,NULL,SDL_MapRGBA(mask->format,color.r,color.g,color.b,color.a));
  SDL_LockSurface(mask);
  for(int y=0;y<radius;y++)for(int x=0;x<radius;x++){
   float dx=radius-x-.5f,dy=radius-y-.5f,coverage=radius+.5f-sqrtf(dx*dx+dy*dy);if(coverage>1)coverage=1;if(coverage<0)coverage=0;
   Uint32 pixel=SDL_MapRGBA(mask->format,color.r,color.g,color.b,(Uint8)lroundf(color.a*coverage));
   Uint32 *top=(Uint32*)((Uint8*)mask->pixels+y*mask->pitch),*bottom=(Uint32*)((Uint8*)mask->pixels+(r.h-1-y)*mask->pitch);
   top[x]=top[r.w-1-x]=bottom[x]=bottom[r.w-1-x]=pixel;
  }
  SDL_UnlockSurface(mask);SDL_SetSurfaceBlendMode(mask,SDL_BLENDMODE_BLEND);
  *old=(PanelCache){.image=mask,.color=color,.w=r.w,.h=r.h,.radius=radius};panel_bytes+=(size_t)mask->pitch*mask->h;hit=old;
 }
 hit->used=++panel_clock;
 if(color.a==255&&radius){
  /* Opaque panels keep fast fills for their large interior; blend only the
   * four antialiased corners, not the entire panel on every repaint. */
  SDL_FillRect(s,&(SDL_Rect){r.x+radius,r.y,r.w-2*radius,r.h},SDL_MapRGB(s->format,color.r,color.g,color.b));
  SDL_FillRect(s,&(SDL_Rect){r.x,r.y+radius,radius,r.h-2*radius},SDL_MapRGB(s->format,color.r,color.g,color.b));
  SDL_FillRect(s,&(SDL_Rect){r.x+r.w-radius,r.y+radius,radius,r.h-2*radius},SDL_MapRGB(s->format,color.r,color.g,color.b));
  for(int y=0;y<2;y++)for(int x=0;x<2;x++){int xx=x?(r.w-radius):0,yy=y?(r.h-radius):0;SDL_BlitSurface(hit->image,&(SDL_Rect){xx,yy,radius,radius},s,&(SDL_Rect){r.x+xx,r.y+yy,0,0});}
 }else SDL_BlitSurface(hit->image,NULL,s,&r);
}
/* Coverage only along a narrow band around the segment: O(length * width),
 * including diagonal animated needles, rather than scanning a full rectangle. */
static void covered_pixel(SDL_Surface *s,int x,int y,SDL_Color c,float coverage){
 if(x<s->clip_rect.x||y<s->clip_rect.y||x>=s->clip_rect.x+s->clip_rect.w||y>=s->clip_rect.y+s->clip_rect.h)return;
 if(coverage<=0)return;if(coverage>1)coverage=1;unsigned a=(unsigned)lroundf(c.a*coverage);
 Uint32 *p=(Uint32*)((Uint8*)s->pixels+y*s->pitch)+x;Uint8 r,g,b,old;SDL_GetRGBA(*p,s->format,&r,&g,&b,&old);
 *p=SDL_MapRGBA(s->format,(c.r*a+r*(255-a)+127)/255,(c.g*a+g*(255-a)+127)/255,(c.b*a+b*(255-a)+127)/255,a+(old*(255-a)+127)/255);
}
void NxtSkin_line(SDL_Surface *s,int x,int y,int ex,int ey,SDL_Color c,int thick){
 if(!s||s->format->BytesPerPixel!=4)return;if(thick<1)thick=1;
 float dx=ex-x,dy=ey-y,length=dx*dx+dy*dy,radius=thick*.5f;
 bool horizontal=fabsf(dx)>=fabsf(dy);float major=horizontal?dx:dy,minor=horizontal?dy:dx;
 int start=(int)floorf(fminf(horizontal?x:y,horizontal?ex:ey)-radius-1),end=(int)ceilf(fmaxf(horizontal?x:y,horizontal?ex:ey)+radius+1);
 int limit=horizontal?s->w:s->h;if(start<0)start=0;if(end>=limit)end=limit-1;
 if(SDL_LockSurface(s)<0)return;
 for(int i=start;i<=end;i++){float t=major?((float)i-(horizontal?x:y))/major:0;if(t<0)t=0;if(t>1)t=1;
  float center=(horizontal?y:x)+t*minor,span=radius*(major?sqrtf(1+(minor/major)*(minor/major)):1)+1;
  for(int j=(int)floorf(center-span);j<=(int)ceilf(center+span);j++){int px=horizontal?i:j,py=horizontal?j:i;float q=length?((px-x)*dx+(py-y)*dy)/length:0;if(q<0)q=0;if(q>1)q=1;float xx=px-x-q*dx,yy=py-y-q*dy;covered_pixel(s,px,py,c,radius+.5f-sqrtf(xx*xx+yy*yy));}
 }
 SDL_UnlockSurface(s);
}
void NxtSkin_panel(SDL_Surface *s,SDL_Rect r,SDL_Color color,int radius){
    if(nxt.wallpaper_mode&&!nxt.interface_style)color.a=nxt.panel_opacity*255/100;
    NxtSkin_rect(s,r,color,radius);
}
/* Render white glyphs once, tint the outline and then the actual text. This
 * protects text over photos without placing an opaque card over the photo. */
void NxtSkin_blitText(SDL_Surface *s,SDL_Surface *text,SDL_Rect src,int x,int y,SDL_Color color){
    if(nxt.wallpaper_mode&&!nxt.interface_style){int d=unit(s,1);if(d<1)d=1;int light=color.r*299+color.g*587+color.b*114>140000;Uint8 shadow=light?0:255;
        SDL_SetSurfaceColorMod(text,shadow,shadow,shadow);SDL_SetSurfaceAlphaMod(text,210);
        SDL_BlitSurface(text,&src,s,&(SDL_Rect){x-d,y,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x+d,y,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y-d,0,0});SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y+d,0,0});
    }
    SDL_SetSurfaceColorMod(text,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(text,color.a);SDL_BlitSurface(text,&src,s,&(SDL_Rect){x,y,0,0});
}
/* Each size has its own entry. A shared icon used in a header and controls
 * must not be rescaled twice on every frame. Only scaled glyphs stay resident. */
typedef struct {char name[48];SDL_Surface *image;int edge;unsigned used;} Icon;
#define ICON_COUNT 128
#define ICON_BYTES (2u*1024u*1024u)
static Icon icons[ICON_COUNT];static unsigned icon_clock;static size_t icon_bytes;
static void drop_icon(Icon *p){if(p->image){icon_bytes-=(size_t)p->image->pitch*p->image->h;SDL_FreeSurface(p->image);}memset(p,0,sizeof(*p));}
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
    NxtSkin_cachedArt(s,image,(SDL_Rect){x,y,w,h});
}
void NxtSkin_icon(SDL_Surface *s,const char *name,int x,int y,int size,SDL_Color color){
 if(!s||!name||size<1||size>512)return;if(NxtCustom_draw(s,name,x,y,size,color))return;
 Icon *hit=NULL,*old=&icons[0];
 for(int i=0;i<ICON_COUNT;i++){Icon *p=&icons[i];if(p->image&&p->edge==size&&!strcmp(p->name,name)){hit=p;break;}if(!p->image||p->used<old->used)old=p;}
 if(!hit){char path[160];snprintf(path,sizeof(path),"./res/icons/%s.png",name);SDL_Surface *raw=IMG_Load(path);if(!raw)return;
  size_t bytes=(size_t)size*size*4;drop_icon(old);
  while(icon_bytes+bytes>ICON_BYTES){Icon *victim=NULL;for(int i=0;i<ICON_COUNT;i++)if(icons[i].image&&(!victim||icons[i].used<victim->used))victim=&icons[i];if(!victim)break;drop_icon(victim);}
  SDL_Surface *scaled=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_ARGB8888);
  if(!scaled||!NxtSkin_smoothScale(raw,NULL,scaled,NULL)){SDL_FreeSurface(scaled);SDL_FreeSurface(raw);return;}SDL_FreeSurface(raw);
  SDL_SetSurfaceBlendMode(scaled,SDL_BLENDMODE_BLEND);old->image=scaled;old->edge=size;snprintf(old->name,sizeof(old->name),"%s",name);icon_bytes+=(size_t)scaled->pitch*size;hit=old;
 }
 hit->used=++icon_clock;SDL_SetSurfaceColorMod(hit->image,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(hit->image,color.a);SDL_BlitSurface(hit->image,NULL,s,&(SDL_Rect){x,y,0,0});
}

static TTF_Font *fonts[32];static int sizes[32],nf,signature;static char font_source[512];
/* One lazy italic face for translated rows; source fonts remain upright.
 * It has its own font/cache key: never mutate the shared regular face. */
static TTF_Font *translation_italic;static int italic_signature,italic_pixels,italic_style;static char italic_source[512];
static int skin_face(int style){(void)style;return nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font?17:nxt.font_index;}
static void forget_italic(void){if(translation_italic){NxtSkin_forgetFont(translation_italic);TTF_CloseFont(translation_italic);translation_italic=NULL;}}

TTF_Font *NxtSkin_font(SDL_Surface *s,int pixels){
    int sig=s->w*10000+skin_face(nxt.interface_style)*200+nxt.font_size+nxt.interface_style*1000000;
    if(sig!=signature||strcmp(font_source,nxt.font_path)){snprintf(font_source,sizeof(font_source),"%s",nxt.font_path);for(int i=0;i<nf;i++){NxtSkin_forgetFont(fonts[i]);TTF_CloseFont(fonts[i]);}nf=0;signature=sig;}
    pixels=unit(s,pixels);if(pixels<12)pixels=12;
    for(int i=0;i<nf;i++)if(sizes[i]==pixels)return fonts[i];
    if(nf>=32)return fonts[0];int style=nxt.interface_style;int face=skin_face(style);TTF_Font *f=(style||NxtLyricPreview_active())?NxtText_open(nxt.font_path[0]&&!(style==NXT_THEME_AURALIS&&nxt.auralis_reference_font)?nxt.font_path:NxtText_fontPath(face),pixels):Fonts_open(pixels);if(f&&!style&&NxtLyricPreview_active())TTF_SetFontStyle(f,CFG_getFontStyle());if(f){sizes[nf]=pixels;fonts[nf++]=f;}return f;
}
void NxtSkin_text(SDL_Surface *s,int pixels,const char *text,int x,int y,int width,SDL_Color color){
    if(!text||width<=0)return;SDL_Surface *rendered=NxtSkin_renderText(NxtSkin_font(s,pixels),text);if(!rendered)return;
    int offset=0;if(rendered->w>width){scrolling_text=true;int distance=rendered->w-width,speed=unit(s,35);if(speed<1)speed=1;int duration=distance*1000/speed,phase=text_age(rendered)%(duration+4000);if(phase>1800)offset=(phase-1800)*speed/1000;if(offset>distance)offset=distance;}
    NxtSkin_blitText(s,rendered,(SDL_Rect){offset,0,width<rendered->w?width:rendered->w,rendered->h},x,y,color);
}
/* Tight device readouts use cached visible glyph bounds, so font padding
 * cannot overlap the next row at 640x480. Keep long output names scrolling. */
void NxtSkin_compactText(SDL_Surface *s,int pixels,const char *value,int x,int y,int width,int height,SDL_Color color){
    if(!value||width<=0||height<=0)return;SDL_Surface *rendered=NULL;SDL_Rect ink={0};
    for(;;){rendered=NxtSkin_renderText(NxtSkin_font(s,pixels),value);if(!rendered||!text_ink(rendered,&ink)||ink.w<=0||ink.h<=0)return;if(ink.h<=height||pixels<=12)break;pixels--;}
    if(ink.w>width){scrolling_text=true;int distance=ink.w-width,speed=unit(s,35);if(speed<1)speed=1;int duration=distance*1000/speed,phase=text_age(rendered)%(duration+4000),offset=phase>1800?(phase-1800)*speed/1000:0;if(offset>distance)offset=distance;ink.x+=offset;ink.w=width;}
    if(ink.h>height){SDL_SetSurfaceColorMod(rendered,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(rendered,color.a);SDL_Rect fitted={x,y,ink.w*height/ink.h,height};SDL_BlitScaled(rendered,&ink,s,&fitted);}else NxtSkin_blitText(s,rendered,ink,x,y+(height-ink.h)/2,color);
}
/* Smooth cached resize preserves anti-aliased glyph edges on long sentences. */
/* Area sampling at cache creation removes downscale aliasing. Alpha is
 * accumulated premultiplied, then unpremultiplied, preserving clean glyph edges.
 * Upscaling uses SDL's bilinear path. No whole-screen supersampling per frame. */
bool NxtSkin_smoothScale(SDL_Surface *source,const SDL_Rect *crop,SDL_Surface *dst,const SDL_Rect *box){
 if(!source||!dst)return false;
 SDL_Rect sr=crop?*crop:(SDL_Rect){0,0,source->w,source->h},dr=box?*box:(SDL_Rect){0,0,dst->w,dst->h};
 if(sr.w<1||sr.h<1||dr.w<1||dr.h<1||sr.x<0||sr.y<0||sr.x+sr.w>source->w||sr.y+sr.h>source->h||dr.x<0||dr.y<0||dr.x+dr.w>dst->w||dr.y+dr.h>dst->h)return false;
 SDL_Surface *rgba=source;if(source->format->format!=dst->format->format)rgba=SDL_ConvertSurface(source,dst->format,0);if(!rgba)return false;
 bool ok=false;
 if(dst->format->format==SDL_PIXELFORMAT_ARGB8888&&dr.w<=sr.w&&dr.h<=sr.h&&(dr.w<sr.w||dr.h<sr.h)){
  bool locked=SDL_LockSurface(rgba)==0;if(locked&&SDL_LockSurface(dst)==0){
   const double sx=(double)sr.w/dr.w,sy=(double)sr.h/dr.h;
   for(int y=0;y<dr.h;y++){double top=y*sy,bottom=(y+1)*sy;Uint32 *out=(Uint32*)((Uint8*)dst->pixels+(dr.y+y)*dst->pitch)+dr.x;
    for(int x=0;x<dr.w;x++){double left=x*sx,right=(x+1)*sx,a=0,r=0,g=0,b=0;
     for(int iy=(int)top;iy<(int)ceil(bottom)&&iy<sr.h;iy++){double wy=fmin(bottom,iy+1.)-fmax(top,iy);const Uint32 *row=(const Uint32*)((const Uint8*)rgba->pixels+(sr.y+iy)*rgba->pitch)+sr.x;
      for(int ix=(int)left;ix<(int)ceil(right)&&ix<sr.w;ix++){double weight=wy*(fmin(right,ix+1.)-fmax(left,ix));Uint32 p=row[ix];double alpha=(p>>24)*weight;a+=alpha;r+=((p>>16)&255)*alpha;g+=((p>>8)&255)*alpha;b+=(p&255)*alpha;}}
     Uint32 aa=(Uint32)lround(a/(sx*sy));out[x]=a>0?(aa<<24)|((Uint32)lround(r/a)<<16)|((Uint32)lround(g/a)<<8)|(Uint32)lround(b/a):0;
    }
   }
   SDL_UnlockSurface(dst);ok=true;
  }if(locked)SDL_UnlockSurface(rgba);
 }else ok=SDL_SoftStretchLinear(rgba,&sr,dst,&dr)==0;
 if(rgba!=source)SDL_FreeSurface(rgba);return ok;
}
static TextCache *glyph_entry(SDL_Surface *image){for(int i=0;i<TEXT_CACHE_COUNT;i++)if(glyphs[i].image==image)return &glyphs[i];return NULL;}
static SDL_Surface *fitted_text(SDL_Surface *image,SDL_Rect ink,int w,int h){
 TextCache *entry=glyph_entry(image);if(!entry)return NULL;
 if(entry->fitted&&(entry->fitted_w!=w||entry->fitted_h!=h)){size_t bytes=(size_t)entry->fitted->pitch*entry->fitted->h;if(entry->stroke){size_t b=(size_t)entry->stroke->pitch*entry->stroke->h;SDL_FreeSurface(entry->stroke);entry->stroke=NULL;entry->bytes-=b;glyph_bytes-=b;}SDL_FreeSurface(entry->fitted);entry->fitted=NULL;entry->bytes-=bytes;glyph_bytes-=bytes;}
 if(!entry->fitted){
  size_t bytes=(size_t)w*h*4;if(bytes>TEXT_CACHE_BYTES/2)return NULL;
  while(glyph_bytes+bytes>TEXT_CACHE_BYTES){TextCache *oldest=NULL;for(int i=0;i<TEXT_CACHE_COUNT;i++)if(&glyphs[i]!=entry&&glyphs[i].image&&(!oldest||glyphs[i].used<oldest->used))oldest=&glyphs[i];if(!oldest)return NULL;drop_glyph(oldest);}
  SDL_Surface *scaled=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);if(!scaled)return NULL;
  SDL_SetSurfaceColorMod(image,255,255,255);SDL_SetSurfaceAlphaMod(image,255);
  if(!NxtSkin_smoothScale(image,&ink,scaled,NULL)){SDL_FreeSurface(scaled);return NULL;}
  SDL_SetSurfaceBlendMode(scaled,SDL_BLENDMODE_BLEND);entry->fitted=scaled;entry->fitted_w=w;entry->fitted_h=h;bytes=(size_t)scaled->pitch*h;entry->bytes+=bytes;glyph_bytes+=bytes;
 }
 return entry->fitted;
}
/* Stroke masks share the 6 MiB LRU budget with glyphs. Dilation is cached
 * once per fitted size, not repeated during 60 Hz lyric motion. */
static bool decorate;
static void clear_stroke(TextCache*e){if(e->stroke){size_t b=(size_t)e->stroke->pitch*e->stroke->h;SDL_FreeSurface(e->stroke);e->stroke=NULL;e->bytes-=b;glyph_bytes-=b;}}
static void stroke_text(SDL_Surface*s,SDL_Surface*original,SDL_Surface*image,SDL_Rect crop,int x,int y,SDL_Color ink){
 TextCache*e=glyph_entry(original);if(!e)return;int radius=nxt.lyric_outline?unit(s,nxt.lyric_outline):0;if(nxt.lyric_outline&&radius<1)radius=1;if(radius>3)radius=3;int r=radius?radius:1;
 if(e->stroke&&(e->stroke_w!=crop.w||e->stroke_h!=crop.h||e->stroke_radius!=r||e->stroke_source!=image))clear_stroke(e);
 if(!e->stroke){size_t bytes=(size_t)(crop.w+2*r)*(crop.h+2*r)*4;if(bytes>TEXT_CACHE_BYTES/2)return;
  while(glyph_bytes+bytes>TEXT_CACHE_BYTES){TextCache*old=NULL;for(int i=0;i<TEXT_CACHE_COUNT;i++)if(&glyphs[i]!=e&&glyphs[i].image&&(!old||glyphs[i].used<old->used))old=&glyphs[i];if(!old)return;drop_glyph(old);}
  e->stroke=SDL_CreateRGBSurfaceWithFormat(0,crop.w+2*r,crop.h+2*r,32,SDL_PIXELFORMAT_ARGB8888);if(!e->stroke)return;
  SDL_SetSurfaceColorMod(image,255,255,255);SDL_SetSurfaceAlphaMod(image,255);SDL_SetSurfaceBlendMode(image,SDL_BLENDMODE_BLEND);
  for(int dy=-r;dy<=r;dy++)for(int dx=-r;dx<=r;dx++)if(dx*dx+dy*dy<=r*r+1){SDL_Rect dest={dx+r,dy+r,0,0};SDL_BlitSurface(image,&crop,e->stroke,&dest);}
  SDL_SetSurfaceBlendMode(e->stroke,SDL_BLENDMODE_BLEND);e->stroke_w=crop.w;e->stroke_h=crop.h;e->stroke_radius=r;e->stroke_source=image;bytes=(size_t)e->stroke->pitch*e->stroke->h;e->bytes+=bytes;glyph_bytes+=bytes;
 }
 if(nxt.lyric_shadow){SDL_SetSurfaceColorMod(e->stroke,0,0,0);SDL_SetSurfaceAlphaMod(e->stroke,ink.a*3/4);SDL_Rect dst={x-r+unit(s,2),y-r+unit(s,3),0,0};SDL_BlitSurface(e->stroke,NULL,s,&dst);}
 if(radius){SDL_SetSurfaceColorMod(e->stroke,(nxt.lyric_outline_rgb>>16)&255,(nxt.lyric_outline_rgb>>8)&255,nxt.lyric_outline_rgb&255);SDL_SetSurfaceAlphaMod(e->stroke,ink.a);SDL_Rect dst={x-r,y-r,0,0};SDL_BlitSurface(e->stroke,NULL,s,&dst);}
}
/* Independent faces keep lyric bold/italic out of menu and title font state. */
static struct {TTF_Font*font;int size,style,signature;char path[512];} lyric_faces[4];static unsigned lyric_face_next;
static void forget_lyric_faces(void){for(int i=0;i<4;i++)if(lyric_faces[i].font){NxtSkin_forgetFont(lyric_faces[i].font);NxtText_close(lyric_faces[i].font);lyric_faces[i].font=NULL;}}
static TTF_Font *lyric_font(SDL_Surface*s,int pixels,bool italic,bool bold){
 TTF_Font*normal=NxtSkin_font(s,pixels);if(!normal||(!italic&&!bold))return normal;int size=unit(s,pixels);if(size<12)size=12;int style=(italic?TTF_STYLE_ITALIC:0)|(bold?TTF_STYLE_BOLD:0);
 int face=skin_face(nxt.interface_style);const char*path=nxt.font_path[0]&&!(nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font)?nxt.font_path:NxtText_fontPath(face);
 const char*ital=italic&&!nxt.font_path[0]?NxtText_italicPath(face):NULL;if(ital)path=ital;
 for(int i=0;i<4;i++)if(lyric_faces[i].font&&lyric_faces[i].size==size&&lyric_faces[i].style==style&&lyric_faces[i].signature==signature&&!strcmp(lyric_faces[i].path,path))return lyric_faces[i].font;
 unsigned i=lyric_face_next++%4;if(lyric_faces[i].font){NxtSkin_forgetFont(lyric_faces[i].font);NxtText_close(lyric_faces[i].font);}lyric_faces[i].font=NxtText_open(path,size);lyric_faces[i].size=size;lyric_faces[i].style=style;lyric_faces[i].signature=signature;snprintf(lyric_faces[i].path,sizeof(lyric_faces[i].path),"%s",path);if(lyric_faces[i].font)TTF_SetFontStyle(lyric_faces[i].font,style);return lyric_faces[i].font?lyric_faces[i].font:normal;
}
static void fit_fontText(SDL_Surface *s,TTF_Font *font,const char *value,int x,int y,int width,int height,SDL_Color color,bool center){
 if(!value||width<=0||height<=0)return;SDL_Surface *rendered=NxtSkin_renderText(font,value);SDL_Rect ink;
 if(!rendered||!text_ink(rendered,&ink)||ink.w<=0||ink.h<=0)return;int w=ink.w,h=ink.h;
 if(w>width){h=(int)((long long)h*width/w);w=width;}if(h>height){w=(int)((long long)w*height/h);h=height;}if(w<1)w=1;if(h<1)h=1;
 int xx=x+(center?(width-w)/2:0),yy=y+(height-h)/2;SDL_Rect previous_clip;SDL_GetClipRect(s,&previous_clip);
 if(decorate){SDL_Rect box={x-unit(s,5),y,width+unit(s,10),height},intersection;if(!SDL_IntersectRect(&previous_clip,&box,&intersection))return;SDL_SetClipRect(s,&intersection);}
 if(w==ink.w&&h==ink.h){if(decorate&&(nxt.lyric_outline||nxt.lyric_shadow))stroke_text(s,rendered,rendered,ink,xx,yy,color);NxtSkin_blitText(s,rendered,ink,xx,yy,color);}
 else {SDL_Surface *scaled=fitted_text(rendered,ink,w,h);if(scaled){if(decorate&&(nxt.lyric_outline||nxt.lyric_shadow))stroke_text(s,rendered,scaled,(SDL_Rect){0,0,w,h},xx,yy,color);NxtSkin_blitText(s,scaled,(SDL_Rect){0,0,w,h},xx,yy,color);}else {SDL_SetSurfaceColorMod(rendered,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(rendered,color.a);SDL_BlitScaled(rendered,&ink,s,&(SDL_Rect){xx,yy,w,h});}}
 if(decorate)SDL_SetClipRect(s,&previous_clip);
}
SDL_Color NxtSkin_lyricColor(int role,Uint8 alpha){unsigned c=role==1?nxt.lyric_translation_rgb:role==2?nxt.song_title_rgb:role==3?nxt.song_artist_rgb:role==4?nxt.lyric_unsung_rgb:nxt.lyric_rgb;return(SDL_Color){c>>16,(c>>8)&255,c&255,alpha};}
void NxtSkin_styledText(SDL_Surface*s,int pixels,const char*value,int x,int y,int width,int height,int role,Uint8 alpha,bool center){
 TTF_Font*f=lyric_font(s,pixels,role==1,(role<2||role==4)&&nxt.lyric_bold);decorate=true;fit_fontText(s,f,value,x,y,width,height,NxtSkin_lyricColor(role,alpha),center);decorate=false;
}
/* Long metadata keeps a readable two-line title instead of a tiny marquee.
 * The split is cached by face/size/width/text; no measuring on warm frames. */
void NxtSkin_songTitle(SDL_Surface*s,int pixels,const char*value,int x,int y,int width,int height,bool center){
 static char saved[2048],first[2048],second[2048],path[512];static int sw,sp,sig;static bool two;TTF_Font*f=NxtSkin_font(s,pixels);if(!value||!f)return;
 if(sw!=width||sp!=pixels||sig!=signature||strcmp(path,font_source)||strcmp(saved,value)){
  snprintf(saved,sizeof(saved),"%s",value);snprintf(first,sizeof(first),"%s",saved);second[0]=0;sw=width;sp=pixels;sig=signature;snprintf(path,sizeof(path),"%s",font_source);two=false;int w=0;TTF_SizeUTF8(f,saved,&w,NULL);
  if(w>width){int best=0,space=0;for(const char*p=saved;*p;){const char*next=NxtText_nextCluster(p);if(!next||next<=p)break;int n=next-saved;memcpy(first,saved,n);first[n]=0;TTF_SizeUTF8(f,first,&w,NULL);if(w>width)break;best=n;if(*p==' ')space=p-saved;p=next;}
   if(space>best/2)best=space;if(best>0&&saved[best]){memcpy(first,saved,best);first[best]=0;const char*p=saved+best;while(*p==' ')p++;snprintf(second,sizeof(second),"%s",p);two=true;}else snprintf(first,sizeof(first),"%s",saved);
  }
 }
 if(two){int h=(height-2)/2;NxtSkin_styledText(s,pixels,first,x,y,width,h,2,255,center);NxtSkin_styledText(s,pixels,second,x,y+h+2,width,h,2,255,center);}
 else NxtSkin_styledText(s,pixels,value,x,y,width,height,2,255,center);
}
void NxtSkin_karaokeText(SDL_Surface *s,int pixels,int index,int x,int y,int width,int height,SDL_Color base,SDL_Color sung,bool center){
 const char *value=NxtLyricPreview_line(index);int count=NxtLyricPreview_wordCount(index);
 if(!value)return;if(!count||!nxt.animations){NxtSkin_styledText(s,pixels,value,x,y,width,height,0,sung.a,center);return;}
 TTF_Font *font=lyric_font(s,pixels,false,nxt.lyric_bold);base=NxtSkin_lyricColor(4,base.a);sung=NxtSkin_lyricColor(0,sung.a);SDL_Surface *image=NxtSkin_renderText(font,value);SDL_Rect ink;
 if(!image||!text_ink(image,&ink)||ink.w<=0)return;TextCache *entry=glyph_entry(image);if(!entry)return;
 unsigned revision=NxtLyricPreview_active()?0xfffffffeu:Lyrics_revision();
 if(entry->words_revision!=revision||entry->words_line!=index){
  for(int i=0;i<count;i++){const LyricWord *word=NxtLyricPreview_wordAt(index,i);char prefix[2048];int n=word->offset;if(n>2047)n=2047;memcpy(prefix,value,n);prefix[n]=0;int start=0,end=0;TTF_SizeUTF8(font,prefix,&start,NULL);n=word->offset+word->length;if(n>2047)n=2047;memcpy(prefix,value,n);prefix[n]=0;TTF_SizeUTF8(font,prefix,&end,NULL);entry->word_edges[i]=start-ink.x;entry->word_edges[i+1]=end-ink.x;}
  entry->words_revision=revision;entry->words_line=index;
 }
 int position=NxtLyricPreview_active()?NxtLyricPreview_position():NxtPlayback_position()-nxt.lyric_offset;float edge=0;
 for(int i=0;i<count;i++){const LyricWord *word=NxtLyricPreview_wordAt(index,i);if(position<word->time_ms)break;float f=word->end_ms>word->time_ms?(float)(position-word->time_ms)/(word->end_ms-word->time_ms):1.f;if(f>1)f=1;edge=entry->word_edges[i]+(entry->word_edges[i+1]-entry->word_edges[i])*f;}
 int w=ink.w,h=ink.h;if(w>width){h=(long long)h*width/w;w=width;}if(h>height){w=(long long)w*height/h;h=height;}int xx=x+(center?(width-w)/2:0),fill=(int)(edge*w/ink.w);if(fill<0)fill=0;if(fill>w)fill=w;
 decorate=true;fit_fontText(s,font,value,x,y,width,height,base,center);decorate=false;
 SDL_Rect clip;SDL_GetClipRect(s,&clip);SDL_Rect mask={xx,y,fill,height},intersection;if(SDL_IntersectRect(&clip,&mask,&intersection)){SDL_SetClipRect(s,&intersection);fit_fontText(s,font,value,x,y,width,height,sung,center);SDL_SetClipRect(s,&clip);}
}

void NxtSkin_fitText(SDL_Surface *s,int pixels,const char *value,int x,int y,int width,int height,SDL_Color color,bool center){
 if(!value||width<=0||height<=0)return;fit_fontText(s,NxtSkin_font(s,pixels),value,x,y,width,height,color,center);
}
void NxtSkin_fitItalicText(SDL_Surface *s,int pixels,const char *value,int x,int y,int width,int height,SDL_Color color,bool center){
 if(!value||width<=0||height<=0)return;TTF_Font *regular=NxtSkin_font(s,pixels);if(!regular)return;
 int size=unit(s,pixels);if(size<12)size=12;int style=TTF_GetFontStyle(regular)|TTF_STYLE_ITALIC;
 if(translation_italic&&(italic_signature!=signature||italic_pixels!=size||italic_style!=style||strcmp(italic_source,font_source)))forget_italic();
 if(!translation_italic){
  const char *ital=nxt.font_path[0]&&!(nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font)?NULL:NxtText_italicPath(nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font?17:nxt.font_index);
  const char *regular_path=nxt.font_path[0]&&!(nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font)?nxt.font_path:NxtText_fontPath(skin_face(nxt.interface_style));
  translation_italic=NxtText_open(ital?ital:regular_path,size);
  if(translation_italic){TTF_SetFontStyle(translation_italic,style);italic_signature=signature;italic_pixels=size;italic_style=style;snprintf(italic_source,sizeof(italic_source),"%s",font_source);}
 }
 fit_fontText(s,translation_italic?translation_italic:regular,value,x,y,width,height,color,center);
}
void NxtSkin_lyricPair(SDL_Surface *s,int index,int x,int y,int width,int height,bool center){
 if(NxtLyricPreview_active())index=1;
 static int last_index=-2;static unsigned last_generation,last_revision;static Uint32 began;
 unsigned generation=NxtPlayback_generation(),revision=Lyrics_revision();Uint32 now=SDL_GetTicks();
 if(index!=last_index||generation!=last_generation||revision!=last_revision){began=index==last_index+1&&generation==last_generation&&revision==last_revision?now:now-360;last_index=index;last_generation=generation;last_revision=revision;}
 int dx=0,dy=0;Uint8 alpha=255;float t=NxtLyricFX_progress(NxtLyricPreview_active()?NxtLyricPreview_age():(Uint32)(now-began));NxtLyricFX_transform(t,s->w,&dx,&dy,&alpha);
 if(NxtLyricFX_slides())dy+=(int)((1.f-t)*12*s->w/1024.f);x+=dx;y+=dy;
 SDL_Color muted=NxtSkin_color(NXT_MUTED),ink=NxtSkin_color(NXT_INK),accent=NxtSkin_color(NXT_ACCENT);muted.a=ink.a=accent.a=alpha;
 const char *original=NxtLyricPreview_line(index),*translated=NxtLyricPreview_translation(index);int mode=NxtLyricPreview_active()?2:nxt.translation_mode;
 if(mode==2&&original){
  int gap=unit(s,4),row=(height-gap)/2;if(row<1)return;
  NxtSkin_karaokeText(s,nxt.lyric_size,index,x,y,width,row,muted,ink,center);
  NxtSkin_styledText(s,nxt.lyric_size,translated&&*translated?translated:Nxt_text("Translating…","Đang dịch…"),x,y+row+gap,width,row,1,alpha,center);
 }else if(mode==1&&translated&&*translated)NxtSkin_styledText(s,nxt.lyric_size,translated,x,y,width,height,1,alpha,center);
 else NxtSkin_karaokeText(s,nxt.lyric_size,index,x,y,width,height,muted,accent,center);
}
void NxtSkin_release(void){NxtMotion_release();forget_lyric_faces();forget_italic();forget_art();NxtTheme_release();NxtCustom_release();NxtMonitor_release();NxtSkin_forgetFont(NULL);glyph_clock=0;scrolling_text=false;for(int i=0;i<PANEL_COUNT;i++)drop_panel(&panels[i]);panel_clock=0;NxtAppearance_release();NxtRecord_release();for(int i=0;i<nf;i++)TTF_CloseFont(fonts[i]);nf=0;signature=0;for(int i=0;i<ICON_COUNT;i++)drop_icon(&icons[i]);icon_clock=0;for(int i=0;i<4;i++){if(source_logos[i])SDL_FreeSurface(source_logos[i]);source_logos[i]=NULL;}screen_source=NXT_SOURCE_AUTO;}
int NxtSkin_status(SDL_Surface *s){
    int p=unit(s,32),y=unit(s,24);
    int charge=0,battery=0;PLAT_getBatteryStatusFine(&charge,&battery);char pct[20],clock[20];snprintf(pct,20,"%d%%",battery);
    time_t now=time(NULL);struct tm local;localtime_r(&now,&local);strftime(clock,20,"%H:%M",&local);
    int right=s->w-p;

    if(battery<0)battery=0;if(battery>100)battery=100;snprintf(pct,sizeof(pct),"%d%%",battery);
    SDL_Rect box={right-unit(s,87),y+unit(s,8),unit(s,82),unit(s,32)};
    NxtSkin_rect(s,box,NxtSkin_color(NXT_MUTED),unit(s,3));NxtSkin_rect(s,(SDL_Rect){box.x+unit(s,2),box.y+unit(s,2),box.w-unit(s,4),box.h-unit(s,4)},NxtSkin_color(NXT_BG),unit(s,2));NxtSkin_rect(s,(SDL_Rect){box.x+box.w,box.y+unit(s,10),unit(s,5),unit(s,12)},NxtSkin_color(NXT_MUTED),unit(s,1));
    SDL_Surface *percent=NxtSkin_renderText(NxtSkin_font(s,18),pct);if(percent){SDL_Color ink=NxtSkin_color(NXT_INK);SDL_SetSurfaceColorMod(percent,ink.r,ink.g,ink.b);SDL_SetSurfaceAlphaMod(percent,255);SDL_BlitSurface(percent,NULL,s,&(SDL_Rect){box.x+(box.w-percent->w)/2,box.y+(box.h-percent->h)/2,0,0});}if(charge)NxtSkin_icon(s,"zap",box.x-unit(s,25),y+unit(s,10),unit(s,22),NxtSkin_color(NXT_ACCENT));
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
    NxtMotion_page(s,title,true);list_top=0;scrolling_text=false;NxtRecord_hide();
    GFX_clear(s);UiLayer_clear(UI_LAYER_STATUS);UiLayer_clear(UI_LAYER_ANIMATION);UiLayer_clear(UI_LAYER_SELECTION);
    NxtAppearance_background(s);
    NxtTheme_chrome(s,false);
    NxtTheme_header(true);
    NxtSkin_panel(s,(SDL_Rect){0,0,s->w,unit(s,132)},NxtSkin_color(NXT_BG),0);
    NxtTheme_header(false);
    NxtSkin_panel(s,(SDL_Rect){0,s->h-unit(s,178),s->w,unit(s,178)},NxtSkin_color(NXT_BG),0);
    bool home=title&&(!strncmp(title,"Chào ",6)||!strncmp(title,"Hello ",6));
    if(!home){SDL_Color body=NxtSkin_color(NXT_BG);if(nxt.wallpaper_mode&&!nxt.interface_style)body.a=nxt.panel_opacity*255/300;NxtSkin_rect(s,(SDL_Rect){unit(s,22),unit(s,142),s->w-unit(s,44),s->h-unit(s,242)},body,unit(s,12));}
    NxtTheme_header(true);
    int p=unit(s,32),y=unit(s,24),logo=unit(s,46);
    NxtSkin_rect(s,(SDL_Rect){p,y,logo,logo},NxtSkin_color(NXT_PANEL),unit(s,12));
    NxtSkin_icon(s,home?NxtPersonal_avatar():"headphones",p+unit(s,6),y+unit(s,5),unit(s,34),NxtSkin_color(NXT_ACCENT));
    if(!home)for(int i=0;i<3;i++)NxtSkin_rect(s,(SDL_Rect){p+unit(s,17+i*5),y+unit(s,33-(i==1?13:i==0?7:10)),unit(s,3),unit(s,i==1?13:i==0?7:10)},NxtSkin_color(NXT_ACCENT),1);
    int status_width=NxtSkin_status(s);
    NxtSkin_compactText(s,24,NXT_APP_NAME,p+logo+unit(s,14),y+unit(s,2),s->w-status_width-p-logo-unit(s,26),unit(s,34),NxtSkin_color(NXT_INK));
    NxtSkinSource source=header_source(title);int detail_width=detail&&*detail?unit(s,270):0;
    int brand_x=s->w-p-unit(s,164)-(detail_width?detail_width+unit(s,18):0);
    int title_width=source!=NXT_SOURCE_AUTO?brand_x-p-unit(s,24):s->w-p*2-(detail_width?detail_width+unit(s,20):0);
    if(title)NxtSkin_compactText(s,32,title,p,unit(s,87),title_width,unit(s,39),NxtSkin_color(NXT_INK));
    if(source!=NXT_SOURCE_AUTO)source_brand(s,source,brand_x,unit(s,88));
    if(detail_width)NxtSkin_compactText(s,19,detail,s->w-p-detail_width,unit(s,92),detail_width,unit(s,27),NxtSkin_color(NXT_MUTED));
    NxtTheme_header(false);
    if(mini)NxtSkin_mini(s);
}
int NxtSkin_top(SDL_Surface *s){return unit(s,145);}
int NxtSkin_bottom(SDL_Surface *s,bool mini){return s->h-unit(s,mini?178:101);}
/* Align the visible glyphs, including Vietnamese marks, rather than a font's
 * transparent ascent/descent padding. Compact hints must fit without scrolling. */
static SDL_Surface *footer_text(SDL_Surface *s,const char *text,int pixels,SDL_Color color,int width,int height,SDL_Rect *ink,int *ascent){
    for(;pixels>=12;pixels--){
        TTF_Font *font=NxtSkin_font(s,pixels);SDL_Surface *rendered=NxtSkin_renderText(font,text);if(!rendered||!text_ink(rendered,ink))return NULL;
        if(ink->w>0&&ink->h>0&&ink->w<=width&&ink->h<=height){if(ascent)*ascent=TTF_FontAscent(font);SDL_SetSurfaceColorMod(rendered,color.r,color.g,color.b);SDL_SetSurfaceAlphaMod(rendered,color.a);return rendered;}
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
        NxtSkin_rect(s,(SDL_Rect){x,y+unit(s,13),w,unit(s,64)},NxtSkin_color(i==1?NXT_ACCENT:NXT_PANEL),unit(s,NxtTheme_radius(12)));
        SDL_Color c=NxtSkin_color(i==1?NXT_ACCENT_INK:NXT_INK),muted=NxtSkin_color(i==1?NXT_ACCENT_INK:NXT_MUTED);
        const char *key="—";for(int k=0;k<16;k++)if(nxt.playing_keys[k]==actions[i]){key=Nxt_keyNames[k];break;}
        SDL_Rect ink;int key_y=y+unit(s,31),space=unit(s,9);
        SDL_Surface *text=footer_text(s,key,15,c,w-2*inside-icon-space,unit(s,22),&ink,NULL);
        int key_w=text?ink.w:0,group_x=cx-(key_w+space+icon)/2;
        if(text){SDL_BlitSurface(text,&ink,s,&(SDL_Rect){group_x,key_y-ink.h/2,0,0});}
        NxtSkin_icon(s,symbol[i],group_x+key_w+space,key_y-icon/2,icon,c);
        int ascent=0;text=footer_text(s,caption[i],18,muted,w-2*inside,unit(s,25),&ink,&ascent);
        if(text){SDL_BlitSurface(text,&ink,s,&(SDL_Rect){cx-ink.w/2,y+unit(s,67)-ascent+ink.y,0,0});}
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
    int titleheight=0,subheight=0;TTF_SizeUTF8(NxtSkin_font(s,25*nxt.font_size/100),"Tiếng Việt ไทย語 한국어",NULL,&titleheight);TTF_SizeUTF8(NxtSkin_font(s,18),"Tiếng Việt ไทย語 한국어",NULL,&subheight);int p=unit(s,32),top=list_top>0?list_top:NxtSkin_top(s),bottom=NxtSkin_bottom(s,mini),row=unit(s,78)*nxt.font_size/100;bool subtitles=false;for(int i=0;i<count;i++)if(items[i]&&strchr(items[i],'\n')){subtitles=true;break;}int minrow=titleheight+(subtitles?subheight:0)+unit(s,21);if(row<minrow)row=minrow;list_top=0;
    int visible=(bottom-top)/row;if(visible<1)visible=1;int first=selected-visible/2;if(first<0)first=0;if(first>count-visible)first=count-visible;if(first<0)first=0;
    for(int i=first;i<count&&i<first+visible;i++){
        int y=top+(i-first)*row;bool chosen=i==selected;bool reference=chosen&&nxt.interface_style==NXT_THEME_AURALIS;SDL_Color selected_ink=NxtSkin_color(reference?NXT_ACCENT_INK:NXT_INK);
        if(reference)NxtSkin_rect(s,(SDL_Rect){p,y,s->w-p*2,row-unit(s,5)},NxtSkin_color(NXT_ACCENT),unit(s,24));
        else if(chosen){NxtSkin_panel(s,(SDL_Rect){p,y,s->w-p*2,row-unit(s,5)},NxtSkin_color(NXT_PANEL),unit(s,11));NxtSkin_rect(s,(SDL_Rect){p,y+unit(s,12),unit(s,3),row-unit(s,29)},NxtSkin_color(NXT_ACCENT),1);}
        const char *text=items[i]?items[i]:"";const char *line=strchr(text,'\n'),*value=strchr(text,'\t');char title[2048];size_t len=line?(size_t)(line-text):strlen(text);if(value&&(size_t)(value-text)<len)len=value-text;if(len>=sizeof(title))len=sizeof(title)-1;memcpy(title,text,len);title[len]=0;
        int texty=y+(line?unit(s,8):(row-titleheight)/2),labelwidth=value?(s->w-p*2)*55/100:s->w-p*2-unit(s,68);NxtSkin_text(s,25*nxt.font_size/100,title,p+unit(s,19),texty,labelwidth,selected_ink);
        if(value){char val[1024];snprintf(val,sizeof(val),"%s",value+1);char *newline=strchr(val,'\n');if(newline)*newline=0;int vw=0;TTF_SizeUTF8(NxtSkin_font(s,21),val,&vw,NULL);int limit=(s->w-p*2)*40/100;if(vw>limit)vw=limit;NxtSkin_text(s,21,val,s->w-p-unit(s,43)-vw,texty+unit(s,3),vw,NxtSkin_color(reference?NXT_ACCENT_INK:NXT_ACCENT));}
        if(line)NxtSkin_text(s,18,line+1,p+unit(s,19),y+unit(s,11)+titleheight,s->w-p*2-unit(s,68),NxtSkin_color(reference?NXT_ACCENT_INK:NXT_MUTED));
        NxtSkin_icon(s,"chevron-right",s->w-p-unit(s,34),y+row/2-unit(s,10),unit(s,21),NxtSkin_color(reference?NXT_ACCENT_INK:chosen?NXT_ACCENT:NXT_MUTED));
        if(!chosen)NxtSkin_rect(s,(SDL_Rect){p+unit(s,14),y+row-unit(s,4),s->w-p*2-unit(s,28),1},NxtSkin_color(NXT_LINE),0);
    }
    if(count>visible){int trackh=bottom-top,thumb=trackh*visible/count;if(thumb<unit(s,14))thumb=unit(s,14);NxtSkin_rect(s,(SDL_Rect){s->w-p+unit(s,13),top,unit(s,3),trackh},NxtSkin_color(NXT_PANEL),1);NxtSkin_rect(s,(SDL_Rect){s->w-p+unit(s,13),top+(trackh-thumb)*first/(count-visible),unit(s,3),thumb},NxtSkin_color(NXT_ACCENT),1);}
    if(!count)NxtSkin_text(s,25,Nxt_text("No items yet.","Chưa có nội dung."),p+unit(s,14),top+unit(s,20),s->w-p*2,NxtSkin_color(NXT_MUTED));
}
void NxtSkin_homeHint(SDL_Surface *s,int selected,const char *hint){
 char greeting[180];snprintf(greeting,sizeof(greeting),"%s%s",Nxt_text("Hello ","Chào "),NxtPersonal_name());NxtSkin_begin(s,greeting,NULL,true);
 if(NxtTheme_home(s,selected)){NxtSkin_footer(s,hint?hint:Nxt_text("D-pad Select · A Open · Y Now playing · START Options","D-pad Chọn · A Mở · Y Đang phát · START Tùy chọn"),false);return;}
 const char *labels[]={Nxt_text("Library","Thư viện"),"Playlist",Nxt_text("Favorites","Yêu thích"),Nxt_text("Online music","Nhạc online"),Nxt_text("Personal","Cá nhân"),Nxt_text("Settings","Cài đặt"),Nxt_text("About, help & updates","Thông tin, hướng dẫn & cập nhật")};
 const char *subs[]={Nxt_text("Folders · artists · albums","Thư mục · nghệ sĩ · album"),Nxt_text("Create and edit playlists","Tạo và sửa danh sách"),Nxt_text("Your favorite tracks","Những bài bạn yêu thích"),"Spotify · SoundCloud · Drive",Nxt_text("For you · statistics · history · profile","Dành cho bạn · thống kê · hồ sơ"),Nxt_text("Audio · files · performance · keys · language","Âm thanh · tệp · hiệu năng · phím · ngôn ngữ"),Nxt_text("Usage guide · changes · OTA · logs","Hướng dẫn sử dụng · thay đổi · OTA · log")};
 const char *symbols[]={"library","list-music","heart","radio","headphones","settings-2","info"};
 int p=unit(s,32),gap=unit(s,12),w=(s->w-p*2-gap)/2,h=unit(s,90),top=unit(s,149);
 for(int i=0;i<7;i++){
  int x=i==6?p:p+(i%2)*(w+gap),y=top+(i==6?3:i/2)*(h+gap),cw=i==6?s->w-p*2:w;
  if(i==selected)NxtSkin_rect(s,(SDL_Rect){x,y,cw,h},NxtSkin_color(NXT_ACCENT),unit(s,13));else NxtSkin_panel(s,(SDL_Rect){x,y,cw,h},NxtSkin_color(NXT_PANEL),unit(s,13));
  SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK),muted=i==selected?ink:NxtSkin_color(NXT_MUTED);
  NxtSkin_icon(s,NxtCustom_menuIcon(i),x+unit(s,19),y+unit(s,29),unit(s,28),ink);NxtSkin_text(s,25,labels[i],x+unit(s,60),y+unit(s,9),cw-unit(s,72),ink);NxtSkin_text(s,18,subs[i],x+unit(s,60),y+unit(s,49),cw-unit(s,72),muted);
 }
 NxtSkin_mini(s);NxtSkin_footer(s,hint?hint:Nxt_text("D-pad Select · A Open · Y Now playing · START Options","D-pad Chọn · A Mở · Y Đang phát · START Tùy chọn"),false);
}
void NxtSkin_home(SDL_Surface *s,int selected){NxtSkin_homeHint(s,selected,NULL);}
void NxtSkin_tabs(SDL_Surface *s,const char *const *labels,int count,int selected,int y){int x=unit(s,32);for(int i=0;i<count;i++){int w=0;TTF_SizeUTF8(NxtSkin_font(s,21),labels[i],&w,NULL);w+=unit(s,28);if(i==selected)NxtSkin_rect(s,(SDL_Rect){x,y,w,unit(s,43)},NxtSkin_color(NXT_ACCENT),unit(s,10));NxtSkin_text(s,21,labels[i],x+unit(s,14),y+unit(s,4),w-unit(s,24),NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK));x+=w+unit(s,8);}list_top=y+unit(s,58);}
void NxtSkin_playing(SDL_Surface *s,int track,int total,bool shuffle,int repeat){
    if(nxt.interface_style){
        NxtMotion_page(s,"@now-playing",false);
        NxtTheme_counters(track,total);NxtRecord_hide();scrolling_text=false;
        UiLayer_clear(UI_LAYER_STATUS);UiLayer_clear(UI_LAYER_ANIMATION);UiLayer_clear(UI_LAYER_SELECTION);
        NxtTheme_playing(s,shuffle,repeat);return;
    }
    NxtTheme_counters(track,total);
    char counter[32];snprintf(counter,32,"%02d / %02d",track,total);
    NxtSkinSource previous=NxtSkin_setSource(playback_source());
    NxtSkin_begin(s,Nxt_text("Now playing","Đang phát"),counter,false);
    NxtMotion_setMenu(false); /* One identity per page; never restart on each playback redraw. */
    NxtSkin_setSource(previous);
    int p=unit(s,32),top=unit(s,147),aw=unit(s,242),x=p+aw+unit(s,32),w=s->w-p-x;
    SDL_Surface *cover=NxtPlayback_art();if(!cover&&!NxtSoundCloud_hasMetadata()&&!NxtSpotify_hasMetadata())cover=album_art_get();
    if(nxt.cover_mode)NxtRecord_draw(s,(SDL_Rect){p,top,aw,aw},cover,NxtPlayback_generation(),nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING);else{
        NxtSkin_rect(s,(SDL_Rect){p,top,aw,aw},NxtSkin_color(NXT_PANEL),unit(s,10));
        if(cover&&cover->w>0&&cover->h>0){
            int cw=aw,ch=aw;
            if(cover->w>cover->h)ch=(int)((long long)aw*cover->h/cover->w);else cw=(int)((long long)aw*cover->w/cover->h);
            if(cw<1)cw=1;if(ch<1)ch=1;
            NxtSkin_cachedArt(s,cover,(SDL_Rect){p+(aw-cw)/2,top+(aw-ch)/2,cw,ch});
        }else NxtSkin_icon(s,"music-2",p+aw/3,top+aw/3,aw/3,NxtSkin_color(NXT_ACCENT));
    }
    const TrackInfo *info=NxtPlayback_info();int titlepx=36,tw=0;TTF_SizeUTF8(NxtSkin_font(s,titlepx),info->title,&tw,NULL);
    bool compact=tw>w&&tw<=w*2;bool scroll_title=tw>w*2;if(compact)titlepx=30;else if(scroll_title)titlepx=32;
    int y=top;TTF_Font *titlefont=NxtSkin_font(s,titlepx);
    if(scroll_title){int th=0;TTF_SizeUTF8(titlefont,info->title,NULL,&th);NxtSkin_text(s,titlepx,info->title,x,y,w,NxtSkin_color(NXT_INK));y+=th+unit(s,5);}
    else y+=Nxt_drawWrapped(s,titlefont,info->title,x,y,w,unit(s,110),NxtSkin_color(NXT_INK))+unit(s,5);
    NxtSkin_text(s,compact?25:27,info->artist[0]?info->artist:Nxt_text("Unknown artist","Chưa rõ nghệ sĩ"),x,y,w,NxtSkin_color(NXT_INK));y+=unit(s,compact?33:42);
    NxtSkin_text(s,compact?20:23,info->album,x,y,w,NxtSkin_color(NXT_MUTED));y+=unit(s,compact?31:40);
    char format[256],part[64];snprintf(format,sizeof(format),"%s",info->codec[0]?info->codec:Nxt_text("Source","Nguồn"));
#define SOURCE_FIELD(fmt,value) do{snprintf(part,sizeof(part),fmt,value);strncat(format,part,sizeof(format)-strlen(format)-1);}while(0)
    if(info->sample_rate>0)SOURCE_FIELD(" · %.1f kHz",info->sample_rate/1000.f);
    if(info->bit_depth>0)SOURCE_FIELD(" · %d-bit",info->bit_depth);
    if(info->bitrate>0)SOURCE_FIELD(" · %d kbps",info->bitrate/1000);
    if(info->channels>0)SOURCE_FIELD(" · %d ch",info->channels);
#undef SOURCE_FIELD
    NxtSkin_text(s,compact?17:18,format,x,y,w,NxtSkin_color(NXT_MUTED));y+=unit(s,compact?28:33);
    NxtSkin_icon(s,Player_isUSBDACActive()?"usb":nxt.output==1?"bluetooth":NxtHardware_hasJack()?"headphones":"volume-2",x,y+unit(s,3),unit(s,23),NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,20,NxtHardware_output(),x+unit(s,34),y,w-unit(s,34),NxtSkin_color(NXT_ACCENT));
    if(Player_isUSBDACActive()){
        y+=unit(s,compact?26:32);
        NxtSkin_text(s,18,NxtHardware_pcm(),x,y,w,NxtSkin_color(NXT_MUTED));
    }
    int lyric_y=top+aw+unit(s,17);if(y+unit(s,compact?29:45)>lyric_y)lyric_y=y+unit(s,compact?29:45);
    int spectrum_y=s->h-unit(s,266),spectrum_h=unit(s,49);bool show_lyrics=NxtLyricPreview_active()||Settings_getBool(&SETTING_LYRICS_ENABLED);
    int needed=unit(s,nxt.translation_mode==2?70:44),spectrum_end=spectrum_y+spectrum_h;
    if(show_lyrics&&lyric_y+needed>spectrum_y){spectrum_y=lyric_y+needed;spectrum_h=spectrum_end-spectrum_y;
     if(spectrum_h<unit(s,16)){spectrum_h=unit(s,16);spectrum_y=spectrum_end-spectrum_h;lyric_y=spectrum_y-needed;}}
    if(show_lyrics&&lyric_y>=top&&spectrum_y>lyric_y){int current=Lyrics_canFollow()?Lyrics_currentIndex(NxtPlayback_position()):0;if(current<0)current=0;
     if(NxtLyricPreview_active()||Lyrics_lineText(current))NxtSkin_lyricPair(s,current,p,lyric_y,s->w-p*2,spectrum_y-lyric_y-unit(s,7),false);
     else if(NxtSoundCloud_active())NxtSkin_text(s,20,NxtSoundCloud_lyricsStatus(),p,lyric_y,s->w-p*2,NxtSkin_color(NXT_MUTED));
     else if(NxtSpotify_active())NxtSkin_text(s,20,NxtSpotify_lyricsStatus(),p,lyric_y,s->w-p*2,NxtSkin_color(NXT_MUTED));else NxtSkin_fitText(s,20,Lyrics_status(),p,lyric_y,s->w-p*2,spectrum_y-lyric_y-unit(s,7),NxtSkin_color(NXT_MUTED),false);}
    Spectrum_setPosition(p,spectrum_y,s->w-p*2,spectrum_h);
    int elapsed=NxtPlayback_position()/1000,duration=NxtPlayback_duration()/1000,left=duration>elapsed?duration-elapsed:0;
    NxtSkin_rect(s,(SDL_Rect){p,s->h-unit(s,203),s->w-p*2,unit(s,6)},NxtSkin_color(NXT_PANEL),unit(s,3));
    if(duration>0)NxtSkin_rect(s,(SDL_Rect){p,s->h-unit(s,203),(s->w-p*2)*elapsed/duration,unit(s,6)},NxtSkin_color(NXT_ACCENT),unit(s,3));
    char time[40];snprintf(time,40,"%02d:%02d",elapsed/60,elapsed%60);NxtSkin_text(s,20,time,p,s->h-unit(s,189),unit(s,120),NxtSkin_color(NXT_INK));snprintf(time,40,"−%02d:%02d",left/60,left%60);NxtSkin_text(s,20,time,s->w-p-unit(s,86),s->h-unit(s,189),unit(s,90),NxtSkin_color(NXT_MUTED));
    int mode_y=s->h-unit(s,140),remaining=Nxt_timerRemaining();char timer[32],volume[32];snprintf(timer,sizeof(timer),remaining?"%d:%02d":"--:--",remaining/60,remaining%60);snprintf(volume,sizeof(volume),"%d%%",NxtVolume_getPercent());
    const char *symbols[]={"shuffle",repeat==1?"repeat-1":"repeat","timer","volume-2","heart","audio-lines","arrow-left"};const char *caps[]={shuffle?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"),repeat==1?Nxt_text("One","Một bài"):repeat==2?Nxt_text("All","Tất cả"):Nxt_text("Off","Tắt"),timer,volume,Nxt_text("Favorite","Yêu thích"),Nxt_text("Spectrum","Phổ nhạc"),Nxt_text("Back","Quay lại")};int actions[]={NC_QUICK,NC_QUICK,NC_QUICK,NC_NONE,NC_FAVORITE,NC_SPECTRUM,NC_MENU};bool active[]={shuffle,repeat!=0,remaining>0,false,NxtPlayback_favorite(),false,false};
    for(int i=0;i<7;i++){int xx=p+i*unit(s,138),ww=unit(s,127);SDL_Color c=NxtSkin_color(active[i]?NXT_ACCENT_INK:NXT_INK);NxtSkin_rect(s,(SDL_Rect){xx,mode_y,ww,unit(s,42)},NxtSkin_color(active[i]?NXT_ACCENT:NXT_PANEL),unit(s,8));NxtSkin_icon(s,symbols[i],xx+unit(s,7),mode_y+unit(s,10),unit(s,21),c);NxtSkin_compactText(s,15,caps[i],xx+unit(s,33),mode_y+unit(s,3),ww-unit(s,39),unit(s,17),c);const char *key=i==3?"SELECT + ↑↓":"";for(int k=0;actions[i]&&k<16;k++)if(nxt.playing_keys[k]==actions[i]){key=Nxt_keyNames[k];break;}NxtSkin_compactText(s,12,key,xx+unit(s,33),mode_y+unit(s,23),ww-unit(s,39),unit(s,14),c);}
    NxtSkin_footer(s,NULL,true);
}
