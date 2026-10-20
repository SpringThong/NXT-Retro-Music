#include "nxt_lyric_preview.h"
#include "nxt_translate.h"
#include "nxt_locale.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "nxt_eq.h"
#include "nxt_playback.h"
#include "nxt_skin.h"
#include "nxt_auralis.h"
#include "nxt_motion.h"
#include "nxt_record.h"
#include "nxt_theme.h"
#include "nxt_performance.h"
#include "nxt_controls.h"
#include "api.h"
#include "ui_fonts.h"
#include "ui_theme.h"
#include "ui_layers.h"
#include "player.h"
#include "module_player.h"
#include "module_settings.h"
#include "module_downloader.h"
#include "downloader.h"
#include "wifi.h"
#include "album_art.h"
#include "playlist_m3u.h"
#include "file_utils.h"
#include "keyboard.h"
#include "background.h"
#include "lyrics.h"
#include "spectrum.h"
#include "resume.h"
#include "toast.h"
#include "settings.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
static TTF_Font *body,*small,*heading,*lyric;
static int font_signature;static char font_source[512];
static bool locked;
static bool select_combo;
static uint32_t select_started;
static bool select_held,select_long;
bool Nxt_selectLongUsed(void){return select_long;}
void Nxt_selectConsume(void){select_held=true;select_long=true;}
bool Nxt_lockInput(void){
    if(ModuleCommon_stopSignalled())return false;
    if(Nxt_displayInputBlocked())return locked;
    if(PAD_isPressed(BTN_SELECT)){
        if(!select_held){select_held=true;select_long=false;select_started=SDL_GetTicks();}
        if(!select_long&&SDL_GetTicks()-select_started>=900){select_long=true;locked=!locked;Toast_show(locked?Nxt_text("Controls locked","Đã khóa nút"):Nxt_text("Controls unlocked","Đã mở khóa nút"),1500);}
    }else select_held=false;
    return locked;
}
static void fonts(SDL_Surface *s) {
    int sz=s->w*25/1024*nxt.font_size/100; if(sz<14)sz=14;
    int sig=sz*10000+nxt.lyric_size*100+nxt.font_index;
    if(body&&sig==font_signature&&!strcmp(font_source,nxt.font_path))return;
    snprintf(font_source,sizeof(font_source),"%s",nxt.font_path);
    if(body){NxtSkin_forgetFont(body);TTF_CloseFont(body);}if(small){NxtSkin_forgetFont(small);TTF_CloseFont(small);}if(heading){NxtSkin_forgetFont(heading);TTF_CloseFont(heading);}if(lyric){NxtSkin_forgetFont(lyric);TTF_CloseFont(lyric);}
    body=Fonts_open(sz);small=Fonts_open(sz*3/4);heading=Fonts_open(sz*3/2);lyric=Fonts_open(nxt.lyric_size*s->w/1024);
    font_signature=sig;
}
static SDL_Color color(ThemeRole role) { return Theme_getColor(role,false); }
static void fill(SDL_Surface *s,SDL_Rect r,SDL_Color c) { SDL_FillRect(s,&r,SDL_MapRGB(s->format,c.r,c.g,c.b)); }
void Nxt_drawText(SDL_Surface *s,TTF_Font *f,const char *text,int x,int y,int w,SDL_Color c) {
    if(!f||!text||w<=0)return;SDL_Surface *t=NxtSkin_renderText(f,text);if(!t)return;
    SDL_Rect src={0,0,t->w>w?w:t->w,t->h};NxtSkin_blitText(s,t,src,x,y,c);
}
/* Wrap on UTF-8 character boundaries, preferring spaces. Long words are split, never cropped. */
static const char *next_utf8(const char *p) { return NxtText_nextCluster(p); }
static char **wrap(TTF_Font *f,const char *text,int w,int *count) {
    *count=0;int cap=16;char **rows=malloc(cap*sizeof(char*));if(!rows)return NULL;
    const char *p=text?text:"";
    while(*p) {
        const char *end=p,*space=NULL,*best=p;
        while(*end&&*end!='\n') { const char *after=next_utf8(end);size_t len=after-p;char temp[4096];if(len>=sizeof(temp))break;memcpy(temp,p,len);temp[len]=0;int width=0;TTF_SizeUTF8(f,temp,&width,NULL);if(width>w&&best!=p)break;best=after;if(*end==' ')space=end;end=after;if(width>w)break; }
        if(*end&&*end!='\n'&&space&&space>p)best=space;
        if(best==p)best=*p=='\n'?p:next_utf8(p);
        size_t n=best-p;char *row=malloc(n+1);if(!row)break;memcpy(row,p,n);row[n]=0;
        if(*count==cap){cap*=2;char **newrows=realloc(rows,cap*sizeof(char*));if(!newrows){free(row);break;}rows=newrows;}rows[(*count)++]=row;
        p=best;while(*p==' ')p++;if(*p=='\n')p++;
    }
    return rows;
}
static void free_rows(char **rows,int n) { for(int i=0;i<n;i++)free(rows[i]);free(rows); }
int Nxt_drawWrapped(SDL_Surface *s,TTF_Font *f,const char *text,int x,int y,int w,int h,SDL_Color c) {
    int count=0;char **rows=wrap(f,text,w,&count);int line=TTF_FontHeight(f)+3,used=0;for(int i=0;i<count;i++){int height=0;TTF_SizeUTF8(f,rows[i],NULL,&height);if(height+3>line)line=height+3;}
    for(int i=0;i<count&&used+line<=h;i++){Nxt_drawText(s,f,rows[i],x,y+used,w,c);used+=line;}free_rows(rows,count);return used;
}
static void page(SDL_Surface *s,const char *title,const char *hint) {
    fonts(s);NxtSkin_begin(s,title,NULL,false);NxtSkin_footer(s,hint,false);
}
static int page_top(SDL_Surface *s) { return NxtSkin_top(s); }
static const char **picker_tabs;static int picker_ntabs,picker_tab;
static NxtPick pick_internal(DisplayContext *display,const char *title,const char **items,int count,int *selected,const char *hint,bool live) {
    if(NxtEQ_closeRequested())return (NxtPick){2,*selected};
    int show=0,dirty=1;BackgroundPlayerType previous=Background_getActive();
    uint32_t opened=SDL_GetTicks(),last_redraw=0;
    if(previous==BG_NONE&&PlayerModule_isActive())Background_setActive(BG_MUSIC);
    if(*selected<0)*selected=0;if(*selected>=count)*selected=count>0?count-1:0;
    for(;;) {
        ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(display);
        if(live&&SDL_GetTicks()-opened>=1000){if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);return (NxtPick){7,*selected};}
        if(Nxt_lockInput()){ModuleCommon_frameEnd(s);continue;}
        GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);
        if(g.should_quit){if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);return (NxtPick){9,*selected};}
        if(g.input_consumed){ModuleCommon_frameEnd(s);continue;}
        int action=0;
        if(NxtEQ_scope()&&(NxtEQ_closeRequested()||NxtControl_pressed(NC_EQ)||NxtPad_justPressed(BTN_SELECT))){NxtEQ_requestClose();action=2;}
        else if(NxtPad_justRepeated(BTN_UP)&&count){*selected=(*selected+count-1)%count;dirty=1;}
        else if(NxtPad_justRepeated(BTN_DOWN)&&count){*selected=(*selected+1)%count;dirty=1;}
        else if(NxtPad_justPressed(BTN_A))action=1;
        else if(NxtPad_justPressed(BTN_B))action=2;
        else if(NxtPad_justPressed(BTN_X))action=3;
        else if(NxtPad_justPressed(BTN_Y))action=4;
        else if(NxtPad_justPressed(BTN_LEFT))action=5;
        else if(NxtPad_justPressed(BTN_RIGHT))action=6;
        else if(NxtPad_justPressed(BTN_START))action=10;
        else if(NxtPad_justPressed(BTN_L1))action=11;
        else if(NxtPad_justPressed(BTN_R1))action=12;
        if(action){if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);return (NxtPick){action,*selected};}
        if(NxtSkin_shouldDraw(last_redraw,dirty||g.dirty)) {
            fonts(s);NxtSkin_begin(s,title,NULL,true);if(picker_ntabs)NxtSkin_tabs(s,picker_tabs,picker_ntabs,picker_tab,NxtSkin_top(s));NxtSkin_list(s,items,count,*selected,true);NxtSkin_footer(s,hint,false);
            ModuleCommon_markSurfaceDrawn();dirty=0;last_redraw=SDL_GetTicks();
        }
        ModuleCommon_PWR_update(&dirty,&show);ModuleCommon_frameEnd(s);
    }
}
NxtPick Nxt_pick(DisplayContext *d,const char *t,const char **i,int n,int *s,const char *h){return pick_internal(d,t,i,n,s,h,false);}
NxtPick Nxt_pickLive(DisplayContext *d,const char *t,const char **i,int n,int *s,const char *h){return pick_internal(d,t,i,n,s,h,true);}
NxtPick Nxt_pickTabs(DisplayContext *d,const char *t,const char **i,int n,int *s,const char *h,const char **tabs,int ntabs,int tab){picker_tabs=tabs;picker_ntabs=ntabs;picker_tab=tab;NxtPick result=pick_internal(d,t,i,n,s,h,true);picker_ntabs=0;return result;}
ModuleExitReason Nxt_document(DisplayContext *display,const char *title,const char *const *paragraphs,int count){int show=0,top=0,total=0,capacity=0,width_cache=0,sig=0;char **lines=NULL;bool dirty=true;Uint32 drawn=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(display);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit){result=MODULE_EXIT_QUIT;break;}fonts(s);int margin=s->w/24,width=s->w-margin*2-12;if(width_cache!=width||sig!=font_signature){free_rows(lines,total);lines=NULL;total=capacity=0;for(int p=0;p<count;p++){int n=0;char **part=wrap(body,paragraphs[p],width,&n);if(total+n+1>capacity){int c=total+n+32;char **new=realloc(lines,c*sizeof(*new));if(!new){free_rows(part,n);break;}lines=new;capacity=c;}for(int j=0;j<n;j++)lines[total++]=part[j];free(part);char *blank=strdup("");if(!blank)break;lines[total++]=blank;}width_cache=width;sig=font_signature;dirty=true;}int lh=TTF_FontHeight(body)+s->w*6/1024,y=NxtSkin_top(s),h=NxtSkin_bottom(s,false)-y,visible=h/lh;if(visible<1)visible=1;int end=total>visible?total-visible:0;if(!g.input_consumed&&!Nxt_lockInput()){if(NxtEQ_scope()&&(NxtEQ_closeRequested()||NxtControl_pressed(NC_EQ)||NxtPad_justPressed(BTN_SELECT))){NxtEQ_requestClose();break;}if(NxtPad_justPressed(BTN_B))break;if(strstr(title,"v1.5")&&strstr(title,nxt.language==1?"Hướng dẫn":"Usage")&&NxtPad_justPressed(BTN_X)){if(Nxt_changes(display)==MODULE_EXIT_QUIT){result=MODULE_EXIT_QUIT;break;}dirty=true;}int move=NxtPad_justRepeated(BTN_UP)?-1:NxtPad_justRepeated(BTN_DOWN)?1:(NxtPad_justPressed(BTN_LEFT)||NxtPad_justPressed(BTN_L1))?-visible:(NxtPad_justPressed(BTN_RIGHT)||NxtPad_justPressed(BTN_R1))?visible:0;if(move){top+=move;dirty=true;}}if(top<0)top=0;if(top>end)top=end;if(NxtSkin_shouldDraw(drawn,dirty||g.dirty)){NxtSkin_begin(s,title,NULL,false);for(int r=0;r<visible&&top+r<total;r++)Nxt_drawText(s,body,lines[top+r],margin,y+r*lh,width,NxtSkin_color(NXT_INK));if(end){int bh=h*visible/total;if(bh<10)bh=10;NxtSkin_rect(s,(SDL_Rect){s->w-margin+4,y+(h-bh)*top/end,4,bh},NxtSkin_color(NXT_MUTED),2);}NxtSkin_footer(s,Nxt_text("↑↓ Scroll · L1/R1 Page · B Back","↑↓ Cuộn · L1/R1 Trang · B Về"),false);ModuleCommon_markSurfaceDrawn();dirty=false;drawn=SDL_GetTicks();}int changed=dirty;ModuleCommon_PWR_update(&changed,&show);dirty|=changed!=0;ModuleCommon_frameEnd(s);}free_rows(lines,total);return result;}

