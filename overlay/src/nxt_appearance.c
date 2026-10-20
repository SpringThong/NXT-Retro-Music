#include "nxt_lyric_preview.h"
#include "nxt_locale.h"
/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_appearance.h"
#include "nxt_auralis.h"
#include "nxt_karaoke.h"
#include "nxt_theme.h"
#include "nxt_custom.h"
#include "nxt_monitor.h"
#include "nxt_wallpapers.h"
#include "nxt_wallpaper_store.h"
#include "nxt_skin.h"
#include "nxt_personal.h"
#include "nxt_log.h"
#include "nxt_playback.h"
#include "nxt_record.h"
#include "album_art.h"
#include "background.h"
#include "module_common.h"
#include "file_utils.h"
#include "keyboard.h"
#include "toast.h"
#include "ui_fonts.h"
#include "nxt_text.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
static const unsigned colors[]={0x83ddb0,0x63b9ef,0xd6a3ff,0xffa4c9,0xf9c46b,0xef9586,0x80e1d6,0xa0d989,0xa8b5ff,0xf2e184,0xf1f3f5,0xbfc8d6,0xdfb897,0x73cec2,0xc5adff,0xffbe83};
static const char *en[]={"Mint","Ocean","Purple","Pink","Amber","Coral","Aqua","Leaf","Indigo","Lemon","White","Silver","Sand","Jade","Lavender","Peach"};
static const char *vi[]={"Mint","Biển xanh","Tím","Hồng","Hổ phách","San hô","Xanh ngọc","Lá xanh","Chàm","Vàng chanh","Trắng","Bạc","Cát","Ngọc bích","Oải hương","Cam đào"};
static SDL_Surface *background;
static char signature[700];
static const char *wall_en[]={"None","Sea","Dusk","Forest","Imported photo","Aurora","Sunrise","Rose clouds","Indigo night","Jade hills","Desert","Retro sunset","Ocean waves","Starry sky","Lavender hills","Peach horizon","Rainy teal","Coffee hills","Winter","Lemon fields","Galaxy","Monochrome"};
static const char *wall_vi[]={"Không","Biển","Hoàng hôn","Rừng","Ảnh đã nhập","Cực quang","Bình minh","Mây hồng","Đêm chàm","Đồi ngọc","Sa mạc","Hoàng hôn retro","Sóng biển","Trời sao","Đồi oải hương","Chân trời đào","Mưa xanh","Đồi cà phê","Mùa đông","Đồng chanh","Thiên hà","Đơn sắc"};
const char *NxtAppearance_wallpaperName(int i){return Nxt_text(wall_en[i>=0&&i<22?i:0],wall_vi[i>=0&&i<22?i:0]);}
typedef struct {unsigned first,second,detail;int kind;} Wallpaper;
static const Wallpaper wallpapers[22]={
 {0},{0x173e58,0x4e9b91,0x97d9cf,0},{0x382755,0xba698c,0xf6c49f,1},{0x123a30,0x538264,0x82b595,2},{0},
 {0x092a41,0x447d79,0x94eec5,3},{0x594268,0xf1bb8c,0xffe2a7,1},{0x572e59,0xbd87ab,0xf0c7d7,0},
 {0x121c4c,0x635b8b,0xadaedb,4},{0x123e3f,0x62a18a,0x9ad2b5,2},{0x875738,0xe6c195,0xffdbab,2},
 {0x37263f,0xb65068,0xffb66f,5},{0x134657,0x2e8999,0x8cdddf,0},{0x101e3a,0x3e5a77,0xdae1ec,4},
 {0x493d77,0xa29bbf,0xccc5e6,2},{0x794b65,0xebc6a8,0xffe9c8,1},{0x173e48,0x3e6775,0x88afbd,6},
 {0x302421,0x94785b,0xcab193,2},{0x456078,0xadbdcc,0xebf6ff,2},{0x43572f,0xaeb66b,0xe0da9a,2},
 {0x201a42,0x6f3c80,0xd595c8,3},{0x222a30,0x7f8e98,0xc4cbd1,2}};
