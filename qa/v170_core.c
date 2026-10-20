/* Actual config/DSP/telemetry with temporary kernel file fixtures. */
#include "nxt.h"
#include "nxt_eq.h"
#include "nxt_controls.h"
#include "api.h"
#include "file_utils.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
static uint32_t qa_ticks=1000;static time_t qa_clock=1791504000;
uint32_t SDL_GetTicks(void){return qa_ticks;}
bool NxtSession_isHidden(void){return false;}
int PWR_getDisplayOff(void){return 0;}
static time_t qa_time(time_t *out){if(out)*out=qa_clock;return qa_clock;}
#define time qa_time
#include "nxt_monitor.c"
#undef time
int userdata_snpath(const char *p,char *o,size_t n){return snprintf(o,n,"%s/data/%s",getenv("QA_ROOT"),p);}
void NxtPersonal_savePreferences(void){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *f,...){}
void Player_refreshOptions(void){}
int Player_getOutputRate(void){return 48000;}
static void fixture(const char *rel,const char *s){char p[1500];snprintf(p,sizeof(p),"%s/%s",getenv("QA_ROOT"),rel);FILE *f=fopen(p,"w");assert(f);assert(fputs(s,f)>=0);assert(!fclose(f));}
static void defaults(void){memset(&nxt,0,sizeof(nxt));nxt.eq_bands=5;nxt.width=nxt.dsp=nxt.limiter=1;nxt.avatar_icon=-1;nxt.animations=nxt.cover_mode=1;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0x63b9ef;nxt.led_rgb2=0xffa4c9;for(int i=0;i<16;i++)nxt.keys[i]=Nxt_keyIds[i];for(int i=0;i<6;i++){nxt.peq[i].hz=100;nxt.peq[i].q=1;}}
static void config_test(void){defaults();nxt.interface_style=23;nxt.cover_mode=0;nxt.avatar_mode=1;nxt.avatar_emoji=63;nxt.avatar_pixel=32;nxt.birth_day=29;nxt.birth_month=2;nxt.genre=9;nxt.mood=2;nxt.spectrum_palette=20;nxt.spectrum_mode=4;nxt.spectrum_rgb=0x123456;nxt.spectrum_rgb2=0xfedcba;nxt.led_effect=5;nxt.led_rgb2=0x5577aa;snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"NXT 🌸");for(int i=0;i<7;i++)nxt.menu_icons[i]=65+i;nxt.wallpaper_mode=12;nxt.accent_color=16;nxt.accent_rgb=0x123456;nxt.wallpaper_dim=20;nxt.panel_opacity=35;assert(Nxt_save());char path[1500];userdata_snpath("nxt.json",path,sizeof(path));defaults();Nxt_loadPreferences(path);assert(nxt.interface_style==23&&nxt.avatar_mode==1&&nxt.avatar_emoji==63&&nxt.birth_day==29&&nxt.genre==9&&nxt.mood==2&&nxt.menu_icons[6]==71&&nxt.cover_mode==0&&nxt.accent_rgb==0x123456&&nxt.wallpaper_mode==12&&nxt.spectrum_mode==4&&nxt.spectrum_rgb2==0xfedcba&&nxt.led_rgb2==0x5577aa&&!strcmp(nxt.avatar_text,"NXT 🌸"));
 fixture("data/old.json","{\"theme\":2,\"accent_color\":16,\"accent_rgb\":1122867,\"wallpaper_mode\":21,\"wallpaper_path\":\"/Pictures/old.jpg\",\"wallpaper_visibility_fix1\":true,\"eq_bands\":31}");userdata_snpath("old.json",path,sizeof(path));Nxt_loadPreferences(path);assert(nxt.interface_style==0&&nxt.avatar_icon==-1&&nxt.avatar_mode==0&&nxt.menu_icons[6]==0&&nxt.led_effect==0&&nxt.birth_day==0&&nxt.eq_bands==31&&nxt.theme==2&&nxt.wallpaper_mode==21&&nxt.accent_rgb==0x112233&&!strcmp(nxt.wallpaper_path,"/Pictures/old.jpg"));
 fixture("data/bad.json","{\"interface_style\":1e99,\"spectrum_rgb\":-1e99,\"avatar_emoji\":999,\"menu_icons\":[1e99,-1e99,\"invalid\"],\"eq_bands\":6}");userdata_snpath("bad.json",path,sizeof(path));Nxt_loadPreferences(path);assert(nxt.interface_style==23&&nxt.spectrum_rgb==0&&nxt.avatar_emoji==63&&nxt.menu_icons[0]==128&&nxt.menu_icons[1]==0&&nxt.eq_bands==10);nxt.avatar_mode=3;snprintf(nxt.avatar_path,sizeof(nxt.avatar_path),"someone.png");nxt.menu_icons[0]=128;nxt.genre=19;userdata_snpath("new-profile.json",path,sizeof(path));Nxt_loadPreferences(path);assert(nxt.avatar_mode==0&&!nxt.avatar_path[0]&&nxt.menu_icons[0]==0&&nxt.genre==0);
 puts("PASS actual atomic config roundtrip, all new preferences, old profile migration retains Original colors/background; new profiles clear identity; malformed numeric bounds under UBSan.");}
