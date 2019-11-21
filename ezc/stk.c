
#include "ezc-impl.h"

void ezc_stk_free(ezc_stk* stk) {
    ezc_free(stk->base);
    *stk = EZC_STK_EMPTY;
}

void ezc_stk_resize(ezc_stk* stk, int new_n) {
    if (new_n <= stk->n) {
        stk->n = new_n;
    } else {
        int start_n = stk->n;
        stk->n = new_n;
        if (stk->n > stk->max_n) {
            stk->max_n = (int)(1.5 * stk->n + 10);
            stk->base = ezc_realloc(stk->base, sizeof(ezc_obj) * stk->max_n);
        }

        int i;
        for (i = start_n; i < stk->n; ++i) {
            stk->base[i] = EZC_OBJ_EMPTY;
        }
    }
}

int ezc_stk_push(ezc_stk* stk, ezc_obj obj) {
    int idx = stk->n++;
    if (stk->n > stk->max_n) {
        stk->max_n = (int)(1.5 * stk->n + 10);
        stk->base = ezc_realloc(stk->base, sizeof(ezc_obj) * stk->max_n);
    }
    stk->base[idx] = obj;
    return idx;
}

ezc_obj ezc_stk_pop(ezc_stk* stk) {
    return stk->base[--stk->n];
}

ezc_obj ezc_stk_get(ezc_stk* stk, int idx) {
    return stk->base[idx];
}

void ezc_stk_swap(ezc_stk* stk, int idx, int jdx) {
    ezc_obj tmp = stk->base[idx];
    stk->base[idx] = stk->base[jdx];
    stk->base[jdx] = tmp;
}



