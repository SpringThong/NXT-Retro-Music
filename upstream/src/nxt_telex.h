/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_TELEX_H
#define NXT_TELEX_H
#include <stdbool.h>
#include <stddef.h>
#define NXT_TELEX_CAPACITY 1024
typedef struct { char raw[NXT_TELEX_CAPACITY]; size_t literal_bytes; } NxtTelex;
bool NxtTelex_append(NxtTelex *, char *out, size_t capacity, const char *input);
bool NxtTelex_backspace(NxtTelex *, char *out, size_t capacity);
void NxtTelex_reset(NxtTelex *, const char *text);
#endif
