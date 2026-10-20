/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_MOTION_H
#define NXT_MOTION_H
#include <SDL2/SDL.h>
#include <stdbool.h>
/* Page presentation is atomic: these compatibility hooks never translate,
 * fade, retain or restore a whole page. Local lyric/record animation is separate. */
void NxtMotion_page(SDL_Surface *,const char *identity,bool menu);
void NxtMotion_input(SDL_Surface *);
void NxtMotion_setMenu(bool);
bool NxtMotion_frame(SDL_Surface *,bool freshly_drawn);
void NxtMotion_release(void);
#endif
