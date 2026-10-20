/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_AURALIS_H
#define NXT_AURALIS_H
#include "nxt.h"
bool NxtAuralis_playing(SDL_Surface *,int,int,bool,int);
void NxtAuralis_lyrics(SDL_Surface *,int,bool);
bool NxtAuralis_motion(void);
void NxtAuralis_release(void);
const char *NxtLyricFX_name(int);
ModuleExitReason NxtLyricFX_settings(DisplayContext *);
float NxtLyricFX_progress(Uint32 age);
void NxtLyricFX_transform(float progress,int scale,int *dx,int *dy,Uint8 *alpha);
bool NxtLyricFX_slides(void);
#endif
