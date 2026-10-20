/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_KARAOKE_H
#define NXT_KARAOKE_H
#include "nxt.h"
/* Optional, explicit-consent audio analysis; never runs in the audio callback. */
void NxtKaraoke_tick(void);
void NxtKaraoke_quit(void);
const char *NxtKaraoke_status(void);
ModuleExitReason NxtKaraoke_settings(DisplayContext *);
#endif
