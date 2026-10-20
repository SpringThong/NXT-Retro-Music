/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_CUSTOM_H
#define NXT_CUSTOM_H
#include "nxt.h"
#define NXT_CUSTOM_ICONS 300
#define NXT_CUSTOM_EMOJI 180
const char *NxtCustom_iconName(int);
const char *NxtCustom_iconLabel(int);
const char *NxtCustom_emoji(int);
const char *NxtCustom_menuIcon(int);
bool NxtCustom_draw(SDL_Surface *,const char *,int,int,int,SDL_Color);
void NxtCustom_drawAvatar(SDL_Surface *,int,int,int,const char *,const char *,int,int,int,SDL_Color);
void NxtCustom_release(void);
int NxtCustom_gallery(DisplayContext *,bool,int);
ModuleExitReason NxtCustom_menu(DisplayContext *);
ModuleExitReason NxtCustom_avatarEdit(DisplayContext *);
ModuleExitReason NxtCustom_spectrum(DisplayContext *);
ModuleExitReason NxtCustom_rgb(DisplayContext *,const char *,int *);
const char *NxtCustom_genre(int);
const char *NxtCustom_mood(int);
void NxtCustom_query(char *,size_t,int);
#endif
