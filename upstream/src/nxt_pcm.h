/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_PCM_H
#define NXT_PCM_H
#include <stdbool.h>
#include <stddef.h>
typedef struct {int rate,bits,channels;char format[32];} NxtPCM;
/* Device transport precision, not the original file's effective bit depth. */
bool NxtPCM_parse(const char *text,NxtPCM *out);
bool NxtPCM_read(const char *asound_root,int card,NxtPCM *out);
void NxtPCM_label(const NxtPCM *pcm,bool vietnamese,char *out,size_t size);
#endif
