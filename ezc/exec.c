// ezc/exec.c - file for execution on the EZC VM
//
// This is the core of the EZC execution context
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-19
//

#include "ezc-impl.h"

// executes on a VM
int ezc_vm_exec(ezc_vm* vm, ezcp prog) {
    ezc_trace("ezc_vm_exec(%p, {...})", vm);

    // macros to declare required 'builtins'
    #define EZC_BUILTIN(name) int bidx_##name = -1; ezc_func bi_##name;
    #define RUN_BUILTIN(name) if (bidx_##name < 0) { \
        bidx_##name = ezc_vm_getfunci(vm, EZC_STR_CONST(#name)); \
        if (bidx_##name < 0) { \
            ezc_error("Couldn't find builtin function '%s'", #name); \
            ezc_printmeta(cur); \
            return 1; \
        } else { \
            bi_##name = vm->funcs.vals[bidx_##name]; \
        } \
    } \
    if ((status = bi_##name._c(vm)) != 0) { return status; } \

    // macro to simplify the `else if`s
    #define CASE_BUILTIN(code, name) else if (cur.type == code) { RUN_BUILTIN(name) }

    // list of builtins callable
    EZC_BUILTIN(exec)
    EZC_BUILTIN(copy)
    EZC_BUILTIN(del)
    EZC_BUILTIN(dump)
    EZC_BUILTIN(swap)
    EZC_BUILTIN(add)
    EZC_BUILTIN(sub)
    EZC_BUILTIN(mul)
    EZC_BUILTIN(div)
    EZC_BUILTIN(mod)
    EZC_BUILTIN(pow)
    EZC_BUILTIN(under)
    EZC_BUILTIN(get)
    EZC_BUILTIN(wall)
    EZC_BUILTIN(eq)

    int status = 0;
    int i;
    for (i = 0; i < prog.body._block.n; ++i) {
        // get current instruction
        ezci cur = prog.body._block.children[i];
        if (cur.type == EZCI_INT) {
            ezc_obj new_int = (ezc_obj){ .type = EZC_TYPE_INT, ._int = cur._int };
            ezc_stk_push(&vm->stk, new_int);
        } else if (cur.type == EZCI_REAL) {
            ezc_obj new_real = (ezc_obj){ .type = EZC_TYPE_REAL, ._real = cur._real };
            ezc_stk_push(&vm->stk, new_real);
        } else if (cur.type == EZCI_STR) {
            ezc_obj new_str = (ezc_obj){ .type = EZC_TYPE_STR };
            ezc_str_copy(&new_str._str, cur._str);
            ezc_stk_push(&vm->stk, new_str);
        } else if (cur.type == EZCI_BLOCK) {
            ezc_obj new_block = (ezc_obj){ .type = EZC_TYPE_BLOCK };
            new_block._block = prog.body._block.children[i];
            ezc_stk_push(&vm->stk, new_block);
        }
        // all the builtins
        CASE_BUILTIN(EZCI_EXEC, exec)
        CASE_BUILTIN(EZCI_DEL, del)
        CASE_BUILTIN(EZCI_COPY, copy)
        CASE_BUILTIN(EZCI_ADD, add)
        CASE_BUILTIN(EZCI_SUB, sub)
        CASE_BUILTIN(EZCI_MUL, mul)
        CASE_BUILTIN(EZCI_DIV, div)
        CASE_BUILTIN(EZCI_MOD, mod)
        CASE_BUILTIN(EZCI_POW, pow)
        CASE_BUILTIN(EZCI_UNDER, under)
        CASE_BUILTIN(EZCI_GET, get)
        CASE_BUILTIN(EZCI_EQ, eq)
        CASE_BUILTIN(EZCI_SWAP, swap)
        CASE_BUILTIN(EZCI_WALL, wall)
        else if (cur.type == EZCI_NONE) {
            // do nothing
        } else {
            ezc_warn("Unhandled instruction type!");
        }
    }

    // return 0 (success)
    return 0;
}



