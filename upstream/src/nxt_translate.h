/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_TRANSLATE_H
#define NXT_TRANSLATE_H
#include "nxt.h"
/* UI-thread API; rendering never starts I/O. Translation leaves Lyrics timing
 * and the audio/Bit-Perfect pipeline untouched. */
bool NxtTranslate_copyGroqKey(char *,size_t);
void NxtTranslate_tick(void);
void NxtTranslate_quit(void);
const char *NxtTranslate_line(int index);
const char *NxtTranslate_display(int index);
unsigned NxtTranslate_revision(void);
const char *NxtTranslate_modeName(int mode);
const char *NxtTranslate_status(void);
ModuleExitReason NxtTranslate_settings(DisplayContext *);
#endif

