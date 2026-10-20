/* SPDX-License-Identifier: GPL-3.0-only
 * Device-wide visual work and reversible NextUI CPUFreq policy. Never changes
 * audio callbacks, DAC precision, input polling or thermal safeguards. */
#include "nxt_performance.h"
#include "nxt_cpu.h"
#include "nxt_monitor.h"
#include "nxt_skin.h"
#include "ui_music_playing.h"
#include "toast.h"
#include <stdio.h>
static int mode(void){return nxt.performance_mode>=0&&nxt.performance_mode<=2?nxt.performance_mode:NXT_PERFORMANCE_BALANCED;}
const char *NxtPerformance_name(int value){
 return value==NXT_PERFORMANCE_BATTERY?Nxt_text("Battery saver","Tiết kiệm pin"):value==NXT_PERFORMANCE_FAST?Nxt_text("Performance","Hiệu năng"):Nxt_text("Balanced","Cân bằng");
}
uint32_t NxtPerformance_frameMS(bool playing,bool mechanical,int style){
 if(!playing)return 1000;
 if(nxt.animations&&mechanical)return 16; /* target display cadence only while moving */
 return style==22?1000:250;
}
/* Keep FFT costs separate from animation and the CPU/thermal policy. */
uint32_t NxtPerformance_spectrumMS(void){return mode()==NXT_PERFORMANCE_BATTERY?100:mode()==NXT_PERFORMANCE_BALANCED?50:33;}
uint32_t NxtPerformance_recordMS(void){return nxt.animations?16:1000;}
uint32_t NxtPerformance_previewMS(void){return nxt.animations?16:1000;}
static ModuleExitReason choose_mode(DisplayContext *d){
 int selected=mode();for(;;){char line[3][240];const char *rows[3];
 const char *detail[]={Nxt_text("Lighter effects; prioritize battery life","Giảm hiệu ứng, ưu tiên thời lượng pin"),Nxt_text("Smooth everyday listening","Mượt và nhẹ cho nghe nhạc hằng ngày"),Nxt_text("Smoother motion; uses more battery","Chuyển động mượt hơn, tốn pin hơn")};
 for(int i=0;i<3;i++){snprintf(line[i],sizeof(line[i]),"%s%s\n%s",i==mode()?"✓  ":"",NxtPerformance_name(i),detail[i]);rows[i]=line[i];}
 NxtPick p=Nxt_pick(d,Nxt_text("Performance mode","Chế độ hiệu năng"),rows,3,&selected,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));
 if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
 if(p.action==1){int old=nxt.performance_mode;nxt.performance_mode=selected;
  if(!Nxt_save()){nxt.performance_mode=old;Toast_show(Nxt_text("Cannot save performance mode","Không lưu được chế độ hiệu năng"),2200);}
  else {bool ok=NxtCPU_apply(selected);MusicPlaying_invalidate();Toast_show(ok?Nxt_text("Performance mode applied","Đã áp dụng chế độ hiệu năng"):Nxt_text("Visual mode applied; CPU control unavailable","Đã đổi mức hiệu ứng; chưa điều chỉnh được CPU"),2500);}
  return MODULE_EXIT_TO_MENU;
 }
 }}
ModuleExitReason NxtPerformance_settings(DisplayContext *d){
 int selected=0;for(;;){char setting[160];snprintf(setting,sizeof(setting),"%s\n%s",Nxt_text("Performance mode","Chế độ hiệu năng"),NxtPerformance_name(mode()));
 const char *rows[]={setting,Nxt_text("Device monitor\nCPU, temperature, RAM and battery","Theo dõi thiết bị\nCPU, nhiệt độ, RAM và pin"),NxtCPU_status()};
 NxtPick p=Nxt_pick(d,Nxt_text("Device performance","Hiệu năng thiết bị"),rows,3,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));
 if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
 if(p.action==1&&selected<2){ModuleExitReason r=selected==0?choose_mode(d):NxtMonitor_run(d);if(r==MODULE_EXIT_QUIT)return r;}
 }}

