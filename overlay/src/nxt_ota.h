/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_OTA_H
#define NXT_OTA_H
#include "nxt.h"
void NxtOTA_init(void);
void NxtOTA_tick(void);
void NxtOTA_quit(void);
ModuleExitReason NxtOTA_guide(DisplayContext *);
ModuleExitReason NxtOTA_run(DisplayContext *);
#endif
