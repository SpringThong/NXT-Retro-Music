/* SPDX-License-Identifier: GPL-3.0-only
 * Native, transactional appearance preview. Drafts touch rendering fields only
 * during paint; audio controls/background saves always see committed settings.
 * Demo cues never enter Lyrics/Translate, the audio engine, network or SD cache. */
#include "nxt_lyric_preview.h"
#include "nxt_lyric_words.h"
#include "nxt_skin.h"
#include "nxt_auralis.h"
#include "nxt_playback.h"
#include "nxt_translate.h"
#include "nxt_theme.h"
#include "nxt_text.h"
#include "nxt_motion.h"
#include "module_common.h"
#include "display_helper.h"
#include "background.h"
#include "api.h"
#include "ui_fonts.h"
#include "toast.h"
#include <stdio.h>
#include <string.h>
static bool preview,hold_words;
static Uint32 epoch;
static LyricLine demo[4];
bool NxtLyricPreview_active(void){return preview;}
int NxtLyricPreview_position(void){return hold_words?1600:(int)NxtLyricPreview_age();}
Uint32 NxtLyricPreview_age(void){return (Uint32)(SDL_GetTicks()-epoch)%3400u;}
const char *NxtLyricPreview_line(int index){return preview?demo[(unsigned)index%4].text:Lyrics_lineText(index);}
const char *NxtLyricPreview_translation(int index){
 if(!preview)return NxtTranslate_line(index);
 static const char*en[]={"A quiet moment, just for you.","Let the melody carry you home.","Stay a little longer with this song.","The night is quiet, the music stays."};
 static const char*vi[]={"Một khoảnh khắc bình yên dành cho bạn.","Để giai điệu đưa bạn trở về.","Nán lại thêm một chút cùng bài hát này.","Đêm lặng yên, tiếng nhạc vẫn ở đây."};
 return (nxt.language==1?en:vi)[(unsigned)index%4];
}
int NxtLyricPreview_wordCount(int index){return preview?demo[(unsigned)index%4].word_count:Lyrics_wordCount(index);}
const LyricWord *NxtLyricPreview_wordAt(int index,int word){return preview?(word>=0&&word<demo[(unsigned)index%4].word_count?&demo[(unsigned)index%4].words[word]:NULL):Lyrics_wordAt(index,word);}
static void prepare(void){
 static const char*en[]={"A quiet moment, just for you.","Let the melody carry you home.","Stay a little longer with this song.","The night is quiet, the music stays."};
 static const char*vi[]={"Một khoảnh khắc bình yên dành cho bạn.","Để giai điệu đưa bạn trở về.","Nán lại thêm một chút cùng bài hát này.","Đêm lặng yên, tiếng nhạc vẫn ở đây."};
 for(int i=0;i<4;i++){memset(&demo[i],0,sizeof(demo[i]));demo[i].time_ms=400;snprintf(demo[i].text,sizeof(demo[i].text),"%s",(nxt.language==1?vi:en)[i]);NxtWords_estimate(&demo[i],2600);}
}
#define LOOK_FIELDS(X) X(font_index) X(auralis_reference_font) X(lyric_size) X(lyric_effect) X(animations) X(lyric_rgb) X(lyric_unsung_rgb) X(lyric_translation_rgb) X(lyric_bold) X(lyric_outline) X(lyric_outline_rgb) X(lyric_shadow) X(song_title_rgb) X(song_artist_rgb) X(song_title_size) X(karaoke_estimate) X(auralis_now_brightness) X(auralis_full_brightness)
typedef struct {
#define DECLARE(n) int n;
 LOOK_FIELDS(DECLARE)
#undef DECLARE
 char font_path[512];
} Look;
static Look capture(void){Look d={0};
#define TAKE(n) d.n=nxt.n;
 LOOK_FIELDS(TAKE)
#undef TAKE
 snprintf(d.font_path,sizeof(d.font_path),"%s",nxt.font_path);return d;}
static void apply(const Look*d){
#define PUT(n) nxt.n=d->n;
 LOOK_FIELDS(PUT)
#undef PUT
 snprintf(nxt.font_path,sizeof(nxt.font_path),"%s",d->font_path);}
