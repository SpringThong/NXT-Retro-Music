/* SPDX-License-Identifier: GPL-3.0-only
 * NextUI/NX Redux reversible CPUFreq policy. Never touch voltages, thermal trips,
 * other ROMs or a game's CPU policy. Frequencies are kHz, not MHz. */
#include "nxt_cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef NXT_CPU_ROOT
#define NXT_CPU_ROOT "/sys/devices/system/cpu/cpu0/cpufreq"
#endif
static char status[256]="CPU policy not applied";
#if (defined(NXT_ROM_NEXTUI) || defined(NXT_ROM_NXREDUX)) && (!defined(PLATFORM_DESKTOP) || defined(NXT_CPU_TEST))
static bool owned;
static int requested=-1,old_min,old_max,old_speed,applied_max;
static char old_governor[64],applied_governor[64];
static bool read_node(const char *name,char *s,size_t n){char p[512];snprintf(p,sizeof(p),"%s/%s",NXT_CPU_ROOT,name);FILE *f=fopen(p,"r");if(!f)return false;size_t z=fread(s,1,n-1,f);s[z]=0;bool ok=!ferror(f);fclose(f);while(z&&strchr(" \r\n\t",s[z-1]))s[--z]=0;return ok&&z;}
static int number(const char *name){char s[96],*end;if(!read_node(name,s,sizeof(s)))return 0;long x=strtol(s,&end,10);return !*end&&x>0&&x<=4000000?(int)x:0;}
static bool write_node(const char *name,const char *s){char p[512];snprintf(p,sizeof(p),"%s/%s",NXT_CPU_ROOT,name);FILE *f=fopen(p,"w");if(!f)return false;bool ok=fprintf(f,"%s\n",s)>0;return fclose(f)==0&&ok;}
static bool put(const char *n,int v){char s[32];snprintf(s,sizeof(s),"%d",v);return v>0&&write_node(n,s);}
static bool has(const char *list,const char *word){size_t n=strlen(word);for(const char *p=list;*p;){while(*p&&strchr(" \t\r\n",*p))p++;const char *e=p;while(*e&&!strchr(" \t\r\n",*e))e++;if((size_t)(e-p)==n&&!memcmp(p,word,n))return true;p=e;}return false;}
static int frequency(int cap,int floor){char s[2048];if(!read_node("scaling_available_frequencies",s,sizeof(s)))return cap;int best=0;for(char *p=s;*p;){char *e;long x=strtol(p,&e,10);if(e==p){p++;continue;}if(x>=floor&&x<=cap&&x>best)best=(int)x;p=e;}return best;}
static void restore(void){
 if(!owned)return;
 /* Restore only attributes we still own. Another foreground app / ROM may
  * have replaced our policy; never overwrite that newer policy. */
 char current[64];bool ours=read_node("scaling_governor",current,sizeof(current))&&!strcmp(current,applied_governor)&&number("scaling_max_freq")==applied_max;
 if(ours){put("scaling_min_freq",old_min<applied_max?old_min:applied_max);put("scaling_max_freq",old_max);put("scaling_min_freq",old_min);write_node("scaling_governor",old_governor);if(!strcmp(old_governor,"userspace")&&old_speed)put("scaling_setspeed",old_speed);}
 owned=false;applied_max=0;
}
bool NxtCPU_apply(int mode){
 if(mode<0||mode>2)return false;
 if(!owned){
  if(!read_node("scaling_governor",old_governor,sizeof(old_governor))||(old_min=number("scaling_min_freq"))<=0||(old_max=number("scaling_max_freq"))<old_min){snprintf(status,sizeof(status),"CPU control unavailable / permission denied");return false;}
  old_speed=number("scaling_setspeed");if(!old_speed)old_speed=number("scaling_cur_freq");
 }
 int hardware=number("cpuinfo_max_freq"),floor=number("cpuinfo_min_freq");if(!floor)floor=old_min;
 int limit=mode==0?1008000:mode==1?1608000:2000000;
 /* NX's launcher applies its own temporary low cap; it is not a hardware
  * maximum. Restore that cap afterwards, but allow the app's chosen mode. */
#ifndef NXT_ROM_NXREDUX
 if(limit>old_max)limit=old_max;
#endif
 if(hardware>0&&limit>hardware)limit=hardware;
 if(limit<floor)limit=floor;limit=frequency(limit,floor);
 if(!limit){snprintf(status,sizeof(status),"No supported CPU frequency for this mode");return false;}
 char available[512]="";read_node("scaling_available_governors",available,sizeof(available));
 const char *governor=has(available,"schedutil")?"schedutil":has(available,"ondemand")?"ondemand":has(available,"userspace")||!strcmp(old_governor,"userspace")?"userspace":NULL;
 if(!governor){snprintf(status,sizeof(status),"ROM has no supported CPU governor");return false;}
 /* Lower the minimum first, so a boot-time 2 GHz minimum cannot defeat a
  * lower cap. Use the driver-supported table; never overclock. */
 if(!put("scaling_min_freq",floor)||!put("scaling_max_freq",limit)||!write_node("scaling_governor",governor)||(!strcmp(governor,"userspace")&&!put("scaling_setspeed",limit))){
  put("scaling_max_freq",old_max);put("scaling_min_freq",old_min);write_node("scaling_governor",old_governor);if(!strcmp(old_governor,"userspace")&&old_speed)put("scaling_setspeed",old_speed);
  owned=false;snprintf(status,sizeof(status),"CPU write failed; previous policy restored");return false;
 }
 owned=true;requested=mode;applied_max=limit;snprintf(applied_governor,sizeof(applied_governor),"%s",governor);
 if(number("scaling_max_freq")!=limit){restore();snprintf(status,sizeof(status),"CPU limit rejected by ROM");return false;}
 snprintf(status,sizeof(status),"CPU %s · %d MHz %s",governor,limit/1000,!strcmp(governor,"userspace")?"target":"maximum");return true;
}
void NxtCPU_release(void){restore();snprintf(status,sizeof(status),"ROM CPU policy restored");}
void NxtCPU_tick(bool hidden){if(hidden){if(owned)NxtCPU_release();}else if(!owned&&requested>=0){int m=requested;requested=-1;NxtCPU_apply(m);}}
#else
bool NxtCPU_apply(int mode){(void)mode;snprintf(status,sizeof(status),"ROM-managed CPU; visual workload control only");return true;}
void NxtCPU_tick(bool hidden){(void)hidden;}
void NxtCPU_release(void){}
#endif
const char *NxtCPU_status(void){return status;}
