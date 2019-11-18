
#include "ezc-impl.h"


void ezc_stk_init(ezc_stk* stk) {
    stk->base = NULL;
    stk->n = 0;
    stk->__max = 0;
}

void ezc_stk_free(ezc_stk* stk) {
    ezc_free(stk->base);
    stk->base = NULL;
    stk->n = 0;
    stk->__max = 0;
}

int ezc_stk_push(ezc_stk* stk, ezc_obj obj) {
    if (++stk->n > stk->__max) {
        stk->__max = stk->n + 10;
        stk->base = ezc_realloc(stk->base, sizeof(ezc_obj) * stk->__max);
    }
    stk->base[stk->n - 1] = obj;
    return stk->n - 1;
}

ezc_obj ezc_stk_pop(ezc_stk* stk) {
    return stk->base[--stk->n];
}

ezc_obj ezc_stk_get(ezc_stk* stk, int idx) {
    return stk->base[idx];
}

ezc_obj ezc_stk_peek(ezc_stk* stk) {
    return stk->base[stk->n - 1];
}

void ezc_stk_swap(ezc_stk* stk, int idx, int jdx) {
    ezc_obj tmp = stk->base[idx];
    stk->base[idx] = stk->base[jdx];
    stk->base[jdx] = tmp;
}

ezc_obj ezc_stk_peekn(ezc_stk* stk, int back) {
    return stk->base[stk->n - 1 - back];
}





