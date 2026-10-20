/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_USB_H
#define NXT_USB_H
#include <stdbool.h>
/* 0 unknown, 1 USB host, 2 USB device. Status reads never touch action nodes. */
int NxtUSB_role(const char *base);
/* 0 failed, 1 already selected, 2 confirmed switch, 3 request read; role not yet confirmed. */
int NxtUSB_switch(const char *base,bool host);
#endif
