#ifndef NXT_IME_H
#define NXT_IME_H
#include <stdbool.h>
#include <stddef.h>
bool NxtIME_compose(int,const char *,char *,size_t);
int NxtIME_candidates(int,const char *,char [32][128]);
#endif
