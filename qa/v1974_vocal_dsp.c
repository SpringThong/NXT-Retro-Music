#include "nxt.h"
#include "nxt_vocal.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
void Old_configure(const NxtConfig*,int);void Old_process(float*,int,float);
NxtConfig nxt;
float NxtEQ_frequency(int n,int i){return 31.25f*powf(2.f,(float)i*8.f/(n-1));}
static void settle_off(int rate){float z[4096]={0};NxtVocal_setEnabled(false);for(int k=0;k<8;k++)NxtVocal_process(z,2048,rate);}
static unsigned random_state=129;static float random_sample(void){random_state=1664525u*random_state+1013904223u;return ((random_state>>8)/16777216.f-.5f)*1.8f;}
static void *toggle_thread(void*a){for(int i=0;i<20000;i++){NxtVocal_setEnabled(i&1);NxtVocal_setMode((i>>1)&1);NxtVocal_source(i);NxtVocal_mono();}return NULL;}
int main(void){
 assert(!NxtVocal_enabled()&&NxtVocal_mode()==1);
 const int rates[]={8000,44100,48000,96000,192000};
 for(unsigned r=0;r<sizeof(rates)/sizeof(*rates);r++){
  int rate=rates[r];settle_off(rate);
  for(int dsp=0;dsp<2;dsp++){
   NxtConfig c={0};c.dsp=dsp;c.eq_bands=5;c.width=1.25;c.balance=-.13;c.crossfeed=.23;c.preamp=-3;c.limiter=1;c.eq[0]=2;c.eq[2]=-1;
   NxtDSP_configure(&c,rate);Old_configure(&c,rate);
   for(int block=0;block<17;block++){float a[2048],b[2048];for(int j=0;j<2048;j++)a[j]=b[j]=random_sample();NxtDSP_process(a,1024,-2);Old_process(b,1024,-2);assert(!memcmp(a,b,sizeof(a)));}
  }
  int n=rate;float*p=malloc(n*2*sizeof(float));assert(p);NxtVocal_setMode(1);NxtVocal_setEnabled(true);
  for(int i=0;i<n;i++){float m=.35f*sinf(6.2831853f*440*i/rate),side=.25f*sinf(6.2831853f*1500*i/rate);p[2*i]=m+side;p[2*i+1]=m-side;}
  NxtVocal_process(p,n,rate);assert(!NxtVocal_mono());double error=0,power=0;
  for(int i=rate/10;i<n;i++){float side=.25f*sinf(6.2831853f*1500*i/rate);error+=(p[2*i]-side)*(p[2*i]-side);power+=side*side;assert(p[2*i]==p[2*i+1]&&isfinite(p[2*i]));}
  assert(error/power<1e-10); /* No centre sine, side accompaniment retained and mono-compatible. */
  /* Centre-only vocal break in a stereo song remains removed. */
  for(int i=0;i<n;i++)p[2*i]=p[2*i+1]=.3f*sinf(6.2831853f*440*i/rate);
  NxtVocal_process(p,n,rate);assert(!NxtVocal_mono());for(int i=rate/10;i<n;i++)assert(fabsf(p[2*i])<1e-7);
  NxtVocal_source(100+r); /* A new, genuinely mono source is retained. */
  for(int i=0;i<n;i++)p[2*i]=p[2*i+1]=.3f*sinf(6.2831853f*440*i/rate);
  NxtVocal_process(p,n,rate);assert(NxtVocal_mono());for(int i=rate/10;i<n;i++)assert(fabsf(p[2*i]-.3f*sinf(6.2831853f*440*i/rate))<1e-6);
  free(p);settle_off(rate);
 }
 /* A 20 ms slew prevents a hard sample jump for both enable and disable. */
 float step[2048];for(int i=0;i<1024;i++){step[2*i]=.8;step[2*i+1]=.2;}NxtVocal_setEnabled(true);NxtVocal_process(step,1024,48000);for(int i=1;i<1024;i++)assert(fabsf(step[2*i]-step[2*i-2])<.001);
 for(int i=0;i<1024;i++){step[2*i]=.8;step[2*i+1]=.2;}NxtVocal_setEnabled(false);NxtVocal_process(step,1024,48000);for(int i=1;i<1024;i++)assert(fabsf(step[2*i]-step[2*i-2])<.001);
 /* The retained-bass mode preserves low centre energy while reducing voice-band centre. */
 int rate=48000,n=rate*2;float*p=malloc(n*2*sizeof(float));assert(p);NxtVocal_setMode(0);NxtVocal_setEnabled(true);
 for(int i=0;i<n;i++){float mid=.2*sinf(6.2831853f*60*i/rate)+.2*sinf(6.2831853f*2000*i/rate),side=.15*sinf(6.2831853f*700*i/rate);p[2*i]=mid+side;p[2*i+1]=mid-side;}
 NxtVocal_process(p,n,rate);double bass_s=0,bass_c=0,voice_s=0,voice_c=0;
 for(int i=rate;i<n;i++){float x=p[2*i];bass_s+=x*sin(6.28318530718*60*i/rate);bass_c+=x*cos(6.28318530718*60*i/rate);voice_s+=x*sin(6.28318530718*2000*i/rate);voice_c+=x*cos(6.28318530718*2000*i/rate);}
 double bass=2*hypot(bass_s,bass_c)/rate,voice=2*hypot(voice_s,voice_c)/rate;assert(bass>.17&&voice<.014);free(p);
 /* Toggle/mode requests are atomic; audio owns the filter state. */
 pthread_t t;assert(!pthread_create(&t,NULL,toggle_thread,NULL));NxtConfig c={0};c.eq_bands=5;c.width=1;c.limiter=1;NxtDSP_configure(&c,48000);
 for(int j=0;j<2000;j++){float b[1024];for(int i=0;i<1024;i++)b[i]=random_sample();NxtDSP_process(b,512,0);for(int i=0;i<1024;i++)assert(isfinite(b[i])&&fabsf(b[i])<=1);}
 pthread_join(t,NULL);settle_off(48000);
 puts("PASS disabled DSP byte-identical to v1.9.7.3 across 5 rates/EQ/replay gain; exact synthetic centre removal and retained side; speaker-compatible output; mono bypass; 20ms toggle ramps; bass-preserving alternative; concurrent toggles and finite/clamped full-DSP output.");
}
