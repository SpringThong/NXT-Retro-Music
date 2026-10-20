/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_THEME_H
#define NXT_THEME_H
#include "nxt_skin.h"
#define NXT_THEME_COUNT 51
#define NXT_THEME_AURALIS 50
const char *NxtTheme_name(int);
bool NxtTheme_color(NxtColor,SDL_Color *);
void NxtTheme_header(bool);
void NxtTheme_chrome(SDL_Surface *,bool);
bool NxtTheme_home(SDL_Surface *,int);
bool NxtTheme_playing(SDL_Surface *,bool,int);
void NxtTheme_counters(int,int);
void NxtTheme_release(void);
int NxtTheme_radius(int);
bool NxtTheme_linearHome(void);
ModuleExitReason NxtTheme_gallery(DisplayContext *);
#endif
