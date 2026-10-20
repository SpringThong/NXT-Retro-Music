/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_LYRIC_PREVIEW_H
#define NXT_LYRIC_PREVIEW_H
#include "nxt.h"
#include "lyrics.h"
enum {NXT_EDIT_FONT=-1,NXT_EDIT_SUNG,NXT_EDIT_UNSUNG,NXT_EDIT_TRANSLATION,NXT_EDIT_TITLE,NXT_EDIT_ARTIST,NXT_EDIT_BOLD,NXT_EDIT_OUTLINE,NXT_EDIT_OUTLINE_COLOR,NXT_EDIT_SHADOW,NXT_EDIT_TITLE_SIZE,NXT_EDIT_LYRIC_SIZE,NXT_EDIT_EFFECT,NXT_EDIT_ESTIMATE,NXT_EDIT_ANIMATIONS,NXT_EDIT_NOW_BRIGHTNESS,NXT_EDIT_FULL_BRIGHTNESS,NXT_EDIT_RESET};
ModuleExitReason NxtLyricPreview_run(DisplayContext *,int first);
bool NxtLyricPreview_active(void);
Uint32 NxtLyricPreview_age(void);
int NxtLyricPreview_position(void);
const char *NxtLyricPreview_line(int);
const char *NxtLyricPreview_translation(int);
int NxtLyricPreview_wordCount(int);
const LyricWord *NxtLyricPreview_wordAt(int,int);
/* Draw shared full-lyric rows; used by both live lyrics and the preview. */
void NxtLyricPreview_row(SDL_Surface *,const char *,int owner,bool translated,bool current,int x,int y,int w,int h,Uint8 alpha);
#endif
