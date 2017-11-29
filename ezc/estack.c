
#include "estack.h"
#include <stdio.h>
#include <stdlib.h>


void estack_init(estack_t * estack) {
    estack->len = 0;
    estack->vals = NULL;
}

void estack_push(estack_t * estack, obj_t * obj) {
    estack->len++;
    if (estack->vals == NULL) {
        estack->vals = (obj_t **)malloc(sizeof(obj_t *) * estack->len);
    } else {
        estack->vals = (obj_t **)realloc(estack->vals, sizeof(obj_t *) * estack->len);
    }
    
    estack->vals[estack->len - 1] = obj;
}

