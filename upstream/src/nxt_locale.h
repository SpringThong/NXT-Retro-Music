#ifndef NXT_LOCALE_H
#define NXT_LOCALE_H
#include "nxt.h"
const char *NxtLocale_text(const char *);
const char *NxtLocale_name(int);
ModuleExitReason NxtLocale_run(DisplayContext *);
#endif
