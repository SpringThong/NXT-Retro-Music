#include "nxt_lyric_preview.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * Auralis Black Glass: cached native-resolution full cover, paired bilingual
 * rows and real Enhanced-LRC word timings. No decoder/network work per frame. */
#include "nxt_auralis.h"
#include "nxt_skin.h"
#include "nxt_playback.h"
#include "nxt_translate.h"
#include "nxt_personal.h"
#include "nxt_volume.h"
#include "nxt_motion.h"
#include "lyrics.h"
#include "spectrum.h"
#include "settings.h"
#include "album_art.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
/* All effects reuse cached glyph surfaces. No blur, full-screen blend or
 * rescaling on animation frames. Off still retains word-timestamp karaoke. */
const char *NxtLyricFX_name(int effect){
 static const char *en[]={"Off","Glide up","Fade","Glide + fade","Slide from right","Soft rise","Slide from left","Drop in","Diagonal glide","Soft settle"};
 static const char *vi[]={"Tắt","Trượt lên","Mờ nhẹ","Trượt + mờ","Lướt từ phải","Nổi nhẹ","Lướt từ trái","Hạ nhẹ","Lướt chéo","Nhún nhẹ"};
 if(effect<0||effect>9)effect=1;return Nxt_text(en[effect],vi[effect]);
}
ModuleExitReason NxtLyricFX_settings(DisplayContext *d){return NxtLyricPreview_run(d,NXT_EDIT_EFFECT);}
float NxtLyricFX_progress(Uint32 age){if(!nxt.animations||nxt.lyric_effect==0||age>=360)return 1.f;float t=age/360.f;return 1.f-(1.f-t)*(1.f-t)*(1.f-t);}
bool NxtLyricFX_slides(void){return nxt.animations&&(nxt.lyric_effect==1||nxt.lyric_effect==3);}
void NxtLyricFX_transform(float t,int scale,int *dx,int *dy,Uint8 *alpha){
 if(t<0)t=0;if(t>1)t=1;*dx=*dy=0;*alpha=255;
 if(!nxt.animations||!nxt.lyric_effect)return;
 if(nxt.lyric_effect>=2)*alpha=(Uint8)(112+143*t);
 if(nxt.lyric_effect==4)*dx=(int)((1.f-t)*28*scale/1024.f);
 if(nxt.lyric_effect==6)*dx=-(int)((1.f-t)*28*scale/1024.f);
 if(nxt.lyric_effect==7)*dy=-(int)((1.f-t)*18*scale/1024.f);
 if(nxt.lyric_effect==8){*dx=(int)((1.f-t)*22*scale/1024.f);*dy=(int)((1.f-t)*14*scale/1024.f);}
 if(nxt.lyric_effect==9)*dy=-(int)(sinf(t*3.14159265f)*(1.f-t)*18*scale/1024.f);
 if(nxt.lyric_effect==5)*dy=(int)((1.f-t)*18*scale/1024.f);
}
static SDL_Surface *backdrop,*art_source;
static unsigned art_generation,cue_generation,lyric_generation;
static int backdrop_full=-1,backdrop_brightness=-1,cue=-1;
static Uint32 cue_started;
static bool cue_full;
static int u(SDL_Surface *s,int n){return (n*s->w+512)/1024;}
static SDL_Color rgb(unsigned c){return(SDL_Color){c>>16,(c>>8)&255,c&255,255};}
static Uint8 text_alpha=255;static int lyric_dx;
static void text(SDL_Surface *s,int size,const char *v,int x,int y,int w,int h,unsigned c,bool italic,bool center){
 SDL_Color tint=rgb(c);tint.a=text_alpha;
 if(italic)NxtSkin_fitItalicText(s,size,v,u(s,x),u(s,y),u(s,w),u(s,h),tint,center);
 else NxtSkin_fitText(s,size,v,u(s,x),u(s,y),u(s,w),u(s,h),tint,center);
}
static void rect(SDL_Surface*s,int x,int y,int w,int h,unsigned c,int radius){NxtSkin_rect(s,(SDL_Rect){u(s,x),u(s,y),u(s,w),u(s,h)},rgb(c),u(s,radius));}
static void background(SDL_Surface *s,bool full){
 SDL_Surface *art=NxtPlayback_art();if(!art&&!NxtSpotify_hasMetadata()&&!NxtSoundCloud_hasMetadata())art=album_art_get();unsigned gen=NxtPlayback_generation();
 int brightness=full?nxt.auralis_full_brightness:nxt.auralis_now_brightness;if(brightness<(full?30:40)||brightness>(full?90:95))brightness=full?60:72;
 if(backdrop&&(art_source!=art||art_generation!=gen||backdrop->w!=s->w||backdrop->h!=s->h||backdrop_full!=full||backdrop_brightness!=brightness)){SDL_FreeSurface(backdrop);backdrop=NULL;}
 if(!backdrop){
  backdrop=SDL_CreateRGBSurfaceWithFormat(0,s->w,s->h,32,SDL_PIXELFORMAT_ARGB8888);
  if(backdrop){
   SDL_FillRect(backdrop,NULL,SDL_MapRGB(backdrop->format,36,41,47));
   if(art&&art->w>0&&art->h>0){SDL_Rect crop={0,0,art->w,art->h};if((long long)art->w*s->h>(long long)art->h*s->w){crop.w=(long long)art->h*s->w/s->h;crop.x=(art->w-crop.w)/2;}else{crop.h=(long long)art->w*s->h/s->w;crop.y=(art->h-crop.h)*18/100;}if(!NxtSkin_smoothScale(art,&crop,backdrop,NULL))SDL_BlitScaled(art,&crop,backdrop,NULL);}
   SDL_LockSurface(backdrop);
   for(int y=0;y<backdrop->h;y++){Uint32 *row=(Uint32*)((Uint8*)backdrop->pixels+y*backdrop->pitch);float fy=(float)y/backdrop->h;
    float light=brightness/100.f;if(fy<.22f)light*=.62f+.38f*fy/.22f;if(fy>.60f)light*=1.f-(fy-.60f)*1.25f;
    for(int x=0;x<backdrop->w;x++){Uint8 r,g,b,a;SDL_GetRGBA(row[x],backdrop->format,&r,&g,&b,&a);row[x]=SDL_MapRGB(backdrop->format,r*light,g*light,b*light);}}
   SDL_UnlockSurface(backdrop);SDL_SetSurfaceBlendMode(backdrop,SDL_BLENDMODE_NONE);
   art_source=art;art_generation=gen;backdrop_full=full;backdrop_brightness=brightness;
  }
 }
 if(backdrop)SDL_BlitSurface(backdrop,NULL,s,NULL);else SDL_FillRect(s,NULL,SDL_MapRGB(s->format,36,41,47));
}
static float transition(int index,bool full){
 if(NxtLyricPreview_active())return NxtLyricFX_progress(NxtLyricPreview_age());
 unsigned gen=NxtPlayback_generation(),rev=Lyrics_revision();Uint32 now=SDL_GetTicks();
 if(cue!=index||cue_generation!=gen||cue_full!=full||lyric_generation!=rev){bool adjacent=cue>=0&&index==cue+1&&cue_generation==gen&&cue_full==full&&lyric_generation==rev;cue_started=adjacent?now:now-360;cue=index;cue_generation=gen;lyric_generation=rev;cue_full=full;}
 return NxtLyricFX_progress((Uint32)(now-cue_started));
}
static void pair(SDL_Surface *s,int index,int y,int h,bool active,bool full,Uint8 alpha){
 const char *original=NxtLyricPreview_line(index);if(!original)return;
 text_alpha=alpha;
 int mode=NxtLyricPreview_active()?2:nxt.translation_mode;const char *translated=mode?NxtLyricPreview_translation(index):NULL;
 int size=full?nxt.lyric_size+8:nxt.lyric_size+4;if(size<30)size=30;if(size>48)size=48;
 int x=(full?58:44)+lyric_dx,w=full?908:936,row=mode==2?(h-8)/2:h;
 unsigned ink=active?0xdadada:0x989ca2,sub=active?0xc9c9c9:0x858a91;
 if(mode==1&&translated&&*translated)NxtSkin_styledText(s,size,translated,u(s,x),u(s,y),u(s,w),u(s,h),1,alpha,full);
 else{
  if(active&&NxtLyricPreview_wordCount(index))NxtSkin_karaokeText(s,size,index,u(s,x),u(s,y),u(s,w),u(s,row),(SDL_Color){145,148,152,alpha},(SDL_Color){218,218,218,alpha},full);
  else NxtSkin_styledText(s,size,original,u(s,x),u(s,y),u(s,w),u(s,row),active?0:4,alpha,full);
  if(mode==2)NxtSkin_styledText(s,size-8,translated&&*translated?translated:Nxt_text("Translating…","Đang dịch…"),u(s,x),u(s,y+row+8),u(s,w),u(s,row),1,alpha,full);
 }
 text_alpha=255;
}
static void lyrics(SDL_Surface *s,int focus,bool follow,bool full){
 int mode=NxtLyricPreview_active()?2:nxt.translation_mode;int count=NxtLyricPreview_active()?4:Lyrics_lineCount(),current=NxtLyricPreview_active()?1:Lyrics_currentIndex(NxtPlayback_position());bool synced=NxtLyricPreview_active()||Lyrics_canFollow();int index=follow&&synced?current:focus;if(index<0)index=0;if(index>=count&&count>0)index=count-1;
 int top=full?(NxtLyricPreview_active()?140:180):245,bottom=full?668:526;SDL_Rect old;SDL_GetClipRect(s,&old);SDL_Rect clip={u(s,34),u(s,top),u(s,956),u(s,bottom-top)};SDL_SetClipRect(s,&clip);
 if(!count||(!full&&!NxtLyricPreview_active()&&!Settings_getBool(&SETTING_LYRICS_ENABLED)))text(s,25,!count?Lyrics_status():Nxt_text("Lyrics are off","Lời đang tắt"),44,top+54,936,56,0xc7c7c0,false,full);
 else{
  float t=transition(index,full);int step=full?164:142,height=mode==2?(full?112:94):66,anchor=full?328:290;float movement=NxtLyricFX_slides()?(1.f-t)*step:0;
  for(int i=index-2;i<=index+2;i++){if(i<0||i>=count||(NxtLyricPreview_active()&&!full&&i<index))continue;int y=anchor+(i-index)*step+(int)movement;if(y+height<top||y>bottom)continue;
   if(i==index||i==index+1){const char *label=i==index?(NxtLyricPreview_active()?Nxt_text("SAMPLE · PREVIEW","LỜI MẪU · XEM TRƯỚC"):!follow?Nxt_text("SELECTED LINE","CÂU ĐANG XEM"):synced&&current<0?Nxt_text("WAITING FOR FIRST LINE","CHỜ CÂU ĐẦU"):synced?Lyrics_wordEstimated(i)?Nxt_text("SINGING · ESTIMATED","ĐANG HÁT · ƯỚC LƯỢNG"):Nxt_text("SINGING NOW","ĐANG HÁT"):Nxt_text("LYRICS · UNSYNCED","LỜI · KHÔNG ĐỒNG BỘ")):Nxt_text("UP NEXT","TIẾP THEO");text(s,12,label,full?58:44,y-24,full?908:936,18,i==index?0xdadada:0xa5a59d,false,full);}
   int dx=0,dy=0;Uint8 alpha=255;if(i==index)NxtLyricFX_transform(t,1024,&dx,&dy,&alpha);
   /* Pair X motion is applied to the destination surface viewport via the
    * pair origin, not a second temporary full-screen image. */
   lyric_dx=dx;pair(s,i,y+dy,height,i==current&&synced,full,alpha);lyric_dx=0;
  }
 }
 SDL_SetClipRect(s,&old);
}
static void button(SDL_Surface*s,int x,int y,int w,int h,const char *icon,bool active,bool play){
 int edge=u(s,w),height=u(s,h),cx=x+w/2,cy=y+h/2;
 if(y==650||y==643||y==656)cy=674;
 /* Mirrored logical centres are rounded as offsets from the physical centre. */
 int center=s->w/2+(cx>=512?u(s,cx-512):-u(s,512-cx));
 SDL_Rect r={center-edge/2,u(s,cy)-height/2,edge,height};
 SDL_Color fill=(play||active)?rgb(0xe5e9e6):(SDL_Color){20,24,28,155};int border=u(s,1);if(border<1)border=1;
 NxtSkin_rect(s,(SDL_Rect){r.x-border,r.y-border,r.w+2*border,r.h+2*border},(SDL_Color){240,243,239,(play||active)?180:75},r.h/2+border);
 NxtSkin_rect(s,r,fill,r.h/2);int pixels=u(s,play?34:h>48?30:h>40?28:25);
 NxtSkin_icon(s,icon,r.x+(r.w-pixels)/2,r.y+(r.h-pixels)/2,pixels,rgb((play||active)?0x111815:0xf1f1eb));

}
bool NxtAuralis_playing(SDL_Surface *s,int track,int total,bool shuffle,int repeat){
 background(s,false);NxtSkin_status(s);bool playing=NxtPlayback_state()==PLAYER_STATE_PLAYING;
 NxtSkin_icon(s,NxtPersonal_avatar(),u(s,44),u(s,24),u(s,31),rgb(0xdadada));text(s,20,NXT_APP_NAME,88,25,440,29,0xf1f1ed,false,false);
 char value[768];snprintf(value,sizeof(value),"AURALIS   ·   %s",NxtPersonal_name());text(s,13,value,44,81,740,22,0xb9b9b0,false,false);snprintf(value,sizeof(value),"%d / %d",total?track:0,total);text(s,16,value,820,81,160,22,0xdadada,false,true);
 const TrackInfo *t=NxtPlayback_info();NxtSkin_songTitle(s,nxt.song_title_size,t->title[0]?t->title:Nxt_text("Choose your music","Chọn nhạc của bạn"),u(s,44),u(s,108),u(s,936),u(s,88),false);
 snprintf(value,sizeof(value),"%s%s%s",t->artist,t->album[0]?" · ":"",t->album);NxtSkin_styledText(s,24,value,u(s,44),u(s,203),u(s,936),u(s,34),3,255,false);
 lyrics(s,0,true,false);Spectrum_setPosition(u(s,44),u(s,535),u(s,936),u(s,18));
 snprintf(value,sizeof(value),"%s · %.1f kHz · %d bit · %d kbps · %d ch",t->codec,t->sample_rate/1000.f,t->bit_depth,t->bitrate/1000,t->channels);NxtSkin_styledText(s,13,value,u(s,44),u(s,562),u(s,936),u(s,24),3,255,false);
 snprintf(value,sizeof(value),"%s · %s",NxtHardware_output(),NxtHardware_pcm());NxtSkin_styledText(s,13,value,u(s,44),u(s,585),u(s,936),u(s,24),3,255,false);
 int pos=NxtPlayback_position()/1000,dur=NxtPlayback_duration()/1000;if(pos<0)pos=0;if(dur>0&&pos>dur)pos=dur;rect(s,44,620,936,3,0x77776d,1);if(dur>0)rect(s,44,620,(long long)936*pos/dur,3,0xdadada,1);
 snprintf(value,sizeof(value),"%02d:%02d",pos/60,pos%60);text(s,11,value,44,632,90,15,0xd0d0c5,false,false);snprintf(value,sizeof(value),dur>0?"%02d:%02d":"--:--",dur/60,dur%60);text(s,11,value,900,632,80,15,0xd0d0c5,false,false);
 button(s,380,650,56,56,"skip-back",false,false);button(s,477,643,70,70,playing?"pause":"play",true,true);button(s,588,650,56,56,"skip-forward",false,false);
 const char *icons[]={"list-music","captions","sliders-horizontal","ellipsis"};const int auxiliary_x[]={221,279,703,761};for(int i=0;i<4;i++)button(s,auxiliary_x[i],656,42,42,icons[i],false,false);
 button(s,41,711,38,38,"arrow-left",false,false);button(s,96,711,38,38,"heart",NxtPlayback_favorite(),false);button(s,151,711,38,38,"audio-lines",Spectrum_isEnabled(),false);button(s,206,711,38,38,"volume-2",false,false);
 snprintf(value,sizeof(value),"%d",NxtVolume_getPercent());text(s,12,value,249,721,48,18,0xd2d2c7,false,false);
 button(s,813,711,38,38,"timer",Nxt_timerRemaining()>0,false);button(s,869,711,38,38,repeat==1?"repeat-1":"repeat",repeat!=0,false);button(s,925,711,38,38,"shuffle",shuffle,false);return true;
}
void NxtAuralis_lyrics(SDL_Surface *s,int focus,bool follow){
 NxtMotion_page(s,"@full-lyrics",false);background(s,true);NxtSkin_status(s);text(s,16,Nxt_text("AURALIS · LYRICS","AURALIS · LỜI BÀI HÁT"),44,28,500,25,0xdadada,false,false);
 NxtSkin_styledText(s,nxt.song_title_size*36/50,NxtPlayback_info()->title,u(s,44),u(s,91),u(s,936),u(s,49),2,255,true);lyrics(s,focus,follow,true);
 char info[512];snprintf(info,sizeof(info),"%s · %+d ms · %s",follow?Nxt_text("Follow","Bám lời"):Nxt_text("Manual scroll","Cuộn tay"),nxt.lyric_offset,NxtTranslate_modeName(nxt.translation_mode));text(s,12,info,44,699,936,20,0xbdbdb2,false,true);
 text(s,12,Nxt_text("↑↓ Scroll · Y Follow · X Size · L1/R1 Offset · START Translate · A Pause · B Back","↑↓ Cuộn · Y Bám · X Cỡ · L1/R1 Lệch · START Dịch · A Dừng · B Về"),30,744,964,17,0xaeaea4,false,true);
}
bool NxtAuralis_motion(void){return nxt.animations&&nxt.lyric_effect!=0&&(Uint32)(SDL_GetTicks()-cue_started)<360;}
void NxtAuralis_release(void){if(backdrop)SDL_FreeSurface(backdrop);backdrop=art_source=NULL;backdrop_full=backdrop_brightness=-1;cue=-1;cue_generation=lyric_generation=0;}
