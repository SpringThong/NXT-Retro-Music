/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_skin.h"
#include "nxt_eq.h"
#include "nxt_controls.h"
#include "api.h"
#include "background.h"
#include "toast.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
extern ModuleExitReason Nxt_dspAdvanced(DisplayContext *);
static bool eq_open,eq_close;
bool NxtEQ_scope(void){return eq_open;}
bool NxtEQ_closeRequested(void){return eq_open&&eq_close;}
void NxtEQ_requestClose(void){if(eq_open)eq_close=true;}
static bool eq_exit(void){bool select=NxtControl_pressed(NC_EQ)||NxtPad_justPressed(BTN_SELECT);if(select)NxtEQ_requestClose();return NxtEQ_closeRequested()||select||NxtPad_justPressed(BTN_B);}
static ModuleExitReason eq_options(DisplayContext *);
static ModuleExitReason eq_graph(DisplayContext *);
void Nxt_EQBands(int bands){
 if(bands==nxt.eq_bands)return;float old[31];memcpy(old,nxt.eq,sizeof(old));int previous=nxt.eq_bands;memset(nxt.eq,0,sizeof(nxt.eq));
 for(int i=0;i<bands;i++){float x=(float)i*(previous-1)/(bands-1);int k=(int)x;float part=x-k;nxt.eq[i]=k+1<previous?old[k]*(1-part)+old[k+1]*part:old[previous-1];}nxt.eq_bands=bands;
}
static const char *quick_names[]={"Flat","Pop","Rock","Jazz","Classical","Vocal","EDM","Acoustic","Bass Boost","Treble","Lo-Fi","Loudness"};
static const float quick_gain[12][5]={{0,0,0,0,0},{2,1,-1,2,2},{3,1,-2,2,3},{2,0,-1,2,1},{0,0,0,1,2},{-2,-1,3,2,0},{4,2,-2,2,3},{1,1,1,2,1},{6,3,0,0,0},{-1,0,0,3,5},{2,1,-1,-3,-5},{4,2,-1,2,4}};
float NxtEQ_frequency(int bands,int band){static const float five[]={60,250,1000,4000,12000};return bands==5?five[band<0?0:band>4?4:band]:31.25f*powf(640.f,(float)band/(bands-1));}
const char *NxtEQ_name(int i){return i>=0&&i<12?quick_names[i]:Nxt_text("Custom","Tùy chỉnh");}
void NxtEQ_apply(int i){if(i<0||i>=12)return;nxt.eq_bands=5;memset(nxt.eq,0,sizeof(nxt.eq));memcpy(nxt.eq,quick_gain[i],sizeof(quick_gain[i]));nxt.eq_preset=i;nxt.preamp=0;for(int k=0;k<5;k++)if(nxt.eq[k]>-nxt.preamp)nxt.preamp=-nxt.eq[k];nxt.dsp=1;nxt.bass=nxt.treble=0;for(int k=0;k<6;k++)nxt.peq[k].enabled=0;Nxt_save();}
static const char *quick_icons[]={"eq-flat","eq-pop","eq-rock","eq-jazz","eq-classical","eq-vocal","eq-edm","eq-acoustic","eq-bass","eq-treble","eq-lofi","eq-loudness","sliders-horizontal"};
void NxtEQ_drawPresets(SDL_Surface *s,int selected){
 int u=s->w;NxtSkin_begin(s,Nxt_text("Choose your EQ","Chọn EQ của bạn"),NxtEQ_name(nxt.eq_preset),false);
 const char *tabs[]={Nxt_text("Presets","Bộ chỉnh sẵn"),Nxt_text("Custom EQ","Tùy chỉnh EQ"),Nxt_text("Advanced","Nâng cao")};
 NxtSkin_tabs(s,tabs,3,0,138*u/1024);
 for(int i=0;i<13;i++){int col=i%4,row=i/4,x=32+col*244,y=198+row*112;
  SDL_Color c=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);
  NxtSkin_rect(s,(SDL_Rect){x*u/1024,y*u/1024,229*u/1024,101*u/1024},NxtSkin_color(i==selected?NXT_ACCENT:NXT_PANEL),11*u/1024);
  NxtSkin_icon(s,quick_icons[i],(x+91)*u/1024,(y+10)*u/1024,45*u/1024,c);
  NxtSkin_text(s,21,NxtEQ_name(i),(x+16)*u/1024,(y+60)*u/1024,197*u/1024,c);
  if(i==nxt.eq_preset)NxtSkin_icon(s,"check",(x+199)*u/1024,(y+9)*u/1024,20*u/1024,c);
 }
 NxtSkin_footer(s,Nxt_text("D-pad Select · A Apply · SELECT / B Close · R2 Custom","D-pad Chọn · A Áp dụng · SELECT / B Đóng · R2 Tùy chỉnh"),false);
}
void NxtEQ_drawCustom(SDL_Surface *s,int selected){int u=s->w;char status[180];snprintf(status,sizeof(status),"5 %s · Preamp %+.1f dB · %s%s",Nxt_text("bands","dải"),nxt.preamp,nxt.dsp?"DSP":"Bypass",NxtDSP_clipped()?" · CLIP":"");NxtSkin_begin(s,Nxt_text("Custom EQ","EQ tùy chỉnh"),NULL,true);NxtSkin_text(s,21,status,32*u/1024,151*u/1024,960*u/1024,NxtSkin_color(NXT_MUTED));int top=220,bottom=510;for(int db=-12;db<=12;db+=6){int y=(top+bottom)/2-db*(bottom-top)/24;NxtSkin_rect(s,(SDL_Rect){60*u/1024,y*u/1024,906*u/1024,1},NxtSkin_color(NXT_LINE),0);}for(int i=0;i<5;i++){int x=123+i*195,y=(top+bottom)/2-(int)(nxt.eq[i]*(bottom-top)/24);NxtSkin_rect(s,(SDL_Rect){x*u/1024,top*u/1024,13*u/1024,(bottom-top)*u/1024},NxtSkin_color(NXT_PANEL),5*u/1024);NxtSkin_rect(s,(SDL_Rect){x*u/1024,y*u/1024,13*u/1024,(bottom-y)*u/1024},NxtSkin_color(i==selected?NXT_ACCENT:NXT_MUTED),5*u/1024);NxtSkin_rect(s,(SDL_Rect){(x-19)*u/1024,(y-9)*u/1024,51*u/1024,18*u/1024},NxtSkin_color(i==selected?NXT_ACCENT:NXT_INK),7*u/1024);char t[60];snprintf(t,sizeof(t),"%+.1f dB",nxt.eq[i]);NxtSkin_text(s,24,t,(x-57)*u/1024,183*u/1024,157*u/1024,NxtSkin_color(i==selected?NXT_ACCENT:NXT_INK));float hz=NxtEQ_frequency(5,i);if(hz>=1000)snprintf(t,sizeof(t),"%.0f kHz",hz/1000);else snprintf(t,sizeof(t),"%.0f Hz",hz);NxtSkin_text(s,22,t,(x-44)*u/1024,530*u/1024,143*u/1024,NxtSkin_color(NXT_MUTED));}NxtSkin_footer(s,Nxt_text("←→ Band · ↑↓ Gain · L1/R1 Preamp · X DSP · START Options · B Save","←→ Dải · ↑↓ Gain · L1/R1 Preamp · X DSP · START Tùy chọn · B Lưu"),false);}
static ModuleExitReason custom_five(DisplayContext *d){Nxt_EQBands(5);nxt.eq_preset=12;Nxt_save();int selected=0,show=0;uint32_t drawn=0;for(;;){ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);bool dirty=g.dirty,changed=false;if(g.should_quit)return MODULE_EXIT_QUIT;if(!g.input_consumed&&!Nxt_lockInput()){if(eq_exit()){Nxt_save();return MODULE_EXIT_TO_MENU;}if(!!NxtPad_justRepeated(BTN_LEFT)){selected=(selected+4)%5;dirty=true;}if(!!NxtPad_justRepeated(BTN_RIGHT)){selected=(selected+1)%5;dirty=true;}int delta=!!NxtPad_justRepeated(BTN_UP)-!!NxtPad_justRepeated(BTN_DOWN);if(delta){nxt.eq[selected]=fmaxf(-12,fminf(12,nxt.eq[selected]+delta*.5f));changed=true;}delta=!!NxtPad_justRepeated(BTN_R1)-!!NxtPad_justRepeated(BTN_L1);if(delta){nxt.preamp=fmaxf(-24,fminf(12,nxt.preamp+delta*.5f));changed=true;}if(NxtPad_justPressed(BTN_X)){nxt.dsp=!nxt.dsp;changed=true;}if(NxtPad_justPressed(BTN_START)){if(eq_options(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;Nxt_EQBands(5);drawn=0;}if(changed)Nxt_save();}if(NxtSkin_shouldDraw(drawn,dirty||changed)){NxtEQ_drawCustom(s,selected);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(s);}}
static ModuleExitReason dsp_screen(DisplayContext *d){if(nxt.bit_perfect){const char *rows[]={Nxt_text("EQ is bypassed in Bit-Perfect. Your saved preset is retained.","EQ được bỏ qua trong Bit-Perfect. Giữ nguyên bộ EQ đã lưu."),Nxt_text("Choose Normal mode to edit EQ. SELECT or B closes this screen.","Chọn chế độ Thường để sửa EQ. SELECT hoặc B đóng màn hình.")};int selected=0;for(;;){NxtPick p=Nxt_pick(d,"Bit-Perfect · EQ",rows,2,&selected,Nxt_text("SELECT / B Close","SELECT / B Đóng"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2||p.action==7)return MODULE_EXIT_TO_MENU;}}int selected=nxt.eq_preset,show=0;uint32_t drawn=0;for(;;){if(NxtEQ_closeRequested())return MODULE_EXIT_TO_MENU;int old=selected;ModuleCommon_frameBegin();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit)return MODULE_EXIT_QUIT;if(!g.input_consumed&&!Nxt_lockInput()){if(eq_exit())return MODULE_EXIT_TO_MENU;if(!!NxtPad_justRepeated(BTN_LEFT))selected=(selected+12)%13;if(!!NxtPad_justRepeated(BTN_RIGHT))selected=(selected+1)%13;if(!!NxtPad_justRepeated(BTN_UP))selected=(selected+9)%13;if(!!NxtPad_justRepeated(BTN_DOWN))selected=(selected+4)%13;if(NxtPad_justPressed(BTN_A)){if(selected<12){NxtEQ_apply(selected);return MODULE_EXIT_TO_MENU;}if(custom_five(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;drawn=0;}if(NxtPad_justPressed(BTN_R2)){if(custom_five(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;drawn=0;}if(NxtPad_justPressed(BTN_START)){if(eq_graph(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;drawn=0;}}if(NxtSkin_shouldDraw(drawn,g.dirty||old!=selected)){NxtEQ_drawPresets(s,selected);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(s);}}
static const char *presets[]={"Flat","Pop","Rock","Jazz","Classical","Vocal","EDM"};
static const float gains[7][10]={
 {0,0,0,0,0,0,0,0,0,0},{1,2,2,0,-1,-1,1,2,2,1},{3,2,1,-1,-2,-1,1,2,3,2},
 {2,1,0,0,-1,0,1,2,2,1},{0,0,0,0,0,0,1,1,1,1},{-2,-2,-1,0,2,3,3,1,0,-1},{4,3,2,0,-2,-1,1,2,3,2}};
static void preset(int p){for(int i=0;i<nxt.eq_bands;i++){float x=(float)i*9/(nxt.eq_bands-1);int k=(int)x;nxt.eq[i]=k<9?gains[p][k]+(gains[p][k+1]-gains[p][k])*(x-k):gains[p][9];}nxt.preamp=p?-4:0;nxt.dsp=1;for(int i=0;i<6;i++)nxt.peq[i].enabled=0;nxt.bass=nxt.treble=0;Nxt_save();}
static ModuleExitReason eq_options(DisplayContext *d){int sel=0;for(;;){char mode[100],pre[100],lim[100];snprintf(mode,100,"DSP\t%s",nxt.dsp?Nxt_text("On","Bật"):Nxt_text("Bypass","Bỏ qua"));snprintf(pre,100,"Preamp\t%+.1f dB",nxt.preamp);snprintf(lim,100,"Limiter\t%s",nxt.limiter?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));const char *rows[]={mode,Nxt_text("Built-in presets","Preset có sẵn"),pre,lim,Nxt_text("Advanced / parametric / save / import","Nâng cao / EQ tham số / lưu / nhập"),Nxt_text("Reset to Flat","Khôi phục Flat"),Nxt_text("Custom presets · save / rename / import / export","Preset riêng · lưu / đổi tên / nhập / xuất")};NxtPick p=Nxt_pick(d,"EQ / DSP",rows,7,&sel,Nxt_text("A Open • ←→ Adjust • B Back","A Mở • ←→ Chỉnh • B Quay lại"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1&&p.action!=5&&p.action!=6)continue;int delta=p.action==5?-1:1;if(sel==0)nxt.dsp=!nxt.dsp;if(sel==1){int f=0;NxtPick fp=Nxt_pick(d,Nxt_text("Presets","Preset"),presets,7,&f,Nxt_text("A Apply • B Back","A Áp dụng • B Quay lại"));if(fp.action==1)preset(f);}if(sel==2)nxt.preamp=fmaxf(-24,fminf(12,nxt.preamp+delta*.5f));if(sel==3)nxt.limiter=!nxt.limiter;if(sel==4){if(Nxt_dspAdvanced(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}if(sel==5)preset(0);if(sel==6&&Nxt_presets(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;Nxt_save();}}
static ModuleExitReason eq_graph(DisplayContext *d){int sel=0,show=0;uint32_t redraw=0;
 for(;;){ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(g.should_quit)return MODULE_EXIT_QUIT;if(!g.input_consumed&&!Nxt_lockInput()){
 if(eq_exit())return MODULE_EXIT_TO_MENU;
 if(!!NxtPad_justRepeated(BTN_LEFT))sel=(sel+nxt.eq_bands-1)%nxt.eq_bands;
 if(!!NxtPad_justRepeated(BTN_RIGHT))sel=(sel+1)%nxt.eq_bands;
 if(!!NxtPad_justRepeated(BTN_UP)){nxt.eq[sel]=fminf(12,nxt.eq[sel]+.5f);Nxt_save();}
 if(!!NxtPad_justRepeated(BTN_DOWN)){nxt.eq[sel]=fmaxf(-12,nxt.eq[sel]-.5f);Nxt_save();}
 if(NxtPad_justPressed(BTN_X)){nxt.dsp=!nxt.dsp;Nxt_save();}
 if(NxtPad_justPressed(BTN_Y)){Nxt_EQBands(nxt.eq_bands==10?15:nxt.eq_bands==15?31:10);sel=0;Nxt_save();}
 if(NxtPad_justPressed(BTN_L1)){nxt.preamp=fmaxf(-24,nxt.preamp-.5f);Nxt_save();}
 if(NxtPad_justPressed(BTN_R1)){nxt.preamp=fminf(12,nxt.preamp+.5f);Nxt_save();}
 if(NxtPad_justPressed(BTN_A)){if(eq_options(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
 }
 if(NxtSkin_shouldDraw(redraw,g.dirty||PAD_anyPressed())){
  int u=s->w;NxtSkin_begin(s,"EQ / DSP",NULL,true);int top=NxtSkin_top(s),bottom=NxtSkin_bottom(s,true),left=u/24,right=u-left;
  char status[240];snprintf(status,sizeof(status),"%d %s · Preamp %+.1f dB · %s%s",nxt.eq_bands,Nxt_text("bands","dải"),nxt.preamp,nxt.dsp?Nxt_text("Active","Đang bật"):"Bypass",NxtDSP_clipped()?" · CLIP":"");NxtSkin_text(s,22,status,left,top,right-left,NxtSkin_color(NXT_MUTED));
  int charttop=top+55*u/1024,chartbottom=bottom-76*u/1024,center=(charttop+chartbottom)/2;float pixels=(chartbottom-charttop)/28.f;
  for(int db=-12;db<=12;db+=6){int y=center-db*pixels;SDL_FillRect(s,&(SDL_Rect){left,y,right-left,1},SDL_MapRGB(s->format,NxtSkin_color(NXT_LINE).r,NxtSkin_color(NXT_LINE).g,NxtSkin_color(NXT_LINE).b));}
  for(int i=0;i<nxt.eq_bands;i++){int x=left+(right-left)*(i+.5f)/nxt.eq_bands,w=(right-left)/nxt.eq_bands-5*u/1024;if(w<3)w=3;int y=center-nxt.eq[i]*pixels;SDL_Color c=NxtSkin_color(i==sel?NXT_ACCENT:NXT_MUTED);NxtSkin_rect(s,(SDL_Rect){x-w/2,y<center?y:center,w,abs(y-center)+2},c,3);if(i==sel)NxtSkin_rect(s,(SDL_Rect){x-7*u/1024,y-7*u/1024,14*u/1024,14*u/1024},NxtSkin_color(NXT_ACCENT),7);}
  for(int x=left;x<right;x++){float hz=31.25f*powf(640.f,(float)(x-left)/(right-left)),db=NxtDSP_response(hz);int y=center-db*pixels;if(y>=charttop&&y<chartbottom)SDL_FillRect(s,&(SDL_Rect){x,y,2,2},SDL_MapRGB(s->format,157,228,182));}
  char band[100];snprintf(band,100,"%.0f Hz   %+.1f dB",NxtEQ_frequency(nxt.eq_bands,sel),nxt.eq[sel]);NxtSkin_text(s,27,band,left,chartbottom+15*u/1024,right-left,NxtSkin_color(NXT_ACCENT));
  NxtSkin_footer(s,Nxt_text("←→ Band • ↑↓ Gain • Y Bands • X DSP • L1/R1 Preamp • A Options • B Back","←→ Dải • ↑↓ Gain • Y Số dải • X DSP • L1/R1 Preamp • A Tùy chọn • B Quay lại"),false);ModuleCommon_markSurfaceDrawn();redraw=SDL_GetTicks();
 }ModuleCommon_frameEnd(s);
 }
}

ModuleExitReason Nxt_dspScreen(DisplayContext *d){eq_open=true;eq_close=false;ModuleExitReason result=dsp_screen(d);eq_open=eq_close=false;return result;}
