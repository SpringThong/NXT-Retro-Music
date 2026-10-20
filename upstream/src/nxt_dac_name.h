/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_DAC_NAME_H
#define NXT_DAC_NAME_H
#include <stddef.h>
#include <stdbool.h>
typedef struct {const char *vid,*pid,*manufacturer,*product;} NxtDACIdentity;
void NxtDACName_read(const char *path,const NxtDACIdentity*,char *out,size_t size);
bool NxtDACName_save(const char *path,const NxtDACIdentity*,const char *name);
#endif
