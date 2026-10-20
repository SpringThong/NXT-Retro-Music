/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#ifndef NXT_I18N_H
#define NXT_I18N_H
#include <SDL2/SDL_ttf.h>
#include "nxt_text.h"
SDL_Surface *Nxt_render(TTF_Font *,const char *,SDL_Color);
int NxtPad_isPressed(int);
int NxtPad_justPressed(int);
int NxtPad_justReleased(int);
int NxtPad_justRepeated(int);
#define TTF_RenderUTF8_Blended(font,text,color) Nxt_render((font),(text),(color))
#define TTF_SizeUTF8(font,text,w,h) NxtText_size((font),(text),(w),(h))
#define TTF_OpenFont(path,size) NxtText_open((path),(size))
#define TTF_CloseFont(font) NxtText_close((font))
#endif