void Nxt_renderPlaying(SDL_Surface *s,int show,int track,int total,bool shuffle,int repeat) {
    fonts(s);NxtSkin_playing(s,track,total,shuffle,repeat); /* NXT owns its volume/brightness toast; never paint firmware hint chips here. */
}
ModuleExitReason Nxt_queue(DisplayContext *display) {
    if(NxtSoundCloud_active())return NxtSoundCloud_queue(display);
    if(NxtSpotify_active())return NxtSpotify_queue(display);
    int selected=PlayerModule_queue()->current_index;
    for(;;) {
        PlaylistContext *q=PlayerModule_queue();const char **labels=calloc(q->track_count?q->track_count:1,sizeof(char*));char (*strings)[300]=calloc(q->track_count?q->track_count:1,300);
        for(int i=0;i<q->track_count;i++){snprintf(strings[i],300,"%s%03d  %s",i==q->current_index?"▶ ":"",i+1,q->tracks[i].name);labels[i]=strings[i];}
        NxtPick pick=Nxt_pickLive(display,Nxt_text("Playing queue","Danh sách đang phát"),labels,q->track_count,&selected,Nxt_text("A Play • X Remove • L/R Move • Y Save M3U • B Back","A Phát • X Bỏ • L/R Đổi chỗ • Y Lưu M3U • B Quay lại"));free(labels);free(strings);
        if(pick.action==2)return MODULE_EXIT_TO_MENU;if(pick.action==9)return MODULE_EXIT_QUIT;
        if(pick.action==1){PlayerModule_selectQueue(selected);return MODULE_EXIT_TO_MENU;}
        if(pick.action==3){if(!PlayerModule_removeQueue(selected))Toast_show(Nxt_text("Current track is kept","Giữ lại bài đang phát"),2000);}
        if(pick.action==11&&selected>0){PlayerModule_moveQueue(selected,selected-1);selected--;}
        if(pick.action==12&&selected<q->track_count-1){PlayerModule_moveQueue(selected,selected+1);selected++;}
        if(pick.action==4){
            char *name=Keyboard_open(Nxt_text("Playlist name","Tên danh sách phát"),120);
            if(name&&*name){char safe[128],dir[1024],file[1200],relative[160];
                if(M3U_sanitizeName(name,safe,sizeof(safe))){
                    M3U_create(safe);snprintf(relative,sizeof(relative),"playlists/%s.m3u",safe);userdata_snpath(relative,file,sizeof(file));FILE *saved=fopen(file,"w");
                    if(saved){fputs("#EXTM3U\n",saved);for(int i=0;i<q->track_count;i++)fprintf(saved,"#EXTINF:-1,%s\n%s\n",q->tracks[i].name,q->tracks[i].path);fclose(saved);}
                    snprintf(dir,sizeof(dir),"%s/Playlists",Nxt_musicRoot());mkdir(dir,0755);snprintf(file,sizeof(file),"%s/%s.m3u8",dir,safe);FILE *out=fopen(file,"w");
                    if(out){fputs("#EXTM3U\n",out);for(int i=0;i<q->track_count;i++)fprintf(out,"#EXTINF:-1,%s\n%s\n",q->tracks[i].name,q->tracks[i].path);fclose(out);Toast_show(Nxt_text("Playlist saved; M3U exported to Music/Playlists","Đã lưu danh sách và xuất M3U trong Playlists"),2500);}else Toast_show(saved?Nxt_text("Playlist saved; M3U export failed","Đã lưu danh sách; xuất M3U thất bại"):Nxt_text("Cannot save M3U","Không lưu được M3U"),2500);
                }
            }free(name);
        }
    }
}
ModuleExitReason Nxt_lyricsScreen(DisplayContext *display) {
    Spectrum_suspendVisual(true);NxtRecord_hide();UiLayer_clear(UI_LAYER_ANIMATION);UiLayer_clear(UI_LAYER_STATUS);
    int seen_current=-2,scroll_from=0,scroll_to=0;Uint32 lyric_started=0;
    int cursor=0,old_count=-1,old_size=-1,old_mode=-1,rows_count=0;unsigned old_gen=0,old_lyrics=0,old_translation=0;bool follow=true;int show=0,dirty=1;BackgroundPlayerType previous=Background_getActive();if(previous==BG_NONE&&PlayerModule_isActive())Background_setActive(BG_MUSIC);
    char **rows=NULL;int *owners=NULL;uint32_t last=0;
    for(;;) {
        ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(display);fonts(s);
        GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit||ModuleCommon_stopSignalled()){free_rows(rows,rows_count);free(owners);if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);Spectrum_suspendVisual(false);return MODULE_EXIT_QUIT;}if(g.input_consumed||Nxt_lockInput()){ModuleCommon_frameEnd(s);continue;}
        int count=Lyrics_lineCount(),current=Lyrics_currentIndex(NxtPlayback_position());unsigned gen=NxtPlayback_generation();
        unsigned lyric_revision=Lyrics_revision(),translation_revision=NxtTranslate_revision();
        if(current!=seen_current){if(nxt.animations&&current==seen_current+1)lyric_started=SDL_GetTicks();else lyric_started=SDL_GetTicks()-360;seen_current=current;last=0;}
        if(count!=old_count||old_size!=nxt.lyric_size||old_gen!=gen||old_mode!=nxt.translation_mode||old_lyrics!=lyric_revision||old_translation!=translation_revision)last=0;
        if(count!=old_count||old_size!=nxt.lyric_size||old_gen!=gen||old_mode!=nxt.translation_mode||old_lyrics!=lyric_revision){free_rows(rows,rows_count);free(owners);rows=NULL;owners=NULL;rows_count=0;
            int factor=nxt.translation_mode==2?2:1;
            if(count>0){rows=calloc(count*factor,sizeof(char*));owners=calloc(count*factor,sizeof(int));if(rows&&owners){for(int i=0;i<count;i++){const char *line=Lyrics_lineText(i);rows[rows_count]=strdup(line?line:"");if(!rows[rows_count])break;owners[rows_count++]=i;if(factor==2){rows[rows_count]=strdup("");if(!rows[rows_count])break;owners[rows_count++]=i;}}}else{free(rows);free(owners);rows=NULL;owners=NULL;}}old_count=count;old_size=nxt.lyric_size;old_gen=gen;old_mode=nxt.translation_mode;old_lyrics=lyric_revision;old_translation=0;}
        /* Refresh only changed strings: the worker never writes UI-owned rows. */
        if(nxt.translation_mode&&old_translation!=translation_revision){
            for(int i=0;i<count;i++){int row=nxt.translation_mode==2?i*2+1:i;if(row>=rows_count)break;const char *value=nxt.translation_mode==2?NxtTranslate_line(i):NxtTranslate_display(i);if(!value)value="";if(strcmp(rows[row],value)){char *copy=strdup(value);if(copy){free(rows[row]);rows[row]=copy;}}}
            old_translation=translation_revision;
        }
        if(NxtPad_justPressed(BTN_START)){if(NxtTranslate_settings(display)==MODULE_EXIT_QUIT){free_rows(rows,rows_count);free(owners);if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);Spectrum_suspendVisual(false);return MODULE_EXIT_QUIT;}last=0;}
        if(NxtPad_justPressed(BTN_B)||ModuleCommon_stopSignalled()){free_rows(rows,rows_count);free(owners);if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);Spectrum_suspendVisual(false);return ModuleCommon_stopSignalled()?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
        if(NxtPad_justPressed(BTN_A))NxtPlayback_toggle();
        if(NxtPad_justRepeated(BTN_UP)){lyric_started=SDL_GetTicks();follow=false;if(cursor>0){cursor-=nxt.interface_style==NXT_THEME_AURALIS&&nxt.translation_mode==2?2:1;if(cursor<0)cursor=0;}}
        if(NxtPad_justRepeated(BTN_DOWN)){lyric_started=SDL_GetTicks();follow=false;if(cursor<rows_count-1){cursor+=nxt.interface_style==NXT_THEME_AURALIS&&nxt.translation_mode==2?2:1;if(cursor>=rows_count)cursor=rows_count-1;}}
        if(NxtPad_justPressed(BTN_Y)){if(follow&&nxt.interface_style==NXT_THEME_AURALIS&&current>=0)cursor=current*(nxt.translation_mode==2?2:1);follow=!follow;}
        if(NxtPad_justPressed(BTN_X)){nxt.lyric_size+=4;if(nxt.lyric_size>48)nxt.lyric_size=24;Nxt_save();}
        if(NxtPad_justPressed(BTN_L1)){nxt.lyric_offset-=100;Nxt_save();}
        if(NxtPad_justPressed(BTN_R1)){nxt.lyric_offset+=100;Nxt_save();}
        if(NxtPad_justRepeated(BTN_LEFT))NxtPlayback_seek(NxtPlayback_position()-5000);
        if(NxtPad_justRepeated(BTN_RIGHT))NxtPlayback_seek(NxtPlayback_position()+5000);
        if((PAD_anyJustPressed()||NxtPad_justRepeated(BTN_UP)||NxtPad_justRepeated(BTN_DOWN)||NxtPad_justRepeated(BTN_LEFT)||NxtPad_justRepeated(BTN_RIGHT)))last=0;
        bool moving=nxt.animations&&(nxt.lyric_effect&&(Uint32)(SDL_GetTicks()-lyric_started)<360||(nxt.interface_style==NXT_THEME_AURALIS&&NxtAuralis_motion())||(NxtPlayback_state()==PLAYER_STATE_PLAYING&&Lyrics_wordMotion(current,NxtPlayback_position())));
        uint32_t interval=moving?16:NxtPlayback_state()==PLAYER_STATE_PLAYING?250:1000;
        if(!PWR_getDisplayOff()&&!NxtSession_isHidden()&&(!last||SDL_GetTicks()-last>=interval)) {
            if(nxt.interface_style==NXT_THEME_AURALIS){
                if(cursor>=rows_count)cursor=rows_count>0?rows_count-1:0;
                NxtAuralis_lyrics(s,rows_count&&owners?owners[cursor]:0,follow);
            }else{
            char title[128];snprintf(title,sizeof(title),"%s • %+d ms",Lyrics_isAvailable()&&!Lyrics_canFollow()?Nxt_text("Lyrics · unsynced","Lời · không đồng bộ"):Lyrics_wordEstimated(current)?Nxt_text("Lyrics · estimated","Lời · ước lượng"):Nxt_text("Lyrics","Lời bài hát"),nxt.lyric_offset);
            if(nxt.translation_mode){char base[128];snprintf(base,sizeof(base),"%s",title);snprintf(title,sizeof(title),"%s · %s",base,NxtTranslate_modeName(nxt.translation_mode));}
            page(s,title,Nxt_text("↑↓ Scroll · Y Follow · X Font · L1/R1 Offset · START Translate · A Pause · B Back","↑↓ Cuộn · Y Bám · X Cỡ · L1/R1 Lệch · START Dịch · A Dừng · B Về"));NxtMotion_setMenu(false);
            int p=s->w/40,top=page_top(s),rh=TTF_FontHeight(lyric)+8,visible=(s->h-top-p*2-TTF_FontHeight(small)*2)/rh;
            if(follow&&Lyrics_canFollow()){cursor=0;for(int i=0;i<rows_count;i++)if(owners[i]==current){cursor=i;break;}cursor-=visible/3;if(cursor<0)cursor=0;}
            if(!rows_count)Nxt_drawWrapped(s,body,NxtSoundCloud_active()?NxtSoundCloud_lyricsStatus():NxtSpotify_active()?NxtSpotify_lyricsStatus():Lyrics_status(),p,top,s->w-p*2,s->h-top-p*3,color(THEME_ROLE_SECONDARY));
            if(scroll_to!=cursor){scroll_from=scroll_to;scroll_to=cursor;if(abs(scroll_to-scroll_from)>3)scroll_from=scroll_to;}
            float t=NxtLyricFX_progress((Uint32)(SDL_GetTicks()-lyric_started));
            int shift=NxtLyricFX_slides()?(int)((scroll_to-scroll_from)*rh*(1.f-t)):0;
            SDL_Rect prior;SDL_GetClipRect(s,&prior);SDL_Rect content={p,top,s->w-p*2,visible*rh};SDL_SetClipRect(s,&content);
            int start=cursor-3;if(start<0)start=0;
            for(int i=start;i<rows_count&&i<cursor+visible+3;i++){
                const char *translation=nxt.translation_mode?NxtTranslate_line(owners[i]):NULL;
                bool translated=translation&&*translation&&(nxt.translation_mode==1||(nxt.translation_mode==2&&(i&1)));
                SDL_Color ink=color(owners[i]==current&&Lyrics_canFollow()?THEME_ROLE_PROGRESS_FILL:THEME_ROLE_SECONDARY);
                int dx=0,dy=0;Uint8 alpha=255;if(owners[i]==current)NxtLyricFX_transform(t,s->w,&dx,&dy,&alpha);ink.a=alpha;
                NxtLyricPreview_row(s,rows[i],owners[i],translated,owners[i]==current,p+dx,top+(i-cursor)*rh+shift+dy,s->w-p*2,rh-8,alpha);
            }
            SDL_SetClipRect(s,&prior);
            }
            ModuleCommon_markSurfaceDrawn();last=SDL_GetTicks();
        }
        ModuleCommon_PWR_update(&dirty,&show);ModuleCommon_frameEnd(s);
    }
}

