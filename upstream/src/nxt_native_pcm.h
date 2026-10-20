/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_NATIVE_PCM_H
#define NXT_NATIVE_PCM_H
#include <stdint.h>
#include <stddef.h>
/* Integer samples are signed, full scale and left aligned, exactly as FFmpeg
 * represents S32/S32P. No float conversion, rounding, dither or gain here. */
typedef enum {NXT_PCM_S16,NXT_PCM_S24_3,NXT_PCM_S24,NXT_PCM_S32} NxtNativeFormat;
static inline int NxtNative_width(NxtNativeFormat f){return f==NXT_PCM_S16?2:f==NXT_PCM_S24_3?3:4;}
static inline int NxtNative_precision(NxtNativeFormat f){return f==NXT_PCM_S16?16:f==NXT_PCM_S32?32:24;}
static inline size_t NxtNative_pack(const int32_t *in,size_t samples,NxtNativeFormat format,uint8_t *out){
    int bytes=NxtNative_width(format);
    for(size_t i=0;i<samples;i++){
        uint32_t v=(uint32_t)in[i];
        if(format==NXT_PCM_S16)v>>=16;
        else if(format==NXT_PCM_S24_3)v>>=8;
        else if(format==NXT_PCM_S24)v=(uint32_t)(in[i]/256);
        for(int k=0;k<bytes;k++)out[i*bytes+k]=(uint8_t)(v>>(k*8));
    }
    return samples*(size_t)bytes;
}
#endif
