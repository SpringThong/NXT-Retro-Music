/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_PERFORMANCE_H
#define NXT_PERFORMANCE_H
#include "nxt.h"
#include <stdint.h>
enum { NXT_PERFORMANCE_BATTERY=0, NXT_PERFORMANCE_BALANCED=1, NXT_PERFORMANCE_FAST=2 };
const char *NxtPerformance_name(int);
ModuleExitReason NxtPerformance_settings(DisplayContext *);
uint32_t NxtPerformance_frameMS(bool playing,bool mechanical,int style);
uint32_t NxtPerformance_spectrumMS(void);
uint32_t NxtPerformance_recordMS(void);
uint32_t NxtPerformance_previewMS(void);
#endif

