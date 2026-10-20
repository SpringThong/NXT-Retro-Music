/* SPDX-License-Identifier: GPL-3.0-only. v1.8.1 approved compact native player. */
#include "nxt_eq.h"
#include "nxt_translate.h"
#include "nxt_controls.h"
#include "nxt_personal.h"
#include "nxt_appearance.h"
#include <SDL2/SDL_image.h>
typedef struct {int x,y,w,h;} Box;
typedef struct {Box song,lyrics,spectrum,progress,art;} Scene;
#include "nxt_v18_scene_data.inc"
static SDL_Surface *chassis,*gray_cover,*gray_source;
static int chassis_style=-1,chassis_width,chassis_height,scene_track,scene_total;static unsigned gray_generation;static int gray_style=-1;
void NxtTheme_counters(int track,int total){scene_track=track;scene_total=total;}
static const char *action_key(int action){for(int i=0;i<16;i++)if(nxt.playing_keys[i]==action)return Nxt_keyNames[i];return "";}
static void bounded(SDL_Surface *s,int px,const char *value,Box b,SDL_Color c){if(b.w>0&&b.h>0)NxtSkin_fitText(s,px,value,u(s,b.x),u(s,b.y),u(s,b.w),u(s,b.h),c,false);}
static int scene_x(int x){return 512+(x-512)*86/100;}
static int scene_y(int y){return 120+(y-120)*86/100;}
static Box scene_fit(Box b){return(Box){scene_x(b.x),scene_y(b.y),b.w*86/100,b.h*86/100};}
static void live_art(SDL_Surface *s,SDL_Surface *art,Box b,int style){
 rect(s,b.x,b.y,b.w,b.h,NxtSkin_color(NXT_PANEL),8);
 if(style==0&&nxt.cover_mode){NxtRecord_draw(s,(SDL_Rect){u(s,b.x),u(s,b.y),u(s,b.w),u(s,b.h)},art,NxtPlayback_generation(),nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING);return;}
 if(!art||art->w<=0||art->h<=0){NxtSkin_icon(s,"music-2",u(s,b.x+b.w/3),u(s,b.y+b.h/3),u(s,b.w/3),NxtSkin_color(NXT_ACCENT));return;}
 unsigned gen=NxtPlayback_generation();int size=u(s,b.w),height=u(s,b.h);if(size<1||height<1)return;
 if(gray_cover&&(gray_source!=art||gray_generation!=gen||gray_cover->w!=size||gray_cover->h!=height||gray_style!=style)){SDL_FreeSurface(gray_cover);gray_cover=NULL;}
 if(!gray_cover){gray_cover=SDL_CreateRGBSurfaceWithFormat(0,size,height,32,SDL_PIXELFORMAT_ARGB8888);if(!gray_cover)return;SDL_FillRect(gray_cover,NULL,0);int w=size,h=(int)((long long)size*art->h/art->w);if(h>height){h=height;w=(int)((long long)height*art->w/art->h);}if(w<1)w=1;if(h<1)h=1;
 if(!NxtSkin_smoothScale(art,NULL,gray_cover,&(SDL_Rect){(size-w)/2,(height-h)/2,w,h}))SDL_BlitScaled(art,NULL,gray_cover,&(SDL_Rect){(size-w)/2,(height-h)/2,w,h});
 if(style==22){SDL_LockSurface(gray_cover);for(int y=0;y<height;y++){Uint32 *row=(Uint32*)((Uint8*)gray_cover->pixels+y*gray_cover->pitch);for(int x=0;x<size;x++){Uint8 r,g,bl,a;SDL_GetRGBA(row[x],gray_cover->format,&r,&g,&bl,&a);Uint8 v=(r*77+g*150+bl*29)>>8;row[x]=SDL_MapRGBA(gray_cover->format,v,v,v,a);}}SDL_UnlockSurface(gray_cover);}
 SDL_SetSurfaceBlendMode(gray_cover,SDL_BLENDMODE_BLEND);gray_source=art;gray_generation=gen;gray_style=style;}
 SDL_BlitSurface(gray_cover,NULL,s,&(SDL_Rect){u(s,b.x),u(s,b.y),0,0});
}
static void scene_button(SDL_Surface *s,int i,int row,const char *icon,const char *key,bool active,bool state_only){
 int x=43+i*136,y=row?688:634,w=122,h=row?37:47;SDL_Color c=NxtSkin_color(active?NXT_ACCENT_INK:NXT_INK);rect(s,x,y,w,h,NxtSkin_color(active?NXT_ACCENT:NXT_PANEL),9);
 int edge=row?27:32,ix=state_only?x+(w-edge)/2:x+12;NxtSkin_icon(s,icon,u(s,ix),u(s,y+(h-edge)/2),u(s,edge),c);
 if(state_only){if(!active)line(s,ix+2,y+(h-edge)/2+edge-2,ix+edge-2,y+(h-edge)/2+2,NxtSkin_color(NXT_MUTED),1);return;}
 bounded(s,row?12:15,key,(Box){x+46,y+5,w-51,h-10},c);
}
static void quality_value(char *out,size_t size,const TrackInfo *t){
 snprintf(out,size,"%s",t->codec);char part[64];
#define ADD(fmt,v) do{snprintf(part,sizeof(part),fmt,v);strncat(out,part,size-strlen(out)-1);}while(0)
 if(t->sample_rate>0)ADD(" · %.1f kHz",t->sample_rate/1000.f);if(t->bit_depth>0)ADD(" · %d bit",t->bit_depth);if(t->bitrate>0)ADD(" · %d kbps",t->bitrate/1000);if(t->channels>0)ADD(" · %d ch",t->channels);
#undef ADD
}
bool NxtTheme_playing(SDL_Surface *s,bool shuffle,int repeat){
 if(nxt.interface_style==NXT_THEME_AURALIS)return NxtAuralis_playing(s,scene_track,scene_total,shuffle,repeat);
 int style=nxt.interface_style;if(style<0||style>=NXT_THEME_COUNT)style=0;bool bilingual=NxtLyricPreview_active()||(nxt.translation_mode==2&&Settings_getBool(&SETTING_LYRICS_ENABLED));int body_end=bilingual?520:548;Scene f=scenes[style];f.art=scene_fit(f.art);f.song=scene_fit(f.song);f.spectrum=scene_fit(f.spectrum);if(style==29){f.art=(Box){365,145,294,294};f.song=(Box){100,450,824,64};}if(style==12){f.song.x=100;f.song.w=824;f.song.h=54;}if(f.song.y+f.song.h>body_end)f.song.y=body_end-f.song.h;if(f.art.y+f.art.h>body_end){int edge=body_end-f.art.y;f.art.x+=(f.art.w-edge)/2;f.art.w=f.art.h=edge;}bool playing=NxtPlayback_state()==PLAYER_STATE_PLAYING;
 if(chassis&&(chassis_style!=style||chassis_width!=s->w||chassis_height!=s->h)){SDL_FreeSurface(chassis);chassis=NULL;if(gray_cover)SDL_FreeSurface(gray_cover);gray_cover=NULL;}
 rect(s,0,0,1024,768,NxtSkin_color(NXT_BG),0);
 if(style==0){NxtAppearance_background(s);rect(s,43,140,938,405,NxtSkin_color(NXT_PANEL),14);}
 else if(style==29){rect(s,43,136,938,body_end-136,NxtSkin_color(NXT_PANEL),18);rect(s,f.art.x-10,f.art.y-10,f.art.w+20,f.art.h+20,NxtSkin_color(NXT_LINE),12);}
 else {if(!chassis){char path[128];snprintf(path,sizeof(path),"./res/themes/v18/theme-%02d.png",style);SDL_Surface *raw=IMG_Load(path);if(raw){chassis=SDL_CreateRGBSurfaceWithFormat(0,u(s,881),u(s,660),32,SDL_PIXELFORMAT_ARGB8888);if(chassis){if(!NxtSkin_smoothScale(raw,NULL,chassis,NULL))SDL_BlitScaled(raw,NULL,chassis,NULL);SDL_SetSurfaceBlendMode(chassis,SDL_BLENDMODE_NONE);}SDL_FreeSurface(raw);}chassis_style=style;chassis_width=s->w;chassis_height=s->h;}if(chassis)SDL_BlitSurface(chassis,NULL,s,&(SDL_Rect){u(s,72),u(s,17),0,0});}
 SDL_Color ink=NxtSkin_color(NXT_INK),muted=NxtSkin_color(NXT_MUTED),accent=NxtSkin_color(NXT_ACCENT);
 /* Header and reserved footer are opaque even over a custom wallpaper. */
 rect(s,0,0,1024,132,NxtSkin_color(NXT_BG),0);rect(s,0,bilingual?522:550,1024,bilingual?246:218,NxtSkin_color(NXT_BG),0);
 NxtSkin_icon(s,NxtPersonal_avatar(),u(s,43),u(s,25),u(s,42),accent);bounded(s,24,NXT_APP_NAME,(Box){100,24,440,34},ink);bounded(s,18,Nxt_text("Now playing","Đang phát"),(Box){100,63,360,23},muted);NxtSkin_status(s);
 char value[768];snprintf(value,sizeof(value),"%d / %d",scene_total?scene_track:0,scene_total);bounded(s,18,value,(Box){840,96,141,25},accent);bounded(s,17,NxtTheme_name(style),(Box){43,98,460,22},muted);bounded(s,17,NxtPersonal_name(),(Box){528,98,290,22},muted);
 const TrackInfo *t=NxtPlayback_info();SDL_Surface *art=NxtPlayback_art();if(!art&&!NxtSpotify_hasMetadata()&&!NxtSoundCloud_hasMetadata())art=album_art_get();live_art(s,art,f.art,style);
 bounded(s,30,t->title[0]?t->title:Nxt_text("Choose your music","Chọn nhạc của bạn"),(Box){f.song.x,f.song.y,f.song.w,f.song.h*55/100},ink);
 snprintf(value,sizeof(value),"%s%s%s",t->artist[0]?t->artist:Nxt_text("Unknown artist","Chưa rõ nghệ sĩ"),t->album[0]?" · ":"",t->album);bounded(s,21,value,(Box){f.song.x,f.song.y+f.song.h*60/100,f.song.w,f.song.h*35/100},muted);
 int index=Lyrics_isSynced()?Lyrics_currentIndex(NxtPlayback_position()):0;if(index<0)index=0;const char *current=Settings_getBool(&SETTING_LYRICS_ENABLED)?NxtTranslate_display(index):NULL;if(!current)current=NxtSoundCloud_active()?NxtSoundCloud_lyricsStatus():NxtSpotify_active()?NxtSpotify_lyricsStatus():Lyrics_status();
 int lyric_top=bilingual?527:555,lyric_height=bilingual?70:42;
 rect(s,43,lyric_top,938,lyric_height,NxtSkin_color(NXT_PANEL),8);
 if(NxtLyricPreview_active()||(Settings_getBool(&SETTING_LYRICS_ENABLED)&&Lyrics_lineText(index)))NxtSkin_lyricPair(s,index,u(s,58),u(s,lyric_top+3),u(s,908),u(s,lyric_height-6),true);
 else NxtSkin_fitText(s,nxt.lyric_size,current,u(s,58),u(s,lyric_top+3),u(s,908),u(s,lyric_height-6),ink,true);
 /* Spectrum remains in the chassis; a legacy lyrics box is now free for a
  * compact analyzer on themes which did not have a separate spectrum lane. */
 if(!f.spectrum.w){f.spectrum=scene_fit(scenes[style].lyrics);if(f.spectrum.h>48)f.spectrum.h=48;}if(style==0)f.spectrum=(Box){500,410,410,58};if(f.spectrum.y+f.spectrum.h>body_end)f.spectrum.h=body_end-f.spectrum.y;if(f.spectrum.h<0)f.spectrum.h=0;
 Spectrum_setPosition(u(s,f.spectrum.x),u(s,f.spectrum.y),u(s,f.spectrum.w),u(s,f.spectrum.h));
 int pos=NxtPlayback_position()/1000,dur=NxtPlayback_duration()/1000;if(pos<0)pos=0;if(dur>0&&pos>dur)pos=dur;Box b={43,606,938,3};rect(s,b.x,b.y,b.w,3,muted,1);if(dur>0)rect(s,b.x,b.y,(int)((long long)b.w*pos/dur),3,accent,1);
 snprintf(value,sizeof(value),"%02d:%02d",pos/60,pos%60);bounded(s,15,value,(Box){43,612,95,18},muted);snprintf(value,sizeof(value),dur>0?"%02d:%02d":"--:--",dur/60,dur%60);bounded(s,15,value,(Box){902,612,79,18},muted);
 const char *icons[]={"skip-back",playing?"pause":"play","skip-forward","list-music","captions","sliders-horizontal","ellipsis"};int acts[]={NC_PREV,NC_PLAY,NC_NEXT,NC_QUEUE,NC_LYRICS,NC_EQ,NC_QUICK};for(int i=0;i<7;i++)scene_button(s,i,0,icons[i],action_key(acts[i]),i==1,false);
 int rem=Nxt_timerRemaining();const char *icons2[]={"shuffle",repeat==1?"repeat-1":"repeat","timer","volume-2","heart","audio-lines","arrow-left"};const char *keys2[]={"","","","SELECT ↑↓",action_key(NC_FAVORITE),action_key(NC_SPECTRUM),action_key(NC_MENU)};bool active[]={shuffle,repeat!=0,rem>0,false,NxtPlayback_favorite(),Spectrum_isEnabled(),false};for(int i=0;i<7;i++)scene_button(s,i,1,icons2[i],keys2[i],active[i],i<3);
 char quality[192];quality_value(quality,sizeof(quality),t);snprintf(value,sizeof(value),"%s%s%s",Nxt_text("Source","Nguồn"),quality[0]?" · ":"",quality);bounded(s,15,value,(Box){43,730,938,16},muted);
 snprintf(value,sizeof(value),"%s · %s%s%s",Nxt_text("Output","Đầu ra"),NxtHardware_output(),NxtHardware_pcm()[0]?" · ":"",NxtHardware_pcm());bounded(s,15,value,(Box){43,750,938,16},muted);
 if(style==1||style==4){float p=phase(playing);int cy=style==1?366:292,r=style==1?40:91;for(int j=0;j<2;j++){int cx=style==1?(j?797:528):(j?798:532);for(int k=0;k<3;k++){float q=(j?-p:p)+k*2*M_PI/3;line(s,scene_x(cx),scene_y(cy),scene_x(cx+cosf(q)*r*.6f),scene_y(cy+sinf(q)*r*.6f),muted,3);}}}
 if(style==5||style==32){float level=NxtDSP_peak();if(level<.001f)level=.001f;float a=-2.7f+(fmaxf(-60,20*log10f(level))+60)/60*1.7f;for(int j=0;j<2;j++){int cx=style==5?(j?805:540):(j?736:288),cy=style==5?469:296;line(s,scene_x(cx),scene_y(cy),scene_x(cx+cosf(a)*(style==5?98:172)),scene_y(cy+sinf(a)*95),rgb(0x795638),2);}}
 return true;
}
void NxtTheme_release(void){NxtAuralis_release();if(chassis)SDL_FreeSurface(chassis);if(gray_cover)SDL_FreeSurface(gray_cover);chassis=gray_cover=gray_source=NULL;chassis_style=-1;header_palette=false;at=0;advancing=false;}
