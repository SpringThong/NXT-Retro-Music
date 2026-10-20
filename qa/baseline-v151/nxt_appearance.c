/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_appearance.h"
#include "nxt_skin.h"
#include "nxt_personal.h"
#include "nxt_log.h"
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
    if(!nxt.wallpaper_mode)return;
    char key[700];snprintf(key,sizeof(key),"%dx%d/%d/%d/%d,%d,%d/%s",screen->w,screen->h,nxt.wallpaper_mode,nxt.wallpaper_dim,base.r,base.g,base.b,nxt.wallpaper_path);
    if(strcmp(key,signature)){
        NxtAppearance_release();snprintf(signature,sizeof(signature),"%s",key);
        background=SDL_CreateRGBSurfaceWithFormat(0,screen->w,screen->h,32,SDL_PIXELFORMAT_ARGB8888);if(!background)return;
        SDL_FillRect(background,NULL,SDL_MapRGB(background->format,base.r,base.g,base.b));
        if(nxt.wallpaper_mode==4){SDL_Surface *image=IMG_Load(nxt.wallpaper_path);if(image){
            double scale=(double)screen->w/image->w;if((double)screen->h/image->h>scale)scale=(double)screen->h/image->h;
            SDL_Rect target={(screen->w-(int)(image->w*scale))/2,(screen->h-(int)(image->h*scale))/2,(int)(image->w*scale),(int)(image->h*scale)};
            SDL_BlitScaled(image,NULL,background,&target);SDL_FreeSurface(image);
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
ModuleExitReason NxtAppearance_wallpapers(DisplayContext *d){
    SDL_Surface *thumbs[22]={0};int selected=nxt.wallpaper_mode,show=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;
    for(int i=0;i<22;i++){thumbs[i]=SDL_CreateRGBSurfaceWithFormat(0,240,152,32,SDL_PIXELFORMAT_ARGB8888);if(!thumbs[i])continue;
        if(i==0||i==4){SDL_Color c=NxtSkin_color(NXT_BG);SDL_FillRect(thumbs[i],NULL,SDL_MapRGB(thumbs[i]->format,c.r,c.g,c.b));if(i==4&&nxt.wallpaper_path[0]){SDL_Surface *img=IMG_Load(nxt.wallpaper_path);if(img){SDL_BlitScaled(img,NULL,thumbs[i],NULL);SDL_FreeSurface(img);}}}
        else preset_background(thumbs[i],i);
    }
    for(;;){ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);if(g.should_quit){result=MODULE_EXIT_QUIT;break;}
        if(!g.input_consumed&&!Nxt_lockInput()){
            if(NxtPad_justPressed(BTN_B))break;
            if(NxtPad_justRepeated(BTN_LEFT))selected=(selected+21)%22;
            if(NxtPad_justRepeated(BTN_RIGHT))selected=(selected+1)%22;
            if(NxtPad_justRepeated(BTN_UP))selected=(selected+18)%22;
            if(NxtPad_justRepeated(BTN_DOWN))selected=(selected+4)%22;
            if(NxtPad_justPressed(BTN_A)){if(selected==4&&!nxt.wallpaper_path[0]){Toast_show(Nxt_text("Import a photo first","Hãy nhập ảnh trước"),2000);}else{nxt.wallpaper_mode=selected;NxtAppearance_release();Nxt_save();break;}}
        }
        NxtSkin_begin(screen,Nxt_text("20 wallpapers & your photo","20 hình nền & ảnh của bạn"),NULL,false);int u=screen->w,first=selected/8*8;
        for(int i=first;i<first+8&&i<22;i++){int col=(i-first)%4,row=(i-first)/4;SDL_Rect r={(32+col*244)*u/1024,(156+row*210)*u/1024,228*u/1024,185*u/1024};
            NxtSkin_rect(screen,r,NxtSkin_color(i==selected?NXT_ACCENT:NXT_PANEL),10*u/1024);SDL_Rect image={r.x+5*u/1024,r.y+5*u/1024,r.w-10*u/1024,136*u/1024};if(thumbs[i])SDL_BlitScaled(thumbs[i],NULL,screen,&image);
            NxtSkin_text(screen,18,NxtAppearance_wallpaperName(i),r.x+12*u/1024,r.y+145*u/1024,r.w-24*u/1024,NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK));
        }
        NxtSkin_footer(screen,Nxt_text("D-pad Browse · A Apply · B Cancel","D-pad Chọn · A Áp dụng · B Hủy"),false);ModuleCommon_markSurfaceDrawn();ModuleCommon_frameEnd(screen);
    }
    for(int i=0;i<22;i++)if(thumbs[i])SDL_FreeSurface(thumbs[i]);return result;
}
ModuleExitReason NxtAppearance_run(DisplayContext *d){
    int selected=0;for(;;){char lines[13][620];const char *rows[13];const char *themes[]={Nxt_text("System","Theo hệ thống"),Nxt_text("Dark","Tối"),Nxt_text("Light","Sáng")};
#define R(i,en,vi,fmt,value) snprintf(lines[i],sizeof(lines[i]),"%s\t" fmt,Nxt_text(en,vi),value)
        R(0,"Theme","Giao diện","%s",themes[nxt.theme]);R(1,"Accent palette","Bảng màu","%s",nxt.accent_color<16?(nxt.language?vi:en)[nxt.accent_color]:"RGB");R(2,"Custom RGB / HEX","RGB / HEX tùy chỉnh","#%06X",NxtAppearance_accent());
        R(3,"Wallpaper gallery","Chọn hình nền","%s",NxtAppearance_wallpaperName(nxt.wallpaper_mode));R(4,"Import JPG / PNG","Nhập ảnh JPG / PNG","%s",Nxt_text("From SD card","Từ thẻ nhớ"));R(5,"Wallpaper veil","Lớp phủ hình nền","%d%%",nxt.wallpaper_dim);R(6,"Menu opacity","Độ đậm ô menu","%d%%",nxt.panel_opacity);
        R(7,"Font family","Bộ font","%s",NxtText_fontName(nxt.font_index));R(8,"Interface text","Chữ giao diện","%d%%",nxt.font_size);R(9,"Lyrics text","Chữ lời bài hát","%d px",nxt.lyric_size);R(10,"Lyric delay","Độ trễ lời","%+d ms",nxt.lyric_offset);R(11,"Language","Ngôn ngữ","%s",nxt.language?"Tiếng Việt":"English");R(12,"Listener","Hồ sơ đang dùng","%s",NxtPersonal_name());
#undef R
        for(int i=0;i<13;i++)rows[i]=lines[i];NxtPick p=Nxt_pick(d,Nxt_text("Personal appearance","Giao diện cá nhân"),rows,13,&selected,Nxt_text("A Edit · ←/→ Change · B Back","A Sửa · ←/→ Đổi · B Về"));if(p.action==2||p.action==9){Nxt_save();return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}if(p.action!=1&&p.action!=5&&p.action!=6)continue;int delta=p.action==5?-1:1;ModuleExitReason r=MODULE_EXIT_TO_MENU;
        if(selected==0)nxt.theme=(nxt.theme+3+delta)%3;
        else if(selected==1){if(p.action==1){int color=nxt.accent_color<16?nxt.accent_color:0;NxtPick cp=Nxt_pick(d,Nxt_text("16 colors","16 màu giao diện"),nxt.language?vi:en,16,&color,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(cp.action==1)nxt.accent_color=color;if(cp.action==9)return MODULE_EXIT_QUIT;}else nxt.accent_color=(nxt.accent_color+16+delta)%16;}
        else if(selected==2&&p.action==1)r=rgb_editor(d);
        else if(selected==3){if(p.action==1)r=NxtAppearance_wallpapers(d);else{nxt.wallpaper_mode=(nxt.wallpaper_mode+22+delta)%22;if(nxt.wallpaper_mode==4&&!nxt.wallpaper_path[0])nxt.wallpaper_mode=(nxt.wallpaper_mode+22+delta)%22;}}
        else if(selected==4&&p.action==1){char path[512];if(Nxt_chooseFile(d,".jpg",".png",path))Toast_show(import_wallpaper(path)?Nxt_text("Wallpaper saved","Đã lưu hình nền"):Nxt_text("Use JPG/PNG <=10 MiB and <=16 MP","Dùng JPG/PNG ≤10 MiB và ≤16 MP"),3000);}
        else if(selected==5)nxt.wallpaper_dim=(nxt.wallpaper_dim+100+delta*5)%100;
        else if(selected==6)nxt.panel_opacity=(nxt.panel_opacity+105+delta*5)%105;
        else if(selected==7){if(p.action==1){const char *fonts[16];for(int i=0;i<16;i++)fonts[i]=NxtText_fontName(i);int f=nxt.font_index;NxtPick fp=Nxt_pick(d,Nxt_text("Choose font","Chọn font"),fonts,16,&f,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(fp.action==1)nxt.font_index=f;if(fp.action==9)return MODULE_EXIT_QUIT;}else nxt.font_index=(nxt.font_index+16+delta)%16;nxt.font_path[0]=0;Fonts_unload();Fonts_load();}
        else if(selected==8){nxt.font_size+=delta*5;if(nxt.font_size>120)nxt.font_size=85;if(nxt.font_size<85)nxt.font_size=120;Fonts_unload();Fonts_load();}
        else if(selected==9){nxt.lyric_size+=delta*2;if(nxt.lyric_size>48)nxt.lyric_size=24;if(nxt.lyric_size<24)nxt.lyric_size=48;}
        else if(selected==10){nxt.lyric_offset+=delta*100;if(nxt.lyric_offset>30000)nxt.lyric_offset=30000;if(nxt.lyric_offset< -30000)nxt.lyric_offset=-30000;}
        else if(selected==11)nxt.language=!nxt.language;
        Nxt_save();if(r==MODULE_EXIT_QUIT)return r;
    }
}
