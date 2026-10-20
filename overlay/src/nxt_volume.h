/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_VOLUME_H
#define NXT_VOLUME_H
#include <stdbool.h>
void NxtVolume_set(int value); /* legacy 0..20 callers */
int NxtVolume_getPercent(void);
void NxtVolume_setPercent(int percent);
void NxtVolume_restore(void);
void NxtVolume_tick(void);
void NxtVolume_flush(void);
void NxtVolume_release(void);
float NxtVolume_gain(void);
bool NxtVolume_hardware(void);
void NxtVolume_routeChanged(void);
#endif
