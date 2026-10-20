/* SPDX-License-Identifier: GPL-3.0-only
 * Page changes are atomic. Do not move/snapshot the framebuffer: GPU record,
 * status and spectrum layers have independent coordinates. Keeping the base
 * fixed avoids their separating during navigation and saves a full-screen copy.
 * Lyric, vinyl and spectrum animations retain their own production clocks. */
#include "nxt_motion.h"
void NxtMotion_page(SDL_Surface *s,const char *identity,bool menu){(void)s;(void)identity;(void)menu;}
void NxtMotion_setMenu(bool menu){(void)menu;}
void NxtMotion_input(SDL_Surface *s){(void)s;}
bool NxtMotion_frame(SDL_Surface *s,bool freshly_drawn){(void)s;(void)freshly_drawn;return false;}
void NxtMotion_release(void){}
