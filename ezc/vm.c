
#include "ezc-impl.h"

void ezc_vm_free(ezc_vm* vm) {
    ezc_stk_free(&vm->stk);
}


int ezc_vm_addfunc(ezc_vm* vm, ezc_str name, ezc_func func) {
    int idx = vm->funcs.n++;
    vm->funcs.keys = ezc_realloc(vm->funcs.keys, sizeof(ezc_str) * vm->funcs.n);
    vm->funcs.vals = ezc_realloc(vm->funcs.vals, sizeof(ezc_func) * vm->funcs.n);

    vm->funcs.keys[idx] = EZC_STR_NULL;
    ezc_str_copy(vm->funcs.keys + idx, name);

    vm->funcs.vals[idx] = func;

    return idx; 
}

int ezc_vm_addtype(ezc_vm* vm, ezc_str name, ezct type) {
    int idx = vm->types.n++;
    vm->types.keys = ezc_realloc(vm->types.keys, sizeof(ezc_str) * vm->types.n);
    vm->types.vals = ezc_realloc(vm->types.vals, sizeof(ezct) * vm->types.n);

    vm->types.keys[idx] = EZC_STR_NULL;
    ezc_str_copy(vm->types.keys + idx, name);
    vm->types.vals[idx] = type;

    return idx; 
}

// returns index of function, or -1
int ezc_vm_getfunci(ezc_vm* vm, ezc_str name) {
    int i;
    for (i = 0; i < vm->funcs.n; ++i) {
        if (ezc_str_eq(name, vm->funcs.keys[i])) {
            return i;
        }
    }
    return -1;
}

int ezc_vm_gettypei(ezc_vm* vm, ezc_str name) {
    int i;
    for (i = 0; i < vm->types.n; ++i) {
        if (ezc_str_eq(name, vm->types.keys[i])) {
            return i;
        }
    }
    return -1;
}
/*

void ezc_vm_free(ezc_vm* vm) {
    ezc_stk_free(&vm->stk);
    ezc_free(vm->types);
    ezc_free(vm->type.keys);
    ezc_free(vm->func.keys);
    ezc_free(vm->funcs);
    *vm = EZC_vm_NONE;
}



*/

