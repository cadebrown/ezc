
#include "estack.h"
#include <stdio.h>
#include <stdlib.h>


void estack_init(estack_t * estack) {
    estack->len = 0;
    estack->__max_len_so_far = 0;
    estack->vals = NULL;
}

void estack_push(estack_t * estack, obj_t obj) {
    estack->len++;
    if (estack->vals == NULL) {
        estack->vals = (obj_t *)malloc(sizeof(obj_t) * estack->len);
    } else if (estack->len > estack->__max_len_so_far) {
        estack->vals = (obj_t *)realloc(estack->vals, sizeof(obj_t) * estack->len);
        estack->__max_len_so_far = estack->len;
    }
    
    estack->vals[estack->len - 1] = obj;
}

obj_t estack_pop(estack_t * estack) {
    if (estack->len <= 0 || estack->vals == NULL) {
        return NULL_OBJ;
    } else {
        obj_t ret = estack->vals[estack->len - 1];
        estack->vals[--estack->len] = NULL_OBJ;
        return ret;
    }
}

obj_t estack_get(estack_t * estack, int i) {
    if (i < 0 || i >= estack->len) {
        return NULL_OBJ;
    } else {
        return estack->vals[i];
    }
}
