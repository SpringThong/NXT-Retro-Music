/* SPDX-License-Identifier: GPL-3.0-only
 * Short bounded ramps around network underruns. PCM online paths only;
 * native DSD and local bit-perfect output never pass through this helper. */
#ifndef NXT_ONLINE_RAMP_H
#define NXT_ONLINE_RAMP_H
#include <stdbool.h>
typedef struct { float gain,last[2],tail[2];int remaining;bool gap; } NxtOnlineRamp;
static void NxtOnlineRamp_process(NxtOnlineRamp *r,float *pcm,int frames,int valid,int rate){
    int length=rate/200;if(length<1)length=1;float step=1.f/length;
    for(int i=0;i<frames;i++){
        if(i<valid){
            if(r->gap){r->gain=0;r->gap=false;}
            r->gain+=step;if(r->gain>1)r->gain=1;
            for(int ch=0;ch<2;ch++){pcm[i*2+ch]*=r->gain;r->last[ch]=pcm[i*2+ch];}
        }else{
            if(!r->gap){r->tail[0]=r->last[0];r->tail[1]=r->last[1];r->remaining=length;r->gap=true;}
            float gain=r->remaining>0?(float)--r->remaining/length:0;
            for(int ch=0;ch<2;ch++)pcm[i*2+ch]=r->tail[ch]*gain;
        }
    }
}
#endif
