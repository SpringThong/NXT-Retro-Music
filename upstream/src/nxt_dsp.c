/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
/* Audio EQ uses Robert Bristow-Johnson's biquad equations. No ViPER binary or preset compatibility is claimed. */
#include "nxt.h"
#include "nxt_eq.h"
#include <math.h>
#include <string.h>
#include <pthread.h>
typedef struct { double b0,b1,b2,a1,a2,z1[2],z2[2]; } Biquad;
static Biquad filters[40]; static int count,rate=48000;
static NxtConfig cfg; static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static float peak,envelope; static bool clipped;
static void peak_filter(Biquad *b,double hz,double db,double q) {
    double w=2*M_PI*fmin(hz,rate*.45)/rate,a=sin(w)/(2*q),A=pow(10,db/40),a0=1+a/A;
    *b=(Biquad){.b0=(1+a*A)/a0,.b1=-2*cos(w)/a0,.b2=(1-a*A)/a0,.a1=-2*cos(w)/a0,.a2=(1-a/A)/a0};
}
void NxtDSP_configure(const NxtConfig *c,int r) {
    pthread_mutex_lock(&lock); cfg=*c; rate=r>0?r:48000; count=0;
    for(int i=0;i<c->eq_bands;i++) { double hz=NxtEQ_frequency(c->eq_bands,i); peak_filter(&filters[count++],hz,c->eq[i],c->eq_bands/10.0); }
    peak_filter(&filters[count++],80,c->bass,.65); peak_filter(&filters[count++],10000,c->treble,.65);
    for(int i=0;i<6;i++) if(c->peq[i].enabled) peak_filter(&filters[count++],c->peq[i].hz,c->peq[i].gain,c->peq[i].q);
    envelope=0; pthread_mutex_unlock(&lock);
}
static double process(Biquad *b,double x,int ch) {
    double y=b->b0*x+b->z1[ch]; b->z1[ch]=b->b1*x-b->a1*y+b->z2[ch]; b->z2[ch]=b->b2*x-b->a2*y;
    if(!isfinite(y)) { b->z1[ch]=b->z2[ch]=0; return 0; } return y;
}
void NxtDSP_process(float *p,int frames,float replay_db) {
    pthread_mutex_lock(&lock);
    float g=powf(10,((cfg.dsp?cfg.preamp:0)+replay_db)/20); peak=0; clipped=false;
    for(int i=0;i<frames;i++) {
        float l=p[2*i]*g,r=p[2*i+1]*g;
        if(cfg.dsp) {
            for(int k=0;k<count;k++) { l=process(&filters[k],l,0); r=process(&filters[k],r,1); }
            float mid=(l+r)*.5f,side=(l-r)*.5f*cfg.width; l=mid+side; r=mid-side;
            float old=l; l=l*(1-.3*cfg.crossfeed)+r*.3*cfg.crossfeed; r=r*(1-.3*cfg.crossfeed)+old*.3*cfg.crossfeed;
            l*=cfg.balance>0?1-cfg.balance:1; r*=cfg.balance<0?1+cfg.balance:1;
            float v=fmaxf(fabsf(l),fabsf(r)); envelope+= (v-envelope)*(v>envelope?.02f:.0005f);
            if(cfg.compressor>0&&envelope>.25f) { float target=.25f+powf(envelope/.25f,.5f)*.25f; float gain=fminf(1,target/envelope); gain=1-(1-gain)*cfg.compressor; l*=gain; r*=gain; }
        }
        float v=fmaxf(fabsf(l),fabsf(r)); if(v>peak) peak=v; if(v>1) clipped=true;
        if(cfg.limiter&&v>.95f) { float gain=.95f/v; l*=gain; r*=gain; }
        p[2*i]=fmaxf(-1,fminf(1,l)); p[2*i+1]=fmaxf(-1,fminf(1,r));
    }
    pthread_mutex_unlock(&lock);
}
float NxtDSP_response(float hz) {
    pthread_mutex_lock(&lock); double db=cfg.preamp,w=2*M_PI*hz/rate;
    for(int k=0;k<count;k++) { Biquad *b=&filters[k]; double nr=b->b0+b->b1*cos(w)+b->b2*cos(2*w),ni=-b->b1*sin(w)-b->b2*sin(2*w),dr=1+b->a1*cos(w)+b->a2*cos(2*w),di=-b->a1*sin(w)-b->a2*sin(2*w); db+=10*log10((nr*nr+ni*ni)/(dr*dr+di*di)); }
    pthread_mutex_unlock(&lock); return db;
}
float NxtDSP_peak(void) { pthread_mutex_lock(&lock); float v=peak; pthread_mutex_unlock(&lock); return v; }
bool NxtDSP_clipped(void) { pthread_mutex_lock(&lock); bool v=clipped; pthread_mutex_unlock(&lock); return v; }