static bool integer_input(const char *prompt,int *out,int minimum,int maximum) {
    char *s=Keyboard_open(prompt,12);if(!s)return false;char *end;errno=0;long v=strtol(s,&end,10);
    bool ok=*s&&!*end&&!errno&&v>=minimum&&v<=maximum;free(s);
    if(ok)*out=v;else Toast_show(Nxt_text("Enter a valid number","Hãy nhập số hợp lệ"),2000);return ok;
}
static const char *actions[]={"EQ / DSP","Favorite","Queue","Stop","Lock"};
static const char *actions_vi[]={"EQ / DSP","Yêu thích","Danh sách đang phát","Dừng","Khóa"};
static ModuleExitReason mapping_screen(DisplayContext *display){
    int selected=0;for(;;){char text[15][160];const char *labels[15];for(int i=0;i<14;i++){int physical=0;for(int j=0;j<14;j++)if(nxt.keys[i]==Nxt_keyIds[j])physical=j;snprintf(text[i],160,"%s %s : %s %s",Nxt_text("Action","Chức năng"),Nxt_keyNames[i],Nxt_text("physical button","nút máy"),Nxt_keyNames[physical]);labels[i]=text[i];}labels[14]=Nxt_text("Reset all buttons","Khôi phục tất cả nút");
        NxtPick p=Nxt_pick(display,Nxt_text("Button mapping","Đổi nút điều khiển"),labels,15,&selected,Nxt_text("←→ Swap • A Next • B Back • SELECT+START Lock","←→ Đổi • A Nút kế • B Quay lại • SELECT+START Khóa"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;
        if(p.action==1||p.action==5||p.action==6){if(selected==14)for(int i=0;i<14;i++)nxt.keys[i]=Nxt_keyIds[i];else{int old=nxt.keys[selected],pos=0;for(int i=0;i<14;i++)if(Nxt_keyIds[i]==old)pos=i;int newkey=Nxt_keyIds[(pos+(p.action==5?13:1))%14];for(int i=0;i<14;i++)if(i!=selected&&nxt.keys[i]==newkey)nxt.keys[i]=old;nxt.keys[selected]=newkey;}Nxt_save();}
    }
}
static ModuleExitReason shortcuts_screen(DisplayContext *display) {
    int selected=0;const char *keys[]={"SELECT + A","SELECT + X","SELECT + Y","SELECT + B","SELECT + START"};
    for(;;){char lines[7][160];const char *labels[7];for(int i=0;i<4;i++){snprintf(lines[i],160,"%s : %s",keys[i],nxt.language==1?actions_vi[nxt.shortcut[i]]:actions[nxt.shortcut[i]]);labels[i]=lines[i];}labels[4]=Nxt_text("SELECT + START : Lock / unlock (reserved)","SELECT + START : Khóa / mở khóa (cố định)");labels[5]=Nxt_text("Reset shortcuts","Khôi phục phím tắt");labels[6]=Nxt_text("Remap D-pad / A/B/X/Y / shoulder buttons","Đổi nút D-pad / A/B/X/Y / nút vai");
        NxtPick p=Nxt_pick(display,Nxt_text("Button shortcuts","Phím tắt"),labels,7,&selected,Nxt_text("A / ←→ Change • B Back","A / ←→ Đổi • B Quay lại"));
        if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;
        if(p.action==1||p.action==5||p.action==6){if(selected==6){ModuleExitReason r=mapping_screen(display);if(r==MODULE_EXIT_QUIT)return r;}else if(selected==5)for(int i=0;i<5;i++)nxt.shortcut[i]=i;else if(selected<4)nxt.shortcut[selected]=(nxt.shortcut[selected]+(p.action==5?4:1))%5;Nxt_save();}
    }
}
static ModuleExitReason output_details(DisplayContext *display){
    SDL_Surface *s=DisplayHelper_getSurface(display);fonts(s);int n=0;char **rows=wrap(body,NxtHardware_details(),s->w-s->w/10,&n);int selected=0;ModuleExitReason reason=MODULE_EXIT_TO_MENU;
    for(;;){NxtPick p=Nxt_pick(display,Nxt_text("DAC / output","DAC / đầu ra"),(const char **)rows,n,&selected,Nxt_text("↑↓ Scroll • B Back","↑↓ Cuộn • B Quay lại"));if(p.action==2||p.action==9){reason=p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;break;}}free_rows(rows,n);return reason;
}
static ModuleExitReason utilities(DisplayContext *d){int sel=0;for(;;){char text[160];snprintf(text,sizeof(text),"%s · %s",Nxt_text("Auto screen off","Tự tắt màn hình"),SettingsModule_getScreenOffDisplayStr());const char *rows[]={text};NxtPick r=Nxt_pick(d,Nxt_text("Auto screen off","Tự tắt màn hình"),rows,1,&sel,Nxt_text("←→ Change • B Back","←→ Đổi • B Về"));if(r.action==2||r.action==9)return r.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(r.action==5)SettingsModule_cycleScreenOffPrev();if(r.action==1||r.action==6)SettingsModule_cycleScreenOffNext();}}

ModuleExitReason Nxt_settingsLegacy(DisplayContext *display) {
    int selected=0;
    for(;;){char lines[23][560];const char *labels[23];const char *themes[]={"System","Dark","Light"},*themes_vi[]={"Theo hệ thống","Tối","Sáng"};const char *outputs[]={"Speaker","Bluetooth","USB DAC","Automatic"},*outputs_vi[]={"Loa máy","Bluetooth","USB DAC","Tự động"};const char *trans[]={"Normal","Gapless","Crossfade"},*trans_vi[]={"Thông thường","Liền mạch","Chồng âm"};const char *rg[]={"Off","Track","Album"},*rg_vi[]={"Tắt","Theo bài","Theo album"};
#define ROW(i,en,vi,format,value) snprintf(lines[i],sizeof(lines[i]),"%s : " format,Nxt_text(en,vi),value)
        ROW(0,"Language","Ngôn ngữ","%s",nxt.language==1?"Tiếng Việt":"English");ROW(1,"Theme","Giao diện","%s",nxt.language==1?themes_vi[nxt.theme]:themes[nxt.theme]);ROW(2,"Font scale","Cỡ chữ giao diện","%d%%",nxt.font_size);
        ROW(3,"Lyric font","Cỡ chữ lời","%d px",nxt.lyric_size);ROW(4,"Lyric delay","Độ trễ lời","%+d ms",nxt.lyric_offset);ROW(5,"Music folder","Thư mục nhạc","%s",nxt.music_root);ROW(6,"Font file","File font","%s",nxt.font_path[0]?nxt.font_path:"DejaVu Sans");ROW(7,"Sleep timer","Hẹn giờ dừng","%d min",Nxt_timerRemaining()/60+(Nxt_timerRemaining()%60!=0));
        ROW(8,"Audio output","Đầu ra","%s",nxt.language==1?outputs_vi[nxt.output]:outputs[nxt.output]);ROW(9,"When DAC is unplugged","Khi rút DAC","%s",nxt.unplug_pause?Nxt_text("Pause","Tạm dừng"):Nxt_text("Switch output","Chuyển đầu ra"));ROW(10,"Track transition","Chuyển bài","%s",nxt.language==1?trans_vi[nxt.transition]:trans[nxt.transition]);ROW(11,"Crossfade","Thời gian chồng âm","%d ms",nxt.crossfade_ms);ROW(12,"ReplayGain","ReplayGain","%s",nxt.language==1?rg_vi[nxt.replaygain]:rg[nxt.replaygain]);ROW(13,"Reactive LEDs","LED theo nhạc","%s",nxt.led?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));ROW(14,"LED intensity","Độ sáng LED","%d%%",nxt.led_strength);ROW(15,"EQ / DSP","EQ / DSP","%s",nxt.dsp?Nxt_text("On","Bật"):Nxt_text("Bypass","Bỏ qua"));ROW(16,"Button shortcuts","Phím tắt","%s","…");ROW(17,"Keep playing after exit","Phát nhạc sau khi thoát","%s",nxt.background?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));ROW(18,"Usage guide","Hướng dẫn sử dụng","%s","…");ROW(19,"About","Thông tin ứng dụng","%s","…");ROW(20,"MoMo support number","Số MoMo ủng hộ","%s",nxt.momo[0]?nxt.momo:Nxt_text("Not set","Chưa nhập"));ROW(21,"System / Wi-Fi","Hệ thống / Wi-Fi","%s","…");ROW(22,"Output information","Thông tin DAC / đầu ra","%s",NxtHardware_output());
#undef ROW
        for(int i=0;i<23;i++)labels[i]=lines[i];
        NxtPick p=Nxt_pick(display,Nxt_text("Settings","Cài đặt"),labels,23,&selected,Nxt_text("A / ←→ Change • B Back","A / ←→ Thay đổi • B Quay lại"));
        if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;if(p.action!=1&&p.action!=5&&p.action!=6)continue;int d=p.action==5?-1:1;ModuleExitReason reason=MODULE_EXIT_TO_MENU;
        switch(selected){
            case 0:reason=NxtLocale_run(display);break;
            case 1:nxt.theme=(nxt.theme+d+3)%3;break;
            case 2:nxt.font_size+=d*10;if(nxt.font_size<70)nxt.font_size=150;if(nxt.font_size>150)nxt.font_size=70;Fonts_unload();Fonts_load();font_signature=0;break;
            case 3:nxt.lyric_size+=d*2;if(nxt.lyric_size<18)nxt.lyric_size=60;if(nxt.lyric_size>60)nxt.lyric_size=18;break;
            case 4:nxt.lyric_offset+=d*100;if(nxt.lyric_offset>30000)nxt.lyric_offset=30000;if(nxt.lyric_offset< -30000)nxt.lyric_offset=-30000;break;
            case 5:Nxt_chooseFolder(display);break;
            case 6:{char *f=Keyboard_openLatin(Nxt_text("Absolute font path; empty resets","Đường dẫn font; để trống để khôi phục"),511);if(f){TTF_Font *checked=*f?TTF_OpenFont(f,24):NULL;if(!*f||checked){if(checked)TTF_CloseFont(checked);snprintf(nxt.font_path,sizeof(nxt.font_path),"%s",f);Fonts_unload();Fonts_load();font_signature=0;}else Toast_show(Nxt_text("Font file not found","Không tìm thấy file font"),2000);free(f);}break;}
            case 7:{int minutes;if(integer_input(Nxt_text("Minutes until stop (0 cancels)","Số phút dừng (0 để hủy)"),&minutes,0,INT_MAX))Nxt_sleepTimer(minutes);break;}
            case 8:{int out=(nxt.output+d+4)%4;if(NxtHardware_select(out))nxt.output=out;else Toast_show(Nxt_text("Output unavailable","Đầu ra chưa sẵn sàng"),2500);break;}
            case 9:nxt.unplug_pause=!nxt.unplug_pause;break;
            case 10:nxt.transition=(nxt.transition+d+3)%3;break;
            case 11:integer_input(Nxt_text("Crossfade in milliseconds (500–12000)","Chồng âm: mili giây (500–12000)"),&nxt.crossfade_ms,500,12000);break;
            case 12:nxt.replaygain=(nxt.replaygain+d+3)%3;break;
            case 13:nxt.led=!nxt.led;break;
            case 14:nxt.led_strength+=d*10;if(nxt.led_strength>100)nxt.led_strength=0;if(nxt.led_strength<0)nxt.led_strength=100;break;
            case 15:reason=Nxt_dspScreen(display);break;
            case 16:reason=shortcuts_screen(display);break;
            case 17:nxt.background=!nxt.background;break;
            case 18:reason=Nxt_help(display);break;
            case 19:reason=Nxt_about(display);break;
            case 20:{char *m=Keyboard_open(Nxt_text("MoMo support number","Số MoMo ủng hộ"),63);if(m){snprintf(nxt.momo,sizeof(nxt.momo),"%s",m);free(m);}break;}
            case 21:reason=utilities(display);break;
            case 22:reason=output_details(display);break;
        }
        if(reason==MODULE_EXIT_QUIT)return reason;
        Nxt_save();GFX_setClearColor(Theme_getPackedColor(THEME_ROLE_PAGE_BACKGROUND,false));
    }
}
static ModuleExitReason parametric(DisplayContext *display) {
    int selected=0;for(;;){char lines[24][128];const char *labels[24];for(int i=0;i<6;i++){snprintf(lines[i*4],128,"%s %d : %s",Nxt_text("Filter","Bộ lọc"),i+1,nxt.peq[i].enabled?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));snprintf(lines[i*4+1],128,"%d • %.0f Hz",i+1,nxt.peq[i].hz);snprintf(lines[i*4+2],128,"%d • %+.1f dB",i+1,nxt.peq[i].gain);snprintf(lines[i*4+3],128,"%d • Q %.2f",i+1,nxt.peq[i].q);for(int k=0;k<4;k++)labels[i*4+k]=lines[i*4+k];}NxtPick p=Nxt_pick(display,Nxt_text("Parametric EQ","EQ tham số"),labels,24,&selected,Nxt_text("←→ Adjust • A Toggle / enter Hz • B Back","←→ Chỉnh • A Bật / nhập Hz • B Quay lại"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==1||p.action==5||p.action==6){int i=selected/4,d=p.action==5?-1:1;switch(selected%4){case 0:nxt.peq[i].enabled=!nxt.peq[i].enabled;break;case 1:if(p.action==1){int f;if(integer_input("Hz (20–20000)",&f,20,20000))nxt.peq[i].hz=f;}else nxt.peq[i].hz=fmaxf(20,fminf(20000,nxt.peq[i].hz*powf(2,d/12.f)));break;case 2:nxt.peq[i].gain=fmaxf(-12,fminf(12,nxt.peq[i].gain+d*.5f));break;case 3:nxt.peq[i].q=fmaxf(.1f,fminf(12,nxt.peq[i].q+d*.1f));break;}Nxt_save();}}
}
static void preset_save(void) {
    char *name=Keyboard_open(Nxt_text("Preset name","Tên preset"),80);if(!name)return;char safe[128],file[1024];if(M3U_sanitizeName(name,safe,sizeof(safe))){userdata_snpath("presets",file,sizeof(file));mkdir(file,0755);char path[1200];snprintf(path,sizeof(path),"%s/%s.nxt-eq",file,safe);FILE *f=fopen(path,"w");if(f){fprintf(f,"NXT-EQ-1\n%d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %d\n",nxt.eq_bands,nxt.preamp,nxt.bass,nxt.treble,nxt.balance,nxt.width,nxt.compressor,nxt.crossfeed,nxt.limiter);for(int i=0;i<31;i++)fprintf(f,"%.4f\n",nxt.eq[i]);for(int i=0;i<6;i++)fprintf(f,"%.4f %.4f %.4f %d\n",nxt.peq[i].hz,nxt.peq[i].gain,nxt.peq[i].q,nxt.peq[i].enabled);fclose(f);Toast_show(Nxt_text("Preset saved","Đã lưu preset"),2000);}}free(name);
}
static void preset_load(void) {
    char *path=Keyboard_openLatin(Nxt_text("Preset path (.nxt-eq)","Đường dẫn preset (.nxt-eq)"),511);if(!path)return;FILE *f=fopen(path,"r");if(f){char magic[32];NxtConfig c=nxt;bool ok=fgets(magic,sizeof(magic),f)&&!strcmp(magic,"NXT-EQ-1\n")&&fscanf(f,"%d %f %f %f %f %f %f %f %d",&c.eq_bands,&c.preamp,&c.bass,&c.treble,&c.balance,&c.width,&c.compressor,&c.crossfeed,&c.limiter)==9;for(int i=0;ok&&i<31;i++)ok=fscanf(f,"%f",&c.eq[i])==1&&isfinite(c.eq[i])&&fabsf(c.eq[i])<=12;for(int i=0;ok&&i<6;i++)ok=fscanf(f,"%f %f %f %d",&c.peq[i].hz,&c.peq[i].gain,&c.peq[i].q,&c.peq[i].enabled)==4&&c.peq[i].hz>=20&&c.peq[i].hz<=20000&&fabsf(c.peq[i].gain)<=12&&c.peq[i].q>=.1&&c.peq[i].q<=12;fclose(f);if(ok&&(c.eq_bands==5||c.eq_bands==10||c.eq_bands==15||c.eq_bands==31)&&c.preamp>=-24&&c.preamp<=12&&fabsf(c.bass)<=12&&fabsf(c.treble)<=12&&fabsf(c.balance)<=1&&c.width>=0&&c.width<=2&&c.compressor>=0&&c.compressor<=1&&c.crossfeed>=0&&c.crossfeed<=1){nxt=c;Nxt_save();Toast_show(Nxt_text("Preset loaded","Đã nạp preset"),2000);}else Toast_show(Nxt_text("Invalid preset","Preset không hợp lệ"),2000);}else Toast_show(Nxt_text("Preset not found","Không tìm thấy preset"),2000);free(path);
}
ModuleExitReason Nxt_dspAdvanced(DisplayContext *display) {
    int selected=0;for(;;){int count=17+nxt.eq_bands;char lines[48][128];const char *labels[48];
        snprintf(lines[0],128,"%s : %s",Nxt_text("DSP","DSP"),nxt.dsp?Nxt_text("On","Bật"):Nxt_text("Bypass","Bỏ qua"));snprintf(lines[1],128,"EQ : %d %s",nxt.eq_bands,Nxt_text("bands","dải"));snprintf(lines[2],128,"Preamp : %+.1f dB",nxt.preamp);snprintf(lines[3],128,"Bass : %+.1f dB",nxt.bass);snprintf(lines[4],128,"Treble : %+.1f dB",nxt.treble);snprintf(lines[5],128,"%s : %+.0f%%",Nxt_text("Balance","Cân bằng L/R"),nxt.balance*100);snprintf(lines[6],128,"%s : %.0f%%",Nxt_text("Stereo width","Độ rộng stereo"),nxt.width*100);snprintf(lines[7],128,"Compressor : %.0f%%",nxt.compressor*100);snprintf(lines[8],128,"Crossfeed : %.0f%%",nxt.crossfeed*100);snprintf(lines[9],128,"Limiter : %s",nxt.limiter?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));snprintf(lines[10],128,"%s",Nxt_text("Parametric EQ (6 filters)","EQ tham số (6 bộ lọc)"));snprintf(lines[11],128,"%s",Nxt_text("Flat / reset","Flat / đặt lại"));snprintf(lines[12],128,"%s",Nxt_text("Save preset","Lưu preset"));snprintf(lines[13],128,"%s",Nxt_text("Import preset","Nhập preset"));snprintf(lines[14],128,"%s",Nxt_text("Bass preset","Preset bass"));snprintf(lines[15],128,"%s",Nxt_text("Vocal preset","Preset giọng hát"));snprintf(lines[16],128,"%s",Nxt_text("Response curve","Đường đáp ứng"));
        for(int i=0;i<nxt.eq_bands;i++)snprintf(lines[17+i],128,"%2d • %5.0f Hz : %+.1f dB",i+1,NxtEQ_frequency(nxt.eq_bands,i),nxt.eq[i]);for(int i=0;i<count;i++)labels[i]=lines[i];
        NxtPick p=Nxt_pick(display,Nxt_text("Audio DSP","Xử lý âm thanh DSP"),labels,count,&selected,Nxt_text("←→ Adjust • A Open • B Back","←→ Chỉnh • A Mở • B Quay lại"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;if(p.action!=1&&p.action!=5&&p.action!=6)continue;int d=p.action==5?-1:1;ModuleExitReason r=MODULE_EXIT_TO_MENU;
        switch(selected){case 0:nxt.dsp=!nxt.dsp;break;case 1:Nxt_EQBands(nxt.eq_bands==10?15:nxt.eq_bands==15?31:10);break;case 2:nxt.preamp=fmaxf(-24,fminf(12,nxt.preamp+d*.5f));break;case 3:nxt.bass=fmaxf(-12,fminf(12,nxt.bass+d*.5f));break;case 4:nxt.treble=fmaxf(-12,fminf(12,nxt.treble+d*.5f));break;case 5:nxt.balance=fmaxf(-1,fminf(1,nxt.balance+d*.05f));break;case 6:nxt.width=fmaxf(0,fminf(2,nxt.width+d*.05f));break;case 7:nxt.compressor=fmaxf(0,fminf(1,nxt.compressor+d*.05f));break;case 8:nxt.crossfeed=fmaxf(0,fminf(1,nxt.crossfeed+d*.05f));break;case 9:nxt.limiter=!nxt.limiter;break;case 10:r=parametric(display);break;case 11:memset(nxt.eq,0,sizeof(nxt.eq));for(int i=0;i<6;i++)nxt.peq[i].enabled=0;nxt.preamp=nxt.bass=nxt.treble=nxt.balance=nxt.compressor=nxt.crossfeed=0;nxt.width=1;break;case 12:preset_save();break;case 13:preset_load();break;case 14:nxt.dsp=1;nxt.preamp=-4;nxt.bass=4;break;case 15:nxt.dsp=1;nxt.preamp=-3;for(int i=0;i<nxt.eq_bands;i++){float hz=NxtEQ_frequency(nxt.eq_bands,i);nxt.eq[i]=hz>=500&&hz<=4000?2:0;}break;case 16:{SDL_Surface *s=DisplayHelper_getSurface(display);page(s,Nxt_text("EQ response (dB)","Đáp ứng EQ (dB)"),Nxt_text("B Back","B Quay lại"));int left=s->w/20,top=page_top(s),bottom=s->h-80;for(int x=left;x<s->w-left;x++){float hz=20*powf(1000,(float)(x-left)/(s->w-left*2)),db=NxtDSP_response(hz);int y=(top+bottom)/2-db*(bottom-top)/48;fill(s,(SDL_Rect){x,y,2,3},color(THEME_ROLE_PROGRESS_FILL));}ModuleCommon_markSurfaceDrawn();ModuleCommon_frameEnd(s);while(!NxtPad_justPressed(BTN_B)&&!ModuleCommon_stopSignalled()){ModuleCommon_frameBegin();ModuleCommon_frameEnd(s);}break;}default:if(selected>=17)nxt.eq[selected-17]=fmaxf(-12,fminf(12,nxt.eq[selected-17]+d*.5f));break;}
        Nxt_save();if(r==MODULE_EXIT_QUIT)return r;
    }
}
bool Nxt_isLocked(void) { return locked; }
bool Nxt_shortcuts(DisplayContext *display) {
    if(Nxt_displayInputBlocked())return false;
    bool consume=Nxt_lockInput()||PAD_isPressed(BTN_SELECT);if(consume&&display&&!PWR_getDisplayOff()){static uint32_t draw;uint32_t now=SDL_GetTicks();if(now-draw>=100){PlaylistContext *q=PlayerModule_queue();NxtSkin_playing(DisplayHelper_getSurface(display),q->current_index+1,q->track_count,nxt.shuffle,nxt.repeat_mode);ModuleCommon_markSurfaceDrawn();draw=now;}}return consume;
}
bool Nxt_selectReleased(void){return NxtPad_justPressed(BTN_SELECT)!=0;}
ModuleExitReason Nxt_helpLegacy(DisplayContext *display) {
    char text[30][240];const char *labels[30];const char *keys[]={"SELECT+A","SELECT+X","SELECT+Y","SELECT+B"};
    snprintf(text[0],240,"%s",Nxt_text("D-pad: browse. A: choose / play / pause.","D-pad: di chuyển. A: chọn / phát / tạm dừng."));snprintf(text[1],240,"%s",Nxt_text("B: back; playback continues.","B: quay lại; nhạc tiếp tục phát."));snprintf(text[2],240,"%s",Nxt_text("L1/R1 or Down/Up: previous/next track.","L1/R1 hoặc Xuống/Lên: bài trước/tiếp."));snprintf(text[3],240,"%s",Nxt_text("Hold Left/Right: seek. X: shuffle. Y: repeat mode.","Giữ Trái/Phải: tua. X: ngẫu nhiên. Y: chế độ lặp."));snprintf(text[4],240,"%s",Nxt_text("L2: spectrum. R2: fullscreen lyrics.","L2: phổ nhạc. R2: lời toàn màn hình."));snprintf(text[5],240,"%s",Nxt_text("SELECT: screen off. SELECT+A wakes it.","SELECT: tắt màn hình. SELECT+A bật lại."));snprintf(text[6],240,"%s",Nxt_text("Hold START: exit UI; music stays on if enabled.","Giữ START: thoát giao diện; giữ nhạc nếu đã bật."));snprintf(text[7],240,"%s",Nxt_text("SELECT+START: lock/unlock (always available).","SELECT+START: khóa/mở khóa (luôn dùng được)."));for(int i=0;i<4;i++)snprintf(text[8+i],240,"%s : %s",keys[i],nxt.language==1?actions_vi[nxt.shortcut[i]]:actions[nxt.shortcut[i]]);snprintf(text[12],240,"%s",Nxt_text("Lyrics: Up/Down scroll, Y follow, X font, L/R delay.","Lời: Lên/Xuống cuộn, Y bám lời, X cỡ chữ, L/R độ trễ."));snprintf(text[13],240,"%s",Nxt_text("Queue: A play, L/R move, X remove, Y export M3U.","Danh sách: A phát, L/R đổi chỗ, X bỏ, Y xuất M3U."));snprintf(text[14],240,"%s",Nxt_text("Sleep timer uses real elapsed time, including pause.","Hẹn giờ tính thời gian thực, kể cả khi tạm dừng."));snprintf(text[15],240,"%s",Nxt_text("Reopen the same Pak to control the existing session.","Mở lại cùng Pak để điều khiển phiên đang phát."));for(int i=0;i<14;i++){int key=0;for(int j=0;j<14;j++)if(Nxt_keyIds[j]==nxt.keys[i])key=j;snprintf(text[16+i],240,"%s %s → %s %s",Nxt_text("Action","Chức năng"),Nxt_keyNames[i],Nxt_text("physical button","nút máy"),Nxt_keyNames[key]);}for(int i=0;i<30;i++)labels[i]=text[i];int selected=0;for(;;){NxtPick p=Nxt_pick(display,Nxt_text("Usage guide","Hướng dẫn sử dụng"),labels,30,&selected,Nxt_text("↑↓ Scroll • B Back","↑↓ Cuộn • B Quay lại"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}
}
ModuleExitReason Nxt_aboutLegacy(DisplayContext *display) {
    char version[160],momo[160];snprintf(version,sizeof(version),"NXT Retro Music %s • NextUI / Brick Pro",NXT_VERSION);snprintf(momo,sizeof(momo),"MoMo: %s",nxt.momo[0]?nxt.momo:Nxt_text("Not configured","Chưa cung cấp"));
    const char *labels[]={version,"Vibe Code",Nxt_text("Concept: Nguyễn Xuân Thông","Lên ý tưởng bởi Nguyễn Xuân Thông"),"Spring Thông",momo,Nxt_text("Based on NextUI Music Player","Dựa trên NextUI Music Player"),"Mohammad Syuhada • nborodikhin • LoveRetro","NextUI / NXT (GPLv3) • FFmpeg (LGPL 2.1+)",Nxt_text("Local DSP; no ViPER preset compatibility claimed","DSP tích hợp; không hỗ trợ preset ViPER riêng"),Nxt_text("DAC scores require an identified model and review source","Điểm DAC cần đúng model và nguồn đánh giá")};int selected=0;for(;;){NxtPick p=Nxt_pick(display,Nxt_text("About","Thông tin ứng dụng"),labels,10,&selected,Nxt_text("↑↓ Scroll • B Back","↑↓ Cuộn • B Quay lại"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;}
}

ModuleExitReason Nxt_systemScreen(DisplayContext *d){return utilities(d);}
