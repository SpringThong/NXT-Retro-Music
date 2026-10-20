/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_CONTROLS_H
#define NXT_CONTROLS_H
#include "nxt.h"
typedef enum {NC_NONE,NC_PLAY,NC_MENU,NC_QUEUE,NC_LYRICS,NC_PREV,NC_NEXT,NC_SPECTRUM,NC_FAVORITE,NC_EQ,NC_QUICK,NC_SEEK_BACK,NC_SEEK_FORWARD,NC_VOLUME_UP,NC_VOLUME_DOWN,NC_BRIGHT_UP,NC_BRIGHT_DOWN,NC_SHUFFLE,NC_REPEAT,NC_SCREEN_OFF,NC_BACKGROUND,NC_STOP,NC_COUNT} NxtControlAction;
void NxtControl_defaults(void);void NxtControl_migrate(void);
bool NxtControl_pressed(int);bool NxtControl_repeated(int);bool NxtControl_held(int);bool NxtControl_aux(void);bool NxtControl_seek(int,int);
const char *NxtControl_name(int);ModuleExitReason NxtControl_settings(DisplayContext *);
#endif
