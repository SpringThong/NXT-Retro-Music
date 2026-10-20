/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_RECORD_H
#define NXT_RECORD_H
#include <stdbool.h>
#include <SDL2/SDL.h>
/* Static platter on the page; cached artwork/tonearm on its animation layer. */
void NxtRecord_draw(SDL_Surface *,SDL_Rect,SDL_Surface *,unsigned,bool);
void NxtRecord_drawTilted(SDL_Surface *,SDL_Rect,SDL_Surface *,unsigned,bool);
bool NxtRecord_needsRefresh(bool playing);
void NxtRecord_paint(bool playing);
void NxtRecord_hide(void);
void NxtRecord_release(void);
#endif
