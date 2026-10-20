/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_sort.h"
#include "nxt_text.h"
#include <stdlib.h>
#include <string.h>

static int natural(const unsigned char *a, const unsigned char *b) {
    while (*a && *b) {
        if (*a >= '0' && *a <= '9' && *b >= '0' && *b <= '9') {
            const unsigned char *as=a, *bs=b;
            while (*a=='0') a++;
            while (*b=='0') b++;
            const unsigned char *ae=a, *be=b;
            while (*ae>='0' && *ae<='9') ae++;
            while (*be>='0' && *be<='9') be++;
            if (ae-a != be-b) return ae-a < be-b ? -1 : 1;
            int result=memcmp(a,b,ae-a);
            if (result) return result;
            /* Equal values have a deterministic leading-zero order. */
            if (ae-as != be-bs) return ae-as < be-bs ? -1 : 1;
            a=ae; b=be;
        } else {
            if (*a != *b) return *a < *b ? -1 : 1;
            a++; b++;
        }
    }
    return (*a > *b) - (*a < *b);
}

int NxtSort_compare(const char *a, const char *b) {
    if (!a) a="";
    if (!b) b="";
    char *ak=NxtText_searchKey(a), *bk=NxtText_searchKey(b);
    int result=natural((const unsigned char *)(ak?ak:a),
                       (const unsigned char *)(bk?bk:b));
    free(ak); free(bk);
    return result ? result : strcmp(a,b);
}
int NxtSort_strings(const void *a, const void *b) {
    return NxtSort_compare(*(const char *const *)a,*(const char *const *)b);
}
