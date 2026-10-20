/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_LED_H
#define NXT_LED_H
#include "nxt.h"
#define NXT_LED_PRESETS 16
unsigned NxtLED_rgb(void);const char *NxtLED_name(int);ModuleExitReason NxtLED_palette(DisplayContext *);
unsigned NxtLED_groupRGB(int,uint32_t);
const char *NxtLED_effectName(int);
#endif
