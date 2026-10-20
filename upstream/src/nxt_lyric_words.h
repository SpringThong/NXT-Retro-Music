/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_LYRIC_WORDS_H
#define NXT_LYRIC_WORDS_H
#include "lyrics.h"
typedef struct {uint16_t offset,length;char key[128];int weight;} NxtWordToken;
int NxtWords_tokens(const char *,NxtWordToken *,int);
int NxtWords_estimate(LyricLine *,int end_ms);
#endif