static SDL_Color from_rgb(unsigned rgb){return (SDL_Color){rgb>>16,(rgb>>8)&255,rgb&255,255};}
static void circle(SDL_Surface *s,int x,int y,int radius,SDL_Color c){for(int dy=-radius;dy<=radius;dy++){int dx=sqrt((double)radius*radius-(double)dy*dy);SDL_FillRect(s,&(SDL_Rect){x-dx,y+dy,dx*2+1,1},SDL_MapRGB(s->format,c.r,c.g,c.b));}}
static void preset_background(SDL_Surface *s,int mode){
    Wallpaper p=wallpapers[mode];SDL_Color a=from_rgb(p.first),b=from_rgb(p.second),c=from_rgb(p.detail);
    for(int y=0;y<s->h;y++){int t=y*255/s->h;SDL_Color v={(a.r*(255-t)+b.r*t)/255,(a.g*(255-t)+b.g*t)/255,(a.b*(255-t)+b.b*t)/255,255};SDL_FillRect(s,&(SDL_Rect){0,y,s->w,1},SDL_MapRGB(s->format,v.r,v.g,v.b));}
    if(p.kind==1||p.kind==5){circle(s,s->w*3/4,s->h*38/100,s->w/11,c);if(p.kind==5)for(int y=s->h*36/100;y<s->h*49/100;y+=s->h/45)SDL_FillRect(s,&(SDL_Rect){s->w*61/100,y,s->w*29/100,s->h/130+1},SDL_MapRGB(s->format,b.r,b.g,b.b));}
    if(p.kind==3||p.kind==4){for(int i=0;i<95;i++){int x=(i*127+31)%s->w,y=(i*83+17)%(s->h*3/4);circle(s,x,y,i%11==0?2:1,c);}}
    if(p.kind==3){for(int layer=0;layer<3;layer++)for(int x=0;x<s->w;x++){int y=s->h*(25+layer*12)/100+(int)(sin(x*6.28/s->w+layer)*s->h/12);SDL_Color v={(c.r*(3-layer)+b.r*layer)/3,(c.g*(3-layer)+b.g*layer)/3,(c.b*(3-layer)+b.b*layer)/3,55};NxtSkin_rect(s,(SDL_Rect){x,y,1,s->h/8},v,0);}}
    else if(p.kind==6){for(int i=0;i<70;i++){int x=(i*83+17)%s->w,y=(i*107+19)%s->h;SDL_FillRect(s,&(SDL_Rect){x,y,1,s->h/18},SDL_MapRGB(s->format,c.r,c.g,c.b));}}
    for(int layer=0;layer<3;layer++){SDL_Color v={(b.r*(3-layer)+a.r*(layer+1))/4,(b.g*(3-layer)+a.g*(layer+1))/4,(b.b*(3-layer)+a.b*(layer+1))/4,255};
        for(int x=0;x<s->w;x++){double t=(double)x/s->w;int y=s->h*(60+layer*13)/100+(int)((sin(t*6.28+layer+mode*.2)+(p.kind==2?sin(t*13.5+layer)*.45:0))*s->h/(p.kind==0?35:12));SDL_FillRect(s,&(SDL_Rect){x,y,1,s->h-y},SDL_MapRGB(s->format,v.r,v.g,v.b));}
    }
}
unsigned NxtAppearance_accent(void){return nxt.accent_color==16?(unsigned)nxt.accent_rgb:colors[(unsigned)nxt.accent_color<16?nxt.accent_color:0];}
void NxtAppearance_release(void){if(background)SDL_FreeSurface(background);background=NULL;signature[0]=0;}
void NxtAppearance_background(SDL_Surface *screen){
    SDL_Color base=NxtSkin_color(NXT_BG);SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,base.r,base.g,base.b));
    if(!nxt.wallpaper_mode||nxt.interface_style)return;
    char key[700];snprintf(key,sizeof(key),"%dx%d/%d/%d/%d,%d,%d/%s",screen->w,screen->h,nxt.wallpaper_mode,nxt.wallpaper_dim,base.r,base.g,base.b,nxt.wallpaper_path);
    if(strcmp(key,signature)){
        NxtAppearance_release();snprintf(signature,sizeof(signature),"%s",key);
        background=SDL_CreateRGBSurfaceWithFormat(0,screen->w,screen->h,32,SDL_PIXELFORMAT_ARGB8888);if(!background)return;
        SDL_FillRect(background,NULL,SDL_MapRGB(background->format,base.r,base.g,base.b));
        if(nxt.wallpaper_mode==4){SDL_Surface *image=IMG_Load(nxt.wallpaper_path);if(image){
            if(image->w==screen->w&&image->h==screen->h)SDL_BlitSurface(image,NULL,background,NULL);
            else{SDL_Surface *sized=NxtWallpaper_resample(image,screen->w,screen->h,true,strstr(nxt.wallpaper_path,"-hq2-p.png")?NXT_WP_PIXEL:NXT_WP_SMOOTH);if(sized){SDL_BlitSurface(sized,NULL,background,NULL);SDL_FreeSurface(sized);}}
            SDL_FreeSurface(image);
        }}else{
            preset_background(background,nxt.wallpaper_mode>0&&nxt.wallpaper_mode<22?nxt.wallpaper_mode:1);
        }
        SDL_Surface *veil=SDL_CreateRGBSurfaceWithFormat(0,screen->w,screen->h,32,SDL_PIXELFORMAT_ARGB8888);
        if(veil){SDL_FillRect(veil,NULL,SDL_MapRGBA(veil->format,base.r,base.g,base.b,255));SDL_SetSurfaceBlendMode(veil,SDL_BLENDMODE_BLEND);SDL_SetSurfaceAlphaMod(veil,nxt.wallpaper_dim*255/100);SDL_BlitSurface(veil,NULL,background,NULL);SDL_FreeSurface(veil);}
    }
    if(background)SDL_BlitSurface(background,NULL,screen,NULL);
}
static bool image_size(const char *path){
    FILE *f=fopen(path,"rb");if(!f)return false;unsigned char b[32];uint32_t w=0,h=0;
    if(fread(b,1,24,f)==24&&!memcmp(b,"\x89PNG\r\n\x1a\n",8)){w=((uint32_t)b[16]<<24)|((uint32_t)b[17]<<16)|((uint32_t)b[18]<<8)|b[19];h=((uint32_t)b[20]<<24)|((uint32_t)b[21]<<16)|((uint32_t)b[22]<<8)|b[23];}
    else{rewind(f);if(fgetc(f)==255&&fgetc(f)==216){for(int i=0;i<2000&&!w;i++){
        int x;do{x=fgetc(f);}while(x!=255&&x!=EOF);if(x==EOF)break;do{x=fgetc(f);}while(x==255);if(x==EOF||x==217||x==218)break;if(x==1||(x>=208&&x<=215))continue;
        int a=fgetc(f),c=fgetc(f);if(a<0||c<0)break;int length=(a<<8)|c;if(length<2)break;
        if((x>=192&&x<=195)||(x>=197&&x<=199)||(x>=201&&x<=203)||(x>=205&&x<=207)){if(fread(b,1,5,f)==5){h=(b[1]<<8)|b[2];w=(b[3]<<8)|b[4];}break;}if(fseek(f,length-2,SEEK_CUR))break;
    }}}fclose(f);return w>0&&h>0&&w<=8192&&h<=8192&&(long long)w*h<=16000000;
}
static bool import_wallpaper(const char *path){
    struct stat st;if(lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size>10*1024*1024||!image_size(path)){NxtLog_step("appearance","wallpaper-input",false,"Require JPG/PNG <= 10 MiB and <= 16 MP");return false;}
    SDL_Surface *image=IMG_Load(path);if(!image)return false;double scale=1.;if(image->w>1024)scale=1024./image->w;if(image->h*scale>768)scale=768./image->h;
    int w=image->w*scale,h=image->h*scale;if(w<1)w=1;if(h<1)h=1;
    SDL_Surface *copy=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);bool ok=copy!=NULL;
    char rel[100],target[1200],temp[1220];snprintf(rel,sizeof(rel),"listeners/%d",NxtPersonal_id());userdata_mkdir(rel);snprintf(rel,sizeof(rel),"listeners/%d/background.png",NxtPersonal_id());userdata_snpath(rel,target,sizeof(target));
    if(strlen(target)>=sizeof(nxt.wallpaper_path))ok=false;snprintf(temp,sizeof(temp),"%s.tmp",target);
    if(ok){SDL_BlitScaled(image,NULL,copy,NULL);ok=IMG_SavePNG(copy,temp)==0&&!rename(temp,target);}if(copy)SDL_FreeSurface(copy);SDL_FreeSurface(image);
    if(ok){snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",target);nxt.wallpaper_mode=4;}else unlink(temp);
    NxtLog_step("appearance","wallpaper-import",ok,"Imported resized image");NxtAppearance_release();return ok;
}
static ModuleExitReason rgb_editor(DisplayContext *d){
    unsigned original=NxtAppearance_accent();int oldcolor=nxt.accent_color,oldrgb=nxt.accent_rgb;unsigned rgb=original;int selected=0;nxt.accent_color=16;
    for(;;){nxt.accent_rgb=rgb;char text[5][160];const char *rows[5];const char *names[]={Nxt_text("Red","Đỏ"),Nxt_text("Green","Xanh lá"),Nxt_text("Blue","Xanh dương")};
        for(int i=0;i<3;i++){snprintf(text[i],sizeof(text[i]),"%s\t%d",names[i],(rgb>>(16-i*8))&255);rows[i]=text[i];}
        snprintf(text[3],sizeof(text[3]),"HEX\t#%06X",rgb);rows[3]=text[3];rows[4]=Nxt_text("Save this color","Lưu màu này");
        NxtPick p=Nxt_pick(d,Nxt_text("Custom RGB","RGB tùy chỉnh"),rows,5,&selected,Nxt_text("←/→ ±1 · L1/R1 ±16 · A Save · B Cancel","←/→ ±1 · L1/R1 ±16 · A Lưu · B Hủy"));
        if(p.action==2||p.action==9){nxt.accent_color=oldcolor;nxt.accent_rgb=oldrgb;return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
        if(p.action==1&&selected!=3){nxt.accent_rgb=rgb;Nxt_save();NxtLog_step("appearance","rgb",true,"#%06X",rgb);return MODULE_EXIT_TO_MENU;}
        if(p.action==1&&selected==3){char *input=Keyboard_openLatin("HEX RRGGBB",7);if(input){char *s=input;if(*s=='#')s++;bool valid=strlen(s)==6;for(int i=0;valid&&i<6;i++)valid=isxdigit((unsigned char)s[i]);if(valid)rgb=strtoul(s,NULL,16);else Toast_show(Nxt_text("Use 6 hex digits","Nhập 6 ký tự HEX"),2000);free(input);}}
        if(selected<3&&(p.action==5||p.action==6||p.action==11||p.action==12)){int shift=16-selected*8,value=(rgb>>shift)&255;value+=(p.action==5?-1:p.action==6?1:p.action==11?-16:16);if(value<0)value=0;if(value>255)value=255;rgb=(rgb&~(255u<<shift))|((unsigned)value<<shift);}
    }
}
SDL_Surface *NxtAppearance_thumbnail(int mode){
    if(mode<0||mode>=22)return NULL;
    SDL_Surface *s=SDL_CreateRGBSurfaceWithFormat(0,240,152,32,SDL_PIXELFORMAT_ARGB8888);if(!s)return NULL;
    if(mode==0||mode==4){SDL_Color c=NxtSkin_color(NXT_BG);SDL_FillRect(s,NULL,SDL_MapRGB(s->format,c.r,c.g,c.b));}
    else preset_background(s,mode);return s;
}
ModuleExitReason NxtAppearance_wallpapers(DisplayContext *d){return NxtWallpapers_run(d);}
/* Preview uses the same cached vinyl renderer as the player. No audio restart. */
ModuleExitReason NxtAppearance_album(DisplayContext *d){
 int old=nxt.cover_mode,show=0;uint32_t drawn=0;for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);bool dirty=g.dirty;
 if(g.should_quit){nxt.cover_mode=old;return MODULE_EXIT_QUIT;}
 if(!g.input_consumed&&!Nxt_lockInput()){
 if(NxtPad_justPressed(BTN_B)){nxt.cover_mode=old;NxtRecord_hide();return MODULE_EXIT_TO_MENU;}
 if(NxtPad_justPressed(BTN_A)){Nxt_save();NxtRecord_hide();return MODULE_EXIT_TO_MENU;}
 if(NxtPad_justRepeated(BTN_LEFT)||NxtPad_justRepeated(BTN_RIGHT)){nxt.cover_mode=!nxt.cover_mode;dirty=true;}}
 if(NxtSkin_shouldDraw(drawn,dirty)){
 NxtSkin_begin(s,Nxt_text("Album image","Ảnh album"),nxt.cover_mode?Nxt_text("Vinyl","Đĩa than"):Nxt_text("Album artwork","Ảnh bìa"),false);
 SDL_Surface *art=NxtPlayback_art();if(!art&&!NxtSpotify_hasMetadata()&&!NxtSoundCloud_hasMetadata())art=album_art_get();int u=s->w;SDL_Rect box={337*u/1024,158*u/1024,350*u/1024,350*u/1024};
 if(nxt.cover_mode)NxtRecord_draw(s,box,art,NxtPlayback_generation(),false);else {NxtSkin_rect(s,box,NxtSkin_color(NXT_PANEL),12*u/1024);if(art){int w=box.w,h=box.h;if(art->w>art->h)h=(int)((long long)w*art->h/art->w);else w=(int)((long long)h*art->w/art->h);SDL_BlitScaled(art,NULL,s,&(SDL_Rect){box.x+(box.w-w)/2,box.y+(box.h-h)/2,w,h});}else NxtSkin_icon(s,"music-2",box.x+box.w/3,box.y+box.h/3,box.w/3,NxtSkin_color(NXT_ACCENT));}
 NxtSkin_fitText(s,26,nxt.cover_mode?Nxt_text("Vinyl","Đĩa than"):Nxt_text("Album artwork","Ảnh bìa"),64*u/1024,545*u/1024,896*u/1024,40*u/1024,NxtSkin_color(NXT_INK),true);
 NxtSkin_footer(s,Nxt_text("←→ Preview · A Apply · B Cancel","←→ Xem trước · A Áp dụng · B Hủy"),false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(s);
 }}
static ModuleExitReason NxtAppearance_readability(DisplayContext *);
ModuleExitReason NxtAppearance_original(DisplayContext *d){
 int selected=0;for(;;){char lines[16][620];const char *rows[16];const char *themes[]={Nxt_text("System","Theo hệ thống"),Nxt_text("Dark","Tối"),Nxt_text("Light","Sáng")};
#define R(i,en,vi,fmt,value) snprintf(lines[i],sizeof(lines[i]),"%s\t" fmt,Nxt_text(en,vi),value)
 R(0,"Light / dark appearance","Giao diện sáng / tối","%s",themes[nxt.theme]);R(1,"Accent palette","Bảng màu","%s",nxt.accent_color<16?(nxt.language==1?vi:en)[nxt.accent_color]:"RGB");R(2,"Custom RGB / HEX","RGB / HEX tùy chỉnh","#%06X",NxtAppearance_accent());
 R(3,"Font family","Bộ font","%s",NxtText_fontName(nxt.interface_style==NXT_THEME_AURALIS&&nxt.auralis_reference_font?17:nxt.font_index));R(4,"Interface text","Chữ giao diện","%d%%",nxt.font_size);R(5,"Lyrics text","Chữ lời bài hát","%d px",nxt.lyric_size);R(6,"Wallpaper gallery","Kho hình nền","%s",NxtAppearance_wallpaperName(nxt.wallpaper_mode));R(7,"Wallpaper veil","Lớp phủ hình nền","%d%%",nxt.wallpaper_dim);R(8,"Menu opacity","Độ đậm ô menu","%d%%",nxt.panel_opacity);
 R(9,"Album image","Ảnh album","%s",nxt.cover_mode?Nxt_text("Vinyl","Đĩa than"):Nxt_text("Album artwork","Ảnh bìa"));R(10,"Player animation","Chuyển động giao diện","%s",nxt.animations?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));R(11,"Menu icons","Biểu tượng menu","%s",Nxt_text("Edit","Chỉnh sửa"));R(12,"Lyric delay","Độ trễ lời","%+d ms",nxt.lyric_offset);R(13,"Lyric transition","Hiệu ứng chuyển lời","%s",NxtLyricFX_name(nxt.lyric_effect));R(14,"Voice-timed karaoke","Karaoke theo giọng hát","%s",Nxt_text("Groq / offline LRC","Groq / LRC offline"));
 R(15,"Lyrics & song information","Lời & thông tin bài hát","%s",Nxt_text("Colors / weight / outline","Màu / đậm / viền"));
#undef R
 for(int i=0;i<16;i++)rows[i]=lines[i];NxtPick p=Nxt_pick(d,Nxt_text("Original theme options","Tùy chọn giao diện gốc"),rows,16,&selected,Nxt_text("A Edit · ←/→ Change · B Back","A Sửa · ←/→ Đổi · B Về"));if(p.action==2||p.action==9){Nxt_save();return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}if(p.action!=1&&p.action!=5&&p.action!=6)continue;int delta=p.action==5?-1:1;ModuleExitReason r=MODULE_EXIT_TO_MENU;
 if(selected==0)nxt.theme=(nxt.theme+3+delta)%3;
 else if(selected==1){if(p.action==1){int color=nxt.accent_color<16?nxt.accent_color:0;NxtPick cp=Nxt_pick(d,Nxt_text("Accent palette","Bảng màu"),nxt.language==1?vi:en,16,&color,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(cp.action==1)nxt.accent_color=color;if(cp.action==9)return MODULE_EXIT_QUIT;}else nxt.accent_color=(nxt.accent_color+16+delta)%16;}
 else if(selected==2&&p.action==1)r=rgb_editor(d);
 else if(selected==3){if(p.action==1)r=NxtAppearance_fonts(d);else {nxt.font_index=(nxt.font_index+NXT_FONT_COUNT+delta)%NXT_FONT_COUNT;nxt.auralis_reference_font=0;nxt.font_path[0]=0;Fonts_unload();Fonts_load();}}
 else if(selected==4){nxt.font_size+=delta*5;if(nxt.font_size>120)nxt.font_size=85;if(nxt.font_size<85)nxt.font_size=120;Fonts_unload();Fonts_load();}
 else if(selected==5){nxt.lyric_size+=delta*2;if(nxt.lyric_size>48)nxt.lyric_size=24;if(nxt.lyric_size<24)nxt.lyric_size=48;}
 else if(selected==6){if(p.action==1)r=NxtAppearance_wallpapers(d);else{nxt.wallpaper_mode=(nxt.wallpaper_mode+22+delta)%22;if(nxt.wallpaper_mode==4&&!nxt.wallpaper_path[0])nxt.wallpaper_mode=(nxt.wallpaper_mode+22+delta)%22;}}
 else if(selected==7)nxt.wallpaper_dim=(nxt.wallpaper_dim+100+delta*5)%100;
 else if(selected==8)nxt.panel_opacity=(nxt.panel_opacity+105+delta*5)%105;
 else if(selected==9){if(p.action==1)r=NxtAppearance_album(d);else nxt.cover_mode=!nxt.cover_mode;}
 else if(selected==10)nxt.animations=!nxt.animations;
 else if(selected==11&&p.action==1)r=NxtCustom_menu(d);
 else if(selected==15)r=NxtAppearance_readability(d);
 else if(selected==14)r=NxtKaraoke_settings(d);
 else if(selected==13)r=NxtLyricFX_settings(d);
 else if(selected==12){nxt.lyric_offset+=delta*100;if(nxt.lyric_offset>30000)nxt.lyric_offset=30000;if(nxt.lyric_offset< -30000)nxt.lyric_offset=-30000;}
 Nxt_save();if(r==MODULE_EXIT_QUIT)return r;
 }}
static ModuleExitReason auralis_options(DisplayContext *);
ModuleExitReason NxtAppearance_run(DisplayContext *d){
 int selected=0;for(;;){const char *rows[6];int action[6],n=0;
 rows[n]=Nxt_text("Theme collection","Bộ sưu tập giao diện");action[n++]=0;
 if(nxt.interface_style==NXT_THEME_AURALIS){rows[n]=Nxt_text("Auralis options","Tùy chỉnh Auralis");action[n++]=4;}
 rows[n]=Nxt_text("Original theme options","Tùy chọn giao diện gốc");action[n++]=1;
 rows[n]=Nxt_text("LEDs","Đèn LED");action[n++]=2;
 rows[n]=Nxt_text("Spectrum colors","Phổ nhạc");action[n++]=3;
 if(selected>=n)selected=n-1;NxtPick p=Nxt_pick(d,Nxt_text("Personal appearance","Giao diện cá nhân"),rows,n,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
 int a=action[selected];ModuleExitReason r=a==0?NxtTheme_gallery(d):a==1?NxtAppearance_original(d):a==2?Nxt_LEDSettings(d):a==3?NxtCustom_spectrum(d):a==4?auralis_options(d):NxtAppearance_fonts(d);if(r==MODULE_EXIT_QUIT)return r;
 }}

/* Fonts are shared by every theme. Preview changes stay in RAM until Apply. */
ModuleExitReason NxtAppearance_fonts(DisplayContext*d){return NxtLyricPreview_run(d,NXT_EDIT_FONT);}
static ModuleExitReason auralis_options(DisplayContext *d){
 if(nxt.interface_style!=NXT_THEME_AURALIS)return MODULE_EXIT_TO_MENU;
 int selected=0;for(;;){char a[180],b[180],fx[180],font[180];
  snprintf(a,sizeof(a),"%s\t%d",Nxt_text("Now Playing cover brightness","Độ sáng bìa Đang phát"),nxt.auralis_now_brightness);
  snprintf(b,sizeof(b),"%s\t%d",Nxt_text("Full lyrics cover brightness","Độ sáng bìa Full lời"),nxt.auralis_full_brightness);
  snprintf(fx,sizeof(fx),"%s\t%s",Nxt_text("Lyric transition","Hiệu ứng chuyển lời"),NxtLyricFX_name(nxt.lyric_effect));
  snprintf(font,sizeof(font),"%s\t%s",Nxt_text("Reference font","Font theo ảnh mẫu"),nxt.auralis_reference_font?Nxt_text("On · HarmonyOS Sans","Bật · HarmonyOS Sans"):Nxt_text("Use selected font","Dùng bộ font đã chọn"));
  const char *rows[]={a,b,fx,font,Nxt_text("Lyrics & song information","Lời & thông tin bài hát"),Nxt_text("Cover brightness is saved for this listener. Change the font family in Original theme options.","Độ sáng bìa lưu theo hồ sơ. Đổi bộ font trong Tùy chọn giao diện gốc.")};
  NxtPick p=Nxt_pick(d,Nxt_text("Auralis options","Tùy chỉnh Auralis"),rows,6,&selected,Nxt_text("←→ Adjust · A Open / +5 · B Back","←→ Chỉnh · A Mở / +5 · B Về"));
  if(p.action==9||p.action==2){Nxt_save();return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
  if(p.action!=1&&p.action!=5&&p.action!=6)continue;
  if(selected<2){int *value=selected?&nxt.auralis_full_brightness:&nxt.auralis_now_brightness;int low=selected?30:40,high=selected?90:95;*value+=p.action==5?-1:p.action==1?5:1;if(*value<low)*value=low;if(*value>high)*value=high;}
  else if(selected==2){if(p.action==1){if(NxtLyricFX_settings(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}else nxt.lyric_effect=(nxt.lyric_effect+(p.action==5?9:1))%10;}
  else if(selected==3)nxt.auralis_reference_font=!nxt.auralis_reference_font;
  else if(selected==4&&NxtAppearance_readability(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
  Nxt_save();
 }
}

/* One profile-scoped panel, reachable from either theme options page. */
static ModuleExitReason NxtAppearance_readability(DisplayContext*d){return NxtLyricPreview_run(d,NXT_EDIT_SUNG);}
