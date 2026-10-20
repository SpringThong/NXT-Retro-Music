#include "nxt_decoder.h"
#include "nxt.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv){assert(argc==2);const char *files[]={"viet.wav","korea.mp3","japan.flac","chinese.m4a","traditional-alac.m4a","thai.ogg","thai.opus","test.aac","24bit.wav","32bit.wav"};const char *codecs[]={"pcm_s16le","mp3","flac","aac","alac","vorbis","opus","aac","pcm_s24le","pcm_s32le"};
 for(int i=0;i<10;i++){char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],files[i]);TrackInfo probe;assert(NxtDecoder_probe(path,&probe));assert(!strcmp(probe.codec,codecs[i]));NxtDecoder *d=NxtDecoder_open(path,48000);assert(d);const TrackInfo *meta=NxtDecoder_info(d);assert(meta->sample_rate>0&&meta->channels==2+(i>=8?-1:0));if(i==4)assert(!strcmp(meta->title,"風中的聲音"));if(i==5)assert(!strcmp(meta->title,"น้ำตาและความฝัน"));if(i==8)assert(meta->sample_rate==96000&&meta->bit_depth==24);if(i==9)assert(meta->bit_depth==32);
  float buf[2048];int total=0,got;float peak=0;while((got=NxtDecoder_read(d,buf,1024))>0){total+=got;for(int j=0;j<got*2;j++){assert(isfinite(buf[j]));if(fabsf(buf[j])>peak)peak=fabsf(buf[j]);}}assert(total>48000&&peak>.01f&&peak<1);assert(NxtDecoder_seek(d,1000));assert(NxtDecoder_read(d,buf,1024)>0);assert(NxtDecoder_position(d)>=1000&&NxtDecoder_position(d)<1100);NxtDecoder_close(d);printf("PASS decode / seek: %s (%s) %d frames\n",files[i],codecs[i],total);
 }
 NxtConfig config={0};config.eq_bands=10;config.width=1;config.limiter=1;NxtDSP_configure(&config,48000);float signal[2048],original[2048];for(int i=0;i<2048;i++)original[i]=signal[i]=.25f*sinf(i*.1f);NxtDSP_process(signal,1024,0);assert(!memcmp(signal,original,sizeof(signal)));NxtDSP_process(signal,1024,-6);assert(fabsf(signal[123]-original[123]*powf(10,-6/20.f))<1e-5);
 for(int mode=0;mode<3;mode++){config.dsp=1;config.eq_bands=mode==0?10:mode==1?15:31;for(int i=0;i<config.eq_bands;i++)config.eq[i]=(i%2?12:-12);config.preamp=12;NxtDSP_configure(&config,48000);for(int i=0;i<2048;i++)signal[i]=.9f;NxtDSP_process(signal,1024,0);for(int i=0;i<2048;i++)assert(isfinite(signal[i])&&fabsf(signal[i])<=.951f);}
 puts("PASS: DSP bypass, ReplayGain, 10/15/31-band stability and limiter.");return 0;}
