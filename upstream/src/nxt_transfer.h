#ifndef NXT_TRANSFER_H
#define NXT_TRANSFER_H
#include "nxt.h"
bool NxtTransfer_start(const char *);
void NxtTransfer_stop(void);
bool NxtTransfer_isBusy(void);
void NxtTransfer_status(char *,size_t,char *,size_t,unsigned long long *,unsigned long long *);
ModuleExitReason NxtTransfer_run(DisplayContext *);
#endif
