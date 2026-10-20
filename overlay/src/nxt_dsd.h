/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_DSD_H
#define NXT_DSD_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef enum {NXT_DSD_U8,NXT_DSD_U16_LE,NXT_DSD_U32_LE,NXT_DSD_U16_BE,NXT_DSD_U32_BE,NXT_DSD_DOP24_3LE,NXT_DSD_DOP24_LE,NXT_DSD_DOP32_LE} NxtDSDFormat;
typedef struct {uint32_t rate;int channels,duration_ms;uint64_t samples;char title[256],artist[256],album[256],container[8];} NxtDSDInfo;
typedef struct NxtDSD NxtDSD;
bool NxtDSD_isPath(const char*);
NxtDSD *NxtDSD_open(const char*,char error[256]);
void NxtDSD_close(NxtDSD*);
const NxtDSDInfo *NxtDSD_info(const NxtDSD*);
bool NxtDSD_probe(const char*,NxtDSDInfo*);
uint8_t *NxtDSD_cover(const char*,size_t*);
int NxtDSD_width(NxtDSDFormat);
int NxtDSD_stride(NxtDSDFormat);
int NxtDSD_frameRate(const NxtDSDInfo*,NxtDSDFormat);
int NxtDSD_read(NxtDSD*,uint8_t*,int,NxtDSDFormat);
int NxtDSD_seek(NxtDSD*,int,NxtDSDFormat);
#endif