static void telemetry_test(void){fixture("proc/stat","cpu 100 0 100 800 0 0 0 0\n");fixture("proc/meminfo","MemTotal: 1048576 kB\nMemAvailable: 262144 kB\n");fixture("sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","1400000\n");fixture("sys/class/thermal/thermal_zone0/type","battery\n");fixture("sys/class/thermal/thermal_zone0/temp","90000\n");fixture("sys/class/thermal/thermal_zone1/type","sunxi-soc\n");fixture("sys/class/thermal/thermal_zone1/temp","43000\n");fixture("sys/class/power_supply/battery/capacity","80\n");fixture("sys/class/power_supply/battery/status","Discharging\n");watching=true;NxtMonitor_tick();const NxtMonitorSnapshot *p=&snapshot;assert(!p->cpu_valid&&p->temp_valid&&p->temperature==43&&p->frequency==1400&&p->ram_total==1024&&p->ram_used==768&&p->battery==80&&!p->charging);qa_ticks+=1000;fixture("proc/stat","cpu 130 0 120 850 0 0 0 0\n");NxtMonitor_tick();assert(snapshot.cpu_valid&&fabs(snapshot.cpu-50)<.01);
 qa_ticks+=60000;qa_clock+=60;fixture("sys/class/power_supply/battery/capacity","78\n");NxtMonitor_tick();int drops[7],minutes[7],last[7];history(1,drops,minutes,last);assert(drops[0]==2&&minutes[0]==1&&last[0]==78);qa_ticks+=60000;qa_clock+=60;fixture("sys/class/power_supply/battery/capacity","79\n");fixture("sys/class/power_supply/battery/status","Charging\n");NxtMonitor_tick();history(1,drops,minutes,last);assert(drops[0]==2);qa_ticks+=60000;qa_clock+=1000;fixture("sys/class/power_supply/battery/capacity","70\n");fixture("sys/class/power_supply/battery/status","Discharging\n");NxtMonitor_tick();history(1,drops,minutes,last);assert(drops[0]==2);NxtMonitor_release();qa_ticks+=60000;qa_clock+=60;NxtMonitor_tick();history(1,drops,minutes,last);assert(drops[0]==2);fixture("sys/class/power_supply/battery/capacity","200\n");fixture("sys/class/thermal/thermal_zone1/temp","200000\n");fixture("sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","-1\n");qa_ticks+=1000;NxtMonitor_tick();assert(!snapshot.battery_valid&&!snapshot.temp_valid&&!snapshot.freq_valid);NxtMonitor_release();
 puts("PASS actual proc/sys telemetry: CPU deltas, SoC-only temp, CPU MHz, MemAvailable; real SQLite day history excludes charging, gaps and different sessions; invalid sensors unsupported.");}
static void dsp_test(void){defaults();for(int band=0;band<5;band++){memset(nxt.eq,0,sizeof(nxt.eq));nxt.eq[band]=6;nxt.limiter=0;NxtDSP_configure(&nxt,48000);float hz=NxtEQ_frequency(5,band);assert(fabs(NxtDSP_response(hz)-6)<.03);float *samples=calloc(48000*2,sizeof(float));assert(samples);for(int i=0;i<48000;i++)samples[i*2]=samples[i*2+1]=.02f*sinf(2*M_PI*hz*i/48000);NxtDSP_process(samples,48000,0);double rms=0;for(int i=24000;i<48000;i++)rms+=samples[i*2]*samples[i*2];free(samples);rms=sqrt(rms/24000);assert(rms>.027&&rms<.03);}char path[1500];userdata_snpath("five.nxt-eq",path,sizeof(path));assert(Nxt_presetWrite(path));nxt.eq_bands=31;assert(Nxt_presetRead(path)&&nxt.eq_bands==5);puts("PASS actual five-band biquads at 60/250/1000/4000/12000 Hz: +6dB response and PCM RMS, five-band preset export/import.");}
int main(void){config_test();telemetry_test();dsp_test();return 0;}
