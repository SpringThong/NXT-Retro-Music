/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_MONITOR_H
#define NXT_MONITOR_H
#include "nxt.h"
typedef struct {float cpu,temperature;int frequency,ram_used,ram_total,battery,charging;bool cpu_valid,temp_valid,freq_valid,ram_valid,battery_valid;} NxtMonitorSnapshot;
void NxtMonitor_tick(void);
void NxtMonitor_release(void);
const NxtMonitorSnapshot *NxtMonitor_snapshot(void);
ModuleExitReason NxtMonitor_run(DisplayContext *);
#endif