static int u(SDL_Surface*s,int n){return n*s->w/1024;}
void NxtLyricPreview_row(SDL_Surface*s,const char*text,int owner,bool translated,bool current,int x,int y,int w,int h,Uint8 alpha){
 if(translated)NxtSkin_styledText(s,nxt.lyric_size,text,x,y,w,h,1,alpha,false);
 else if(current&&(preview||Lyrics_canFollow())&&NxtLyricPreview_wordCount(owner))NxtSkin_karaokeText(s,nxt.lyric_size,owner,x,y,w,h,NxtSkin_lyricColor(4,alpha),NxtSkin_lyricColor(0,alpha),false);
 else NxtSkin_styledText(s,nxt.lyric_size,text,x,y,w,h,current?0:4,alpha,false);
}
static void full(SDL_Surface*s){
 if(nxt.interface_style==NXT_THEME_AURALIS){NxtAuralis_lyrics(s,1,true);return;}
 NxtSkin_begin(s,Nxt_text("Lyrics · preview","Lời bài hát · xem trước"),NULL,false);NxtMotion_setMenu(false);
 int top=NxtSkin_top(s),pad=s->w/40;TTF_Font*f=NxtSkin_font(s,nxt.lyric_size);int rh=(f?TTF_FontHeight(f):u(s,38))+8;
 SDL_Rect old;SDL_GetClipRect(s,&old);SDL_Rect clip={pad,top,s->w-pad*2,u(s,620)-top};SDL_SetClipRect(s,&clip);
 float t=NxtLyricFX_progress(NxtLyricPreview_age());int shift=NxtLyricFX_slides()?(int)((1.f-t)*rh*2):0;
 for(int row=0;row<8;row++){int owner=row/2;bool translated=row&1;int dx=0,dy=0;Uint8 alpha=255;if(owner==1)NxtLyricFX_transform(t,s->w,&dx,&dy,&alpha);int y=top+row*rh+shift+dy;
  NxtLyricPreview_row(s,translated?NxtLyricPreview_translation(owner):NxtLyricPreview_line(owner),owner,translated,owner==1,pad+dx,y,s->w-pad*2,rh-8,alpha);
 }SDL_SetClipRect(s,&old);
}
static int *color(Look*d,int field){switch(field){case NXT_EDIT_SUNG:return &d->lyric_rgb;case NXT_EDIT_UNSUNG:return &d->lyric_unsung_rgb;case NXT_EDIT_TRANSLATION:return &d->lyric_translation_rgb;case NXT_EDIT_TITLE:return &d->song_title_rgb;case NXT_EDIT_ARTIST:return &d->song_artist_rgb;case NXT_EDIT_OUTLINE_COLOR:return &d->lyric_outline_rgb;default:return NULL;}}
static const char *name(int field){
 static const char*en[]={"Sung lyric color","Upcoming lyric color","Translation color · italic","Song title color","Artist color","Bold lyrics","Outline width","Outline color","Text shadow","Song title size","Lyric size","Lyric transition","Estimated karaoke offline","Interface animation","Now Playing cover brightness","Full lyrics cover brightness","Reset text style"};
 static const char*vi[]={"Màu lời đang hát","Màu lời chưa hát","Màu bản dịch · chữ nghiêng","Màu tên bài hát","Màu ca sĩ","Lời in đậm","Độ dày viền","Màu viền","Bóng chữ","Cỡ tên bài hát","Cỡ chữ lời","Hiệu ứng chuyển lời","Karaoke ước lượng offline","Chuyển động giao diện","Độ sáng bìa Đang phát","Độ sáng bìa Full lời","Đặt lại kiểu chữ"};
 return field<0?Nxt_text("Font family","Bộ font"):(nxt.language==1?vi:en)[field];
}
static int wrap(int n,int low,int high){int span=high-low+1;return low+((n-low)%span+span)%span;}
static void reset(Look*d){d->lyric_rgb=0xffd166;d->lyric_unsung_rgb=0xeeeeee;d->lyric_translation_rgb=0xbde3e5;d->song_title_rgb=0xffffff;d->song_artist_rgb=0xc8e4e6;d->lyric_bold=0;d->lyric_outline=2;d->lyric_outline_rgb=0x101010;d->lyric_shadow=1;d->song_title_size=50;d->lyric_size=34;d->lyric_effect=1;}
static int step(int n,int low,int high){return n<low?high:n>high?low:n;}
static void adjust(Look*d,int field,int delta){
 int*c=color(d,field);if(c){static const int palette[]={0xffffff,0xffd166,0xbde3e5,0xffa6c1,0xc2b4ff,0xa9df9b,0xeeeeee,0x101010};int at=0;while(at<8&&palette[at]!=*c)at++;*c=palette[wrap(at+delta,0,7)];return;}
 switch(field){case NXT_EDIT_FONT:d->font_index=wrap(d->font_index+delta,0,NXT_FONT_COUNT-1);d->font_path[0]=0;d->auralis_reference_font=0;break;
 case NXT_EDIT_BOLD:d->lyric_bold=!d->lyric_bold;break;case NXT_EDIT_OUTLINE:d->lyric_outline=wrap(d->lyric_outline+delta,0,3);break;
 case NXT_EDIT_SHADOW:d->lyric_shadow=!d->lyric_shadow;break;case NXT_EDIT_TITLE_SIZE:d->song_title_size=step(d->song_title_size+delta*2,38,60);break;
 case NXT_EDIT_LYRIC_SIZE:d->lyric_size=step(d->lyric_size+delta*2,24,48);break;case NXT_EDIT_EFFECT:d->lyric_effect=wrap(d->lyric_effect+delta,0,9);break;
 case NXT_EDIT_ESTIMATE:d->karaoke_estimate=!d->karaoke_estimate;break;case NXT_EDIT_ANIMATIONS:d->animations=!d->animations;break;
 case NXT_EDIT_NOW_BRIGHTNESS:d->auralis_now_brightness=wrap(d->auralis_now_brightness+delta,40,95);break;case NXT_EDIT_FULL_BRIGHTNESS:d->auralis_full_brightness=wrap(d->auralis_full_brightness+delta,30,90);break;case NXT_EDIT_RESET:reset(d);break;}
}
static void value(Look*d,int field,char*out,size_t size){
 int*c=color(d,field);if(c){snprintf(out,size,"#%06X",*c);return;}
 if(field==NXT_EDIT_FONT){snprintf(out,size,"%d/%d · %s",d->font_index+1,NXT_FONT_COUNT,NxtText_fontName(d->font_index));return;}
 if(field==NXT_EDIT_EFFECT){snprintf(out,size,"%s",NxtLyricFX_name(d->lyric_effect));return;}
 int n=field==NXT_EDIT_BOLD?d->lyric_bold:field==NXT_EDIT_OUTLINE?d->lyric_outline:field==NXT_EDIT_SHADOW?d->lyric_shadow:field==NXT_EDIT_TITLE_SIZE?d->song_title_size:field==NXT_EDIT_LYRIC_SIZE?d->lyric_size:field==NXT_EDIT_ESTIMATE?d->karaoke_estimate:field==NXT_EDIT_ANIMATIONS?d->animations:field==NXT_EDIT_NOW_BRIGHTNESS?d->auralis_now_brightness:d->auralis_full_brightness;
 if(field==NXT_EDIT_BOLD||field==NXT_EDIT_SHADOW||field==NXT_EDIT_ESTIMATE||field==NXT_EDIT_ANIMATIONS)snprintf(out,size,"%s",n?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));else if(field==NXT_EDIT_RESET)snprintf(out,size,"%s",Nxt_text("←→ Reset draft","←→ Đặt lại bản nháp"));else snprintf(out,size,"%d",n);
}
static void paint(SDL_Surface*s,Look*d,int field,bool is_full,int rgb_component){
 Look committed=capture();hold_words=field==NXT_EDIT_FONT||color(d,field)!=NULL;apply(d);
 if(is_full)full(s);else NxtSkin_playing(s,0,0,nxt.shuffle,nxt.repeat_mode);
 NxtMotion_setMenu(false);SDL_Color panel={20,24,29,244},ink={240,242,244,255},muted={188,206,209,255};
 NxtSkin_rect(s,(SDL_Rect){u(s,20),u(s,632),u(s,984),u(s,120)},panel,u(s,16));
 char v[240],title[400];value(d,field,v,sizeof(v));snprintf(title,sizeof(title),"%s · %s",name(field),v);
 NxtSkin_fitText(s,22,title,u(s,40),u(s,642),u(s,944),u(s,32),ink,false);
 if(rgb_component>=0){int*c=color(d,field);snprintf(v,sizeof(v),"%c  R %d  G %d  B %d · %s","RGB"[rgb_component],(*c>>16)&255,(*c>>8)&255,*c&255,Nxt_text("↑↓ Channel · ←→ ±1 · L1/R1 ±16 · Y Done","↑↓ Kênh · ←→ ±1 · L1/R1 ±16 · Y Xong"));}
 else snprintf(v,sizeof(v),"%s",field<0?Nxt_text("↑↓/←→ Font · L1/R1 ±10 · X Now Playing / Full lyrics","↑↓/←→ Font · L1/R1 ±10 · X Đang phát / Full lời"):Nxt_text("↑↓ Setting · ←→ Change · Y RGB · X Now Playing / Full lyrics","↑↓ Mục · ←→ Đổi · Y RGB · X Đang phát / Full lời"));
 NxtSkin_fitText(s,14,v,u(s,40),u(s,684),u(s,944),u(s,23),muted,false);
 snprintf(v,sizeof(v),"%s · %s · %s",is_full?Nxt_text("FULL LYRICS","FULL LỜI"):Nxt_text("NOW PLAYING","ĐANG PHÁT"),Nxt_text("SAMPLE LYRICS · SELECT Replay","LỜI MẪU · SELECT Chạy lại"),Nxt_text("A Apply · B Cancel","A Áp dụng · B Hủy"));
 NxtSkin_fitText(s,14,v,u(s,40),u(s,718),u(s,944),u(s,22),ink,false);apply(&committed);
}
ModuleExitReason NxtLyricPreview_run(DisplayContext*d,int first){
 Look draft=capture();bool font=first==NXT_EDIT_FONT,is_full=false;int field=font?-1:wrap(first,0,NXT_EDIT_RESET),rgb=-1,show=0;Uint32 drawn=0;bool dirty=true;NxtMotion_release();prepare();preview=true;epoch=SDL_GetTicks();
 if(font){draft.auralis_reference_font=0;draft.font_path[0]=0;}
 for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface*s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);
  if(g.should_quit||ModuleCommon_stopSignalled()){preview=false;NxtMotion_release();NxtMotion_setMenu(true);return MODULE_EXIT_QUIT;}
  if(!g.input_consumed&&!Nxt_lockInput()){
   if(NxtPad_justPressed(BTN_B)){preview=false;NxtMotion_release();NxtMotion_setMenu(true);return MODULE_EXIT_TO_MENU;}
   if(NxtPad_justPressed(BTN_A)){Look committed=capture();apply(&draft);if(!Nxt_save()){apply(&committed);Toast_show(Nxt_text("Could not save · draft kept","Chưa lưu được · giữ bản nháp"),2000);dirty=true;}else{preview=false;NxtMotion_release();NxtMotion_setMenu(true);if(draft.font_index!=committed.font_index||strcmp(draft.font_path,committed.font_path)||draft.auralis_reference_font!=committed.auralis_reference_font){Fonts_unload();Fonts_load();}return MODULE_EXIT_TO_MENU;}}
   if(NxtPad_justPressed(BTN_X)){is_full=!is_full;dirty=true;epoch=SDL_GetTicks();}
   if(NxtPad_justPressed(BTN_Y)&&color(&draft,field)){rgb=rgb<0?0:-1;dirty=true;}
   if(NxtPad_justPressed(BTN_SELECT)){epoch=SDL_GetTicks();dirty=true;Nxt_selectConsume();}
   int vertical=!!NxtPad_justRepeated(BTN_DOWN)-!!NxtPad_justRepeated(BTN_UP),horizontal=!!NxtPad_justRepeated(BTN_RIGHT)-!!NxtPad_justRepeated(BTN_LEFT),jump=!!NxtPad_justRepeated(BTN_R1)-!!NxtPad_justRepeated(BTN_L1);
   if(rgb>=0){if(vertical)rgb=wrap(rgb+vertical,0,2);int*c=color(&draft,field),delta=horizontal+jump*16;if(delta){int shift=(2-rgb)*8,n=((*c>>shift)&255)+delta;if(n<0)n=0;if(n>255)n=255;*c=(*c&~(255<<shift))|(n<<shift);}}
   else if(font){if(vertical||horizontal||jump)adjust(&draft,field,vertical+horizontal+jump*10);}
   else{if(vertical){field=wrap(field+vertical,0,NXT_EDIT_RESET);if(nxt.interface_style!=NXT_THEME_AURALIS){while(field==NXT_EDIT_TITLE||field==NXT_EDIT_ARTIST||field==NXT_EDIT_TITLE_SIZE||field==NXT_EDIT_NOW_BRIGHTNESS||field==NXT_EDIT_FULL_BRIGHTNESS)field=wrap(field+vertical,0,NXT_EDIT_RESET);}}if(horizontal)adjust(&draft,field,horizontal);}
   if(vertical||horizontal||jump){dirty=true;epoch=SDL_GetTicks();}
  }
  Uint32 now=SDL_GetTicks(),age=NxtLyricPreview_age();Uint32 interval=draft.animations&&((draft.lyric_effect&&age<360)||(!hold_words&&age>=400&&age<2600))?16:250;
  if(!PWR_getDisplayOff()&&!NxtSession_isHidden()&&(dirty||g.dirty||!drawn||(Uint32)(now-drawn)>=interval)){paint(s,&draft,field,is_full,rgb);ModuleCommon_markSurfaceDrawn();drawn=now;dirty=false;}
  ModuleCommon_frameEnd(s);
 }
}
