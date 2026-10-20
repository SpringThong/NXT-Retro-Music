/* SPDX-License-Identifier: GPL-3.0-only
 * Small, cached vinyl renderer. No audio access, timers, threads or new assets. */
#include "nxt_record.h"
#include "nxt_skin.h"
#include "nxt_performance.h"
#include "ui_layers.h"
#include "api.h"
#include <math.h>
#include <stdlib.h>

#define RECORD_TURN_MS 12000.0f
#define RECORD_ARM_MS 240.0f

static SDL_Surface *platter,*label,*animation,*projected;
static const SDL_Surface *cached_art;
static unsigned char *circle_mask;
static SDL_Rect area;
static SDL_Color cached_bg,cached_accent;
static unsigned track_generation;
static bool have_track,visible,clock_set,was_playing,invalid;
static Uint32 last_frame;
static float phase,arm;
static int label_x,label_y;

static float clamp(float n,float lo,float hi){return n<lo?lo:n>hi?hi:n;}
static SDL_Color tint(SDL_Color a,SDL_Color b,float t){
    return (SDL_Color){a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t,255};
}
/* Straight-alpha composition; all our cached surfaces have ARGB8888 pixels. */
static void dot(SDL_Surface *s,int x,int y,SDL_Color c,float coverage){
    if(x<0||y<0||x>=s->w||y>=s->h)return;
    unsigned a=(unsigned)(clamp(coverage,0,1)*c.a+.5f);if(!a)return;
    Uint32 *p=(Uint32 *)((char *)s->pixels+y*s->pitch)+x,d=*p;
    unsigned old=d>>24,remain=old*(255-a)/255,total=a+remain;
    unsigned r=(c.r*a+((d>>16)&255)*remain)/total;
    unsigned g=(c.g*a+((d>>8)&255)*remain)/total;
    unsigned b=(c.b*a+(d&255)*remain)/total;
    *p=(total<<24)|(r<<16)|(g<<8)|b;
}
static void circle(SDL_Surface *s,float cx,float cy,float radius,SDL_Color color){
    int x0=(int)floorf(cx-radius-1),x1=(int)ceilf(cx+radius+1);
    int y0=(int)floorf(cy-radius-1),y1=(int)ceilf(cy+radius+1);
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)
        dot(s,x,y,color,radius+.5f-hypotf(x-cx,y-cy));
}
static void stroke(SDL_Surface *s,float x0,float y0,float x1,float y1,float width,SDL_Color color){
    float dx=x1-x0,dy=y1-y0,length=dx*dx+dy*dy;
    int left=(int)floorf(fminf(x0,x1)-width),right=(int)ceilf(fmaxf(x0,x1)+width);
    int top=(int)floorf(fminf(y0,y1)-width),bottom=(int)ceilf(fmaxf(y0,y1)+width);
    for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){
        float t=length?clamp(((x-x0)*dx+(y-y0)*dy)/length,0,1):0;
        dot(s,x,y,color,width*.5f+.5f-hypotf(x-x0-t*dx,y-y0-t*dy));
    }
}
static void free_cache(void){
    SDL_FreeSurface(platter);SDL_FreeSurface(label);SDL_FreeSurface(animation);SDL_FreeSurface(projected);
    platter=label=animation=projected=NULL;free(circle_mask);circle_mask=NULL;cached_art=NULL;
}
void NxtRecord_release(void){
    free_cache();visible=have_track=clock_set=false;phase=arm=0;invalid=true;
}
void NxtRecord_hide(void){visible=false;}
static bool suspended(void){return PWR_getDisplayOff()||NxtSession_isHidden();}

