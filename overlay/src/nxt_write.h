/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_WRITE_H
#define NXT_WRITE_H
#include <stdbool.h>
#include <SDL2/SDL.h>
bool NxtWrite_text(const char *path,const char *text,bool durable);
bool NxtWrite_image(const char *path,SDL_Surface *image);
bool NxtWrite_remove(const char *path);
void NxtWrite_flush(void);
void NxtWrite_quit(void);
bool NxtWrite_failed(void);
#endif
