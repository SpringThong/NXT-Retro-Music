/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_APPEARANCE_H
#define NXT_APPEARANCE_H
#include "nxt.h"
unsigned NxtAppearance_accent(void);
const char *NxtAppearance_wallpaperName(int);
void NxtAppearance_background(SDL_Surface *);
void NxtAppearance_release(void);
SDL_Surface *NxtAppearance_thumbnail(int);
ModuleExitReason NxtAppearance_wallpapers(DisplayContext *);
ModuleExitReason NxtAppearance_fonts(DisplayContext *);
ModuleExitReason NxtAppearance_run(DisplayContext *);
ModuleExitReason NxtAppearance_original(DisplayContext *);
ModuleExitReason NxtAppearance_album(DisplayContext *);
ModuleExitReason Nxt_LEDSettings(DisplayContext *);
#endif
