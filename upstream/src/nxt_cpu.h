/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_CPU_H
#define NXT_CPU_H
#include <stdbool.h>
bool NxtCPU_apply(int mode);
void NxtCPU_tick(bool hidden);
void NxtCPU_release(void);
const char *NxtCPU_status(void);
#endif
