/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_LOG_H
#define NXT_LOG_H
#include <stdbool.h>
void NxtLog_step(const char *,const char *,bool,const char *,...);
bool NxtLog_export(void);
#endif
