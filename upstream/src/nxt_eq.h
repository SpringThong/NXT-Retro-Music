/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_EQ_H
#define NXT_EQ_H
#include "nxt.h"
#define NXT_EQ_PRESETS 12
bool NxtEQ_scope(void);
bool NxtEQ_closeRequested(void);
void NxtEQ_requestClose(void);
float NxtEQ_frequency(int,int);
const char *NxtEQ_name(int);
void NxtEQ_apply(int);
void NxtEQ_drawPresets(SDL_Surface *,int);
void NxtEQ_drawCustom(SDL_Surface *,int);
#endif
