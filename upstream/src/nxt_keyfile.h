/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_KEYFILE_H
#define NXT_KEYFILE_H
#include <stddef.h>
/* 0 success, 1 file missing, 2 invalid/unreadable. Never logs the key. */
int NxtKeyFile_readGroq(const char *path,char *output,size_t capacity);
#endif
