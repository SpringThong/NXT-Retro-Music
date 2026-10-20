/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_STARTUP_H
#define NXT_STARTUP_H
#include "nxt.h"
#include "player.h"
ModuleExitReason NxtStartup_run(DisplayContext *);
void NxtStartup_save(void);
void NxtStartup_tick(void);
void NxtStartup_quit(void);
bool NxtStartup_preview(void);
const TrackInfo *NxtStartup_info(void);
const char *NxtStartup_path(void);
int NxtStartup_position(void);
SDL_Surface *NxtStartup_art(void);
int NxtStartup_takeSeek(const char *source,const char *path);
#endif
