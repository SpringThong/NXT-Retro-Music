/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_NEXTUI_H
#define NXT_NEXTUI_H
#include <stdbool.h>
#include <stddef.h>
void NxtNextUI_init(void);
void NxtNextUI_quit(void);
bool NxtNextUI_bluetoothConnected(void);
bool NxtNextUI_bluetoothRoute(char *, size_t);
bool NxtNextUI_wifiConnected(void);
bool NxtNextUI_audioConfig(char *, size_t);
bool NxtNextUI_wrapRoute(const char *, char *, size_t);
#endif
