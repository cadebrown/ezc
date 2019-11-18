
#include "ezc-impl.h"
#include "ezc-module.h"

int ezc_exec(ezc_ctx* ctx, ezcp* prog) {
    // required functions
    int _bidx;
    #define EZC_BUILTIN(name) ezc_func bi_##name = ctx->funcs[_bidx = ezc_ctx_getfunci(ctx, EZC_STR_CONST(#name))]; if (_bidx < 0 || (bi_##name).type != EZC_FUNC_TYPE_C) { ezc_error("Couldn't find builtin function '%s'", #name); return 1; }
    #define RUN_BUILTIN(name) if ((status = bi_##name._c(ctx)) != 0) { ezc_printmeta(cur.meta); exit(1); }
    #define CASE_BUILTIN(code, name) else if (cur.type == code) { RUN_BUILTIN(name) }

    EZC_BUILTIN(exec)
    EZC_BUILTIN(dup)
    EZC_BUILTIN(dump)
    EZC_BUILTIN(swap)
    EZC_BUILTIN(add)
    EZC_BUILTIN(sub)
    EZC_BUILTIN(mul)
    EZC_BUILTIN(div)
    EZC_BUILTIN(mod)
    EZC_BUILTIN(pow)
    EZC_BUILTIN(under)
    EZC_BUILTIN(wall)
    EZC_BUILTIN(eq)

    int status = 0;
    int i;
    for (i = 0; i < prog->body._block.n; ++i) {
        ezci cur = prog->body._block.children[i];
        if (cur.type == EZCI_INT) {
            ezc_obj new_int = (ezc_obj){ .type = EZC_TYPE_INT, .int_s = cur._int };
            ezc_stk_push(&ctx->stk, new_int);
        } else if (cur.type == EZCI_REAL) {
            ezc_obj new_real = (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = cur._real };
            ezc_stk_push(&ctx->stk, new_real);
        } else if (cur.type == EZCI_STR) {
            ezc_obj new_str = (ezc_obj){ .type = EZC_TYPE_STR };
            ezc_str_copy(&new_str.str_s, cur._str);
            ezc_stk_push(&ctx->stk, new_str);
        } else if (cur.type == EZCI_BLOCK) {
            ezc_obj new_block = (ezc_obj){ .type = EZC_TYPE_BLOCK };
            new_block.custom_s = (void*)&(prog->body._block.children[i]._block);
            ezc_stk_push(&ctx->stk, new_block);
        } 
        CASE_BUILTIN(EZCI_BANG, exec)
        CASE_BUILTIN(EZCI_COPY, dup)
        CASE_BUILTIN(EZCI_ADD, add)
        CASE_BUILTIN(EZCI_SUB, sub)
        CASE_BUILTIN(EZCI_MUL, mul)
        CASE_BUILTIN(EZCI_DIV, div)
        CASE_BUILTIN(EZCI_MOD, mod)
        CASE_BUILTIN(EZCI_POW, pow)
        CASE_BUILTIN(EZCI_UNDER, under)
        CASE_BUILTIN(EZCI_EQ, eq)
        CASE_BUILTIN(EZCI_SWAP, swap)
        CASE_BUILTIN(EZCI_WALL, wall)
        else if (cur.type == EZCI_NOOP) {
            // do nothing
        } else {
            ezc_warn("Unhandled instruction type!");
        }

        /*if (i == prog->body._block.n - 1) {
            RUN_BUILTIN(dump);
        }*/
    }
    return 0;
}



