/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_SKIN_H
#define NXT_SKIN_H
#include "nxt.h"
typedef enum {NXT_BG,NXT_PANEL,NXT_INK,NXT_MUTED,NXT_LINE,NXT_ACCENT,NXT_ACCENT_INK} NxtColor;
typedef enum {NXT_SOURCE_AUTO,NXT_SOURCE_SPOTIFY,NXT_SOURCE_SOUNDCLOUD,NXT_SOURCE_DRIVE} NxtSkinSource;
/* Provider menus restore the previous source when returning to their caller. */
NxtSkinSource NxtSkin_setSource(NxtSkinSource);
SDL_Color NxtSkin_color(NxtColor);
void NxtSkin_rect(SDL_Surface *,SDL_Rect,SDL_Color,int);
void NxtSkin_panel(SDL_Surface *,SDL_Rect,SDL_Color,int);
void NxtSkin_blitText(SDL_Surface *,SDL_Surface *,SDL_Rect,int,int,SDL_Color);
void NxtSkin_icon(SDL_Surface *,const char *,int,int,int,SDL_Color);
TTF_Font *NxtSkin_font(SDL_Surface *,int);
void NxtSkin_text(SDL_Surface *,int,const char *,int,int,int,SDL_Color);
int NxtSkin_status(SDL_Surface *);
void NxtSkin_begin(SDL_Surface *,const char *,const char *,bool);
void NxtSkin_footer(SDL_Surface *,const char *,bool);
void NxtSkin_mini(SDL_Surface *);
int NxtSkin_top(SDL_Surface *);
int NxtSkin_bottom(SDL_Surface *,bool);
void NxtSkin_list(SDL_Surface *,const char *const *,int,int,bool);
void NxtSkin_home(SDL_Surface *,int);
void NxtSkin_playing(SDL_Surface *,int,int,bool,int);
void NxtSkin_tabs(SDL_Surface *,const char *const *,int,int,int);
void NxtSkin_release(void);
#endif
