#include "nxt_cpu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static void put(const char*n,const char*v){char p[512];snprintf(p,sizeof(p),"%s/%s",NXT_CPU_ROOT,n);FILE*f=fopen(p,"w");assert(f);assert(fprintf(f,"%s\n",v)>0);assert(!fclose(f));}
static int get(const char*n){char p[512];snprintf(p,sizeof(p),"%s/%s",NXT_CPU_ROOT,n);FILE*f=fopen(p,"r");assert(f);int v;assert(fscanf(f,"%d",&v)==1);fclose(f);return v;}
static void baseline(void){put("scaling_governor","userspace");put("scaling_available_governors","userspace");put("scaling_min_freq","2000000");put("scaling_max_freq","2000000");put("scaling_setspeed","2000000");put("cpuinfo_min_freq","408000");put("cpuinfo_max_freq","2000000");put("scaling_available_frequencies","408000 1008000 1608000 2000000");}
int main(void){baseline();
 int cap[]={1008000,1608000,2000000};
 for(int m=0;m<3;m++){assert(NxtCPU_apply(m));assert(get("scaling_max_freq")==cap[m]);assert(get("scaling_setspeed")==cap[m]);assert(get("scaling_min_freq")==408000);}
 assert(NxtCPU_apply(0));NxtCPU_tick(true);assert(get("scaling_max_freq")==2000000&&get("scaling_min_freq")==2000000&&get("scaling_setspeed")==2000000);
 NxtCPU_tick(false);assert(get("scaling_max_freq")==1008000);NxtCPU_release();
 put("scaling_available_governors","schedutil ondemand userspace");assert(NxtCPU_apply(1));assert(strstr(NxtCPU_status(),"schedutil"));assert(get("scaling_max_freq")==1608000);NxtCPU_release();
 baseline();put("cpuinfo_max_freq","1608000");assert(NxtCPU_apply(2));assert(get("scaling_max_freq")==1608000);NxtCPU_release();
 baseline();assert(NxtCPU_apply(0));put("scaling_max_freq","1608000");NxtCPU_release();assert(get("scaling_max_freq")==1608000);
 baseline();put("scaling_governor","performance");put("scaling_available_governors","performance");assert(!NxtCPU_apply(0));assert(get("scaling_max_freq")==2000000);
 assert(!NxtCPU_apply(-1)&&!NxtCPU_apply(3));
 puts("PASS production CPUFreq: three supported targets; boot 2GHz minimum lowered; userspace/dynamic governor; hw cap; hidden/resume/exit restore; external policy respected; unsupported governor rejected.");
}
