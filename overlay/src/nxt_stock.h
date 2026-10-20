/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_STOCK_H
#define NXT_STOCK_H
#include <stdbool.h>
#ifdef NXT_ROM_STOCK
bool NxtStock_wifiConnected(void);
#define NXT_FIRMWARE_NAME "Stock"
void NxtStock_init(void);
void NxtStock_visible(bool visible);
void NxtStock_tick(void);
void NxtStock_quit(void);
int NxtStock_info(void);
bool NxtStock_bluetoothRoute(char *out, unsigned long size);
bool NxtStock_requestPanel(void);
bool NxtStock_panelActive(void);
#else
#ifdef NXT_ROM_NXREDUX
#define NXT_FIRMWARE_NAME "NX Redux"
#else
#define NXT_FIRMWARE_NAME "NextUI"
#endif
static inline void NxtStock_init(void) {}
static inline void NxtStock_visible(bool visible) { (void)visible; }
static inline void NxtStock_tick(void) {}
static inline void NxtStock_quit(void) {}
#endif
#endif