static bool make_platter(int size){
    free_cache();cached_bg=NxtSkin_color(NXT_BG);cached_accent=NxtSkin_color(NXT_ACCENT);
    platter=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_ARGB8888);
    animation=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_ARGB8888);
    int diameter=(int)(size*.66f)+2;
    label=SDL_CreateRGBSurfaceWithFormat(0,diameter,diameter,32,SDL_PIXELFORMAT_ARGB8888);
    circle_mask=malloc((size_t)diameter*diameter);
    if(!platter||!animation||!label||!circle_mask){free_cache();return false;}
    SDL_SetSurfaceBlendMode(platter,SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceBlendMode(animation,SDL_BLENDMODE_BLEND);
    SDL_FillRect(platter,NULL,0);
    float cx=size*.465f,cy=size*.515f,radius=size*.449f;
    SDL_Color vinyl=tint((SDL_Color){28,35,32,255},cached_accent,.10f);
    SDL_Color groove=tint(vinyl,NxtSkin_color(NXT_MUTED),.16f);
    circle(platter,cx+size*.009f,cy+size*.016f,radius,(SDL_Color){0,0,0,80});
    for(int y=0;y<size;y++)for(int x=0;x<size;x++){
        float dx=x-cx,dy=y-cy,r=hypotf(dx,dy),coverage=radius+.5f-r;
        if(coverage<=0)continue;
        float light=clamp((dx-dy)/radius,-1,1)*.07f+.07f;
        SDL_Color color=tint(vinyl,groove,light);
        if(r>radius*.75f){
            float rings=.5f+.5f*cosf(r*2.35f);
            color=tint(color,groove,rings*.55f);
        }
        if(r>radius-2)color=tint(groove,cached_accent,.12f);
        dot(platter,x,y,color,coverage);
    }
    label_x=(int)lroundf(cx-(diameter-1)*.5f);
    label_y=(int)lroundf(cy-(diameter-1)*.5f);
    float center=(diameter-1)*.5f,r=diameter*.5f-1;
    for(int y=0;y<diameter;y++)for(int x=0;x<diameter;x++)
        circle_mask[y*diameter+x]=(unsigned char)(255*clamp(r+.5f-hypotf(x-center,y-center),0,1)+.5f);
    invalid=true;return true;
}
static void make_label(SDL_Surface *art){
    cached_art=art;SDL_Color panel=NxtSkin_color(NXT_PANEL);
    SDL_FillRect(label,NULL,SDL_MapRGB(label->format,panel.r,panel.g,panel.b));
    if(art&&art->w>0&&art->h>0){
        int edge=art->w<art->h?art->w:art->h;
        SDL_Rect crop={(art->w-edge)/2,(art->h-edge)/2,edge,edge};
        if(!NxtSkin_smoothScale(art,&crop,label,NULL))SDL_BlitScaled(art,&crop,label,NULL);
    }else{
        int d=label->w;
        circle(label,d*.5f,d*.5f,d*.44f,tint(panel,NxtSkin_color(NXT_ACCENT),.09f));
        NxtSkin_icon(label,"music-2",d/3,d/5,d/3,NxtSkin_color(NXT_ACCENT));
        NxtSkin_text(label,100,"NXT",d/4,d*3/5,d/2,NxtSkin_color(NXT_ACCENT));
    }
    invalid=true;
}
/* Bilinear rotation of the small album label, with a precomputed round mask. */
static void rotate_label(void){
    float radians=phase*(float)(M_PI/180.0);
    int cosine=(int)lroundf(cosf(radians)*65536),sine=(int)lroundf(sinf(radians)*65536);
    int n=label->w,center=(n-1)*32768;
    for(int y=0;y<n;y++){
        Uint32 *output=(Uint32 *)((char *)animation->pixels+(label_y+y)*animation->pitch)+label_x;
        int vy=y*65536-center;
        for(int x=0;x<n;x++){
            unsigned a=circle_mask[y*n+x];if(!a)continue;
            int vx=x*65536-center;
            int sx=(int)(((int64_t)cosine*vx+(int64_t)sine*vy)>>16)+center;
            int sy=(int)((-(int64_t)sine*vx+(int64_t)cosine*vy)>>16)+center;
            sx=sx<0?0:sx>(n-1)*65536?(n-1)*65536:sx;
            sy=sy<0?0:sy>(n-1)*65536?(n-1)*65536:sy;
            int ix=sx>>16,iy=sy>>16,jx=ix+1<n?ix+1:ix,jy=iy+1<n?iy+1:iy;
            unsigned fx=(sx>>8)&255,fy=(sy>>8)&255;
            Uint32 *row=(Uint32 *)((char *)label->pixels+iy*label->pitch);
            Uint32 *next=(Uint32 *)((char *)label->pixels+jy*label->pitch);
            unsigned weights[4]={(256-fx)*(256-fy),fx*(256-fy),(256-fx)*fy,fx*fy};
            Uint32 pixels[4]={row[ix],row[jx],next[ix],next[jx]},value=a<<24;
            for(int shift=0;shift<=16;shift+=8){
                unsigned sum=32768;
                for(int i=0;i<4;i++)sum+=((pixels[i]>>shift)&255)*weights[i];
                value|=(sum>>16)<<shift;
            }
            output[x]=value;
        }
    }
}
static void draw_arm(void){
    float d=animation->w,px=d*.916f,py=d*.095f;
    float radians=(-3+28*arm)*(float)(M_PI/180.0),dx=-sinf(radians),dy=cosf(radians);
    float endx=px+dx*d*.624f,endy=py+dy*d*.624f;
    SDL_Color metal=tint(NxtSkin_color(NXT_MUTED),(SDL_Color){224,232,225,255},.50f);
    circle(animation,px,py,d*.043f,(SDL_Color){11,19,15,255});
    circle(animation,px,py,d*.029f,metal);
    stroke(animation,px+d*.012f,py+d*.015f,endx+d*.012f,endy+d*.015f,d*.034f,(SDL_Color){0,0,0,80});
    stroke(animation,px,py,endx,endy,d*.019f,metal);
    stroke(animation,px-d*.002f,py,endx-d*.002f,endy,d*.005f,(SDL_Color){239,244,236,220});
    stroke(animation,endx-dx*d*.065f,endy-dy*d*.065f,endx,endy,d*.039f,NxtSkin_color(NXT_MUTED));
    stroke(animation,endx,endy,endx+dx*d*.019f,endy+dy*d*.019f,d*.009f,NxtSkin_color(NXT_ACCENT));
    circle(animation,px,py,d*.008f,(SDL_Color){35,50,43,255});
}
static void update(bool playing){
    Uint32 now=SDL_GetTicks(),elapsed=clock_set?now-last_frame:0;
    if(elapsed>150)elapsed=0; /* Never catch up after leaving/waking the screen. */
    if(was_playing)phase=fmodf(phase+elapsed*(360.0f/RECORD_TURN_MS),360.0f);
    float target=playing?1:0,step=elapsed/RECORD_ARM_MS;
    arm=arm<target?fminf(target,arm+step):fmaxf(target,arm-step);
    was_playing=playing;last_frame=now;clock_set=true;invalid=false;
    SDL_FillRect(animation,NULL,0);rotate_label();
    float d=animation->w;
    circle(animation,d*.465f,d*.515f,d*.020f,(SDL_Color){21,30,24,245});
    circle(animation,d*.465f,d*.515f,d*.010f,(SDL_Color){206,219,207,255});
    draw_arm();
}
bool NxtRecord_needsRefresh(bool playing){
    if(!visible||suspended()){clock_set=false;return false;}
    return invalid||!clock_set||playing!=was_playing||
        (SDL_GetTicks()-last_frame>=NxtPerformance_recordMS()&&(playing||fabsf(arm-(playing?1:0))>.001f));
}
void NxtRecord_paint(bool playing){
    if(!visible||!animation)return;
    if(suspended()){clock_set=false;return;}
    if(NxtRecord_needsRefresh(playing))update(playing);
    if(area.w!=area.h){
        if(projected&&(projected->w!=area.w||projected->h!=area.h)){SDL_FreeSurface(projected);projected=NULL;}
        if(!projected)projected=SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_ARGB8888);
        if(projected){SDL_FillRect(projected,NULL,0);SDL_BlitScaled(animation,NULL,projected,NULL);UiLayer_blit(projected,area.x,area.y,UI_LAYER_ANIMATION);}
    }else UiLayer_blit(animation,area.x,area.y,UI_LAYER_ANIMATION);
}
static void draw_record(SDL_Surface *screen,SDL_Rect bounds,SDL_Surface *art,unsigned generation,bool playing,bool tilted){
    if(!screen||bounds.w<16||bounds.h<16||(!tilted&&bounds.w!=bounds.h)){NxtRecord_hide();return;}
    SDL_Color bg=NxtSkin_color(NXT_BG),accent=NxtSkin_color(NXT_ACCENT);
    bool rebuild=!platter||platter->w!=bounds.w||
        bg.r!=cached_bg.r||bg.g!=cached_bg.g||bg.b!=cached_bg.b||
        accent.r!=cached_accent.r||accent.g!=cached_accent.g||accent.b!=cached_accent.b;
    if(rebuild&&!make_platter(bounds.w)){NxtRecord_hide();return;}
    bool changed=!have_track||generation!=track_generation;
    if(changed){phase=0;arm=playing?1:0;was_playing=playing;clock_set=false;track_generation=generation;have_track=true;}
    if(rebuild||changed||cached_art!=art)make_label(art);
    area=bounds;visible=true;
    if(tilted)SDL_BlitScaled(platter,NULL,screen,&area);else SDL_BlitSurface(platter,NULL,screen,&area);
    NxtRecord_paint(playing);
}
void NxtRecord_draw(SDL_Surface *screen,SDL_Rect bounds,SDL_Surface *art,unsigned generation,bool playing){draw_record(screen,bounds,art,generation,playing,false);}
void NxtRecord_drawTilted(SDL_Surface *screen,SDL_Rect bounds,SDL_Surface *art,unsigned generation,bool playing){draw_record(screen,bounds,art,generation,playing,true);}
