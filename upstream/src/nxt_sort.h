/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_SORT_H
#define NXT_SORT_H
/* Natural numbers, case/diacritic-insensitive Vietnamese and Latin names. */
int NxtSort_compare(const char *a, const char *b);
int NxtSort_strings(const void *a, const void *b);
#endif
