/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_TEXT_H
#define NXT_TEXT_H
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stddef.h>
TTF_Font *NxtText_open(const char *,int);
void NxtText_close(TTF_Font *);
int NxtText_size(TTF_Font *,const char *,int *,int *);
SDL_Surface *NxtText_render(TTF_Font *,const char *,SDL_Color);
bool NxtText_hasGlyph(TTF_Font *,Uint32);
const char *NxtText_nextCluster(const char *);
#define NXT_FONT_COUNT 60
#define NXT_FONT_MISANS 16
const char *NxtText_fontName(int);
const char *NxtText_fontPath(int);
const char *NxtText_italicPath(int);
void NxtText_copy(char *,const char *,size_t);
char *NxtText_searchKey(const char *);
#endif
