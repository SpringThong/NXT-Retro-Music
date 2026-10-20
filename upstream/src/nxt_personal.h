/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_PERSONAL_H
#define NXT_PERSONAL_H
#include "nxt.h"
void NxtPersonal_init(void);
void NxtPersonal_tick(void);
void NxtPersonal_flush(void);
void NxtPersonal_quit(void);
int NxtPersonal_id(void);
const char *NxtPersonal_name(void);
const char *NxtPersonal_avatar(void);
int NxtPersonal_route(const char *,char *,size_t);
void NxtPersonal_savePreferences(void);
const char *NxtPersonal_country(int);
ModuleExitReason NxtPersonal_checkIn(DisplayContext *);
ModuleExitReason NxtPersonal_login(DisplayContext *);
void NxtPersonal_preferencesPath(char *,size_t);
ModuleExitReason NxtPersonal_run(DisplayContext *);
ModuleExitReason NxtPersonal_forYou(DisplayContext *);
ModuleExitReason NxtPersonal_statistics(DisplayContext *);
#endif
