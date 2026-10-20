/* SPDX-License-Identifier: GPL-3.0-only
 * Exact USB hw transport. Never infer DoP capability from PCM support. */
#include "nxt_dsd_alsa.h"
#include "nxt_log.h"
#include <stdio.h>
#include <errno.h>
int NxtDSD_openALSA(snd_pcm_t**out,int card,const NxtDSDInfo*i,int mode,NxtDSDFormat*fmt,char error[256]){
 *out=NULL;if(card<0||!i||(mode!=0&&mode!=1)){snprintf(error,256,"Select a compatible USB DAC");return -1;}
 const snd_pcm_format_t types[]={SND_PCM_FORMAT_DSD_U8,SND_PCM_FORMAT_DSD_U16_LE,SND_PCM_FORMAT_DSD_U32_LE,SND_PCM_FORMAT_DSD_U16_BE,SND_PCM_FORMAT_DSD_U32_BE,SND_PCM_FORMAT_S24_3LE,SND_PCM_FORMAT_S24_LE,SND_PCM_FORMAT_S32_LE};
 const char*names[]={"DSD_U8","DSD_U16_LE","DSD_U32_LE","DSD_U16_BE","DSD_U32_BE","S24_3LE","S24_LE","S32_LE"};
 int last=-ENODEV,best_stage=-1,best_dev=0,best_format=mode?5:0,best_rate=0;const char*best_step="open";int opened=0;
 for(int dev=0;dev<8;dev++){char name[64];snprintf(name,64,"hw:%d,%d",card,dev);snd_pcm_t*p=NULL;int r=snd_pcm_open(&p,name,SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK|SND_PCM_NO_AUTO_RESAMPLE|SND_PCM_NO_AUTO_CHANNELS|SND_PCM_NO_AUTO_FORMAT|SND_PCM_NO_SOFTVOL);
  if(r<0){if(best_stage<0||r==-EBUSY){last=r;best_dev=dev;}if(r==-EBUSY)break;continue;}
  opened++;if(snd_pcm_type(p)!=SND_PCM_TYPE_HW){snd_pcm_close(p);continue;}
  for(int f=mode?5:0;f<(mode?8:5);f++){NxtDSDFormat format=f;int stride=NxtDSD_stride(format),rate=NxtDSD_frameRate(i,format);if(i->samples%(8*stride))continue;
   int stage=0;const char*step="params";snd_pcm_hw_params_t*h;snd_pcm_hw_params_alloca(&h);
   r=snd_pcm_hw_params_any(p,h);
#define SET(label,call) do {if(r>=0){stage++;step=label;r=(call);}}while(0)
   SET("resample",snd_pcm_hw_params_set_rate_resample(p,h,0));
   SET("access",snd_pcm_hw_params_set_access(p,h,SND_PCM_ACCESS_RW_INTERLEAVED));
   SET("format",snd_pcm_hw_params_set_format(p,h,types[f]));
   SET("channels",snd_pcm_hw_params_set_channels(p,h,i->channels));
   SET("rate",snd_pcm_hw_params_set_rate(p,h,rate,0));
   unsigned period=20000,buffer=100000;int dir=0;
   SET("period",snd_pcm_hw_params_set_period_time_near(p,h,&period,&dir));
   dir=0;SET("buffer",snd_pcm_hw_params_set_buffer_time_near(p,h,&buffer,&dir));
   SET("apply",snd_pcm_hw_params(p,h));
   if(r>=0){stage++;step="verify";unsigned observed=0,ch=0;snd_pcm_format_t actual;
    r=snd_pcm_hw_params_get_rate(h,&observed,NULL);if(r>=0)r=snd_pcm_hw_params_get_channels(h,&ch);if(r>=0)r=snd_pcm_hw_params_get_format(h,&actual);
    if(r>=0&&(observed!=(unsigned)rate||ch!=(unsigned)i->channels||actual!=types[f]||snd_pcm_format_physical_width(actual)!=NxtDSD_width(format)*8))r=-EINVAL;
   }
   if(r>=0){snd_pcm_sw_params_t*sw;snd_pcm_sw_params_alloca(&sw);snd_pcm_uframes_t frames=0;snd_pcm_hw_params_get_period_size(h,&frames,NULL);
    SET("sw-current",snd_pcm_sw_params_current(p,sw));SET("sw-start",snd_pcm_sw_params_set_start_threshold(p,sw,1));SET("sw-avail",snd_pcm_sw_params_set_avail_min(p,sw,frames?frames:1));SET("sw-apply",snd_pcm_sw_params(p,sw));
   }
   SET("prepare",snd_pcm_prepare(p));
#undef SET
   if(r>=0){*fmt=format;*out=p;NxtLog_step("audio","dsd-transport",true,"%s; %s; format=%s; frame_rate=%d; DSD_rate=%u; channels=%d",name,mode?"DoP":"Native",names[f],rate,i->rate,i->channels);return 0;}
   NxtLog_step("audio","dsd-probe",false,"%s; %s; %s; frame_rate=%d; step=%s; error=%d (%s)",name,mode?"DoP":"Native",names[f],rate,step,r,snd_strerror(r));
   if(stage>best_stage){best_stage=stage;last=r;best_dev=dev;best_format=f;best_rate=rate;best_step=step;}
   snd_pcm_hw_free(p);
  }snd_pcm_close(p);
 }
 if(last==-EBUSY)snprintf(error,256,"DAC busy at hw:%d,%d; stop other audio playback (%s)",card,best_dev,snd_strerror(last));
 else if(!opened)snprintf(error,256,"USB PCM unavailable at card %d (%s)",card,snd_strerror(last));
 else snprintf(error,256,"%s: hw:%d,%d %s %d Hz; %s failed (%s). %s",mode?"DoP":"Native DSD",card,best_dev,names[best_format],best_rate,best_step,snd_strerror(last),!mode&&best_stage<=3?"ROM driver has no usable Native DSD format; confirmed DoP DACs may use DoP.":"Exact original DSD transport unavailable; no PCM conversion.");
 return -1;
}
