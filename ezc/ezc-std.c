// ezc-std.c - the standard library for EZC

#define EZC_MODULE_NAME std

#include "ezc-impl.h"
#include "ezc-module.h"

#include <math.h>

/* utility macros */

// require N arguments
#define REQ_N(_n) if (ctx->stk.n < _n) { ezc_error("%d items are required, stack only had %d", _n, ctx->stk.n); return 1; }

#define OBJ_T(_obj) (ctx->types[_obj.type])
#define OBJ_INIT(_obj) (OBJ_T(_obj).f_init(&_obj))
#define OBJ_FREE(_obj) (OBJ_T(_obj).f_free(&_obj))
#define OBJ_COPY(_obj, _from) { _obj.type = _from.type; OBJ_T(_obj).f_copy(&_obj, &_from); }
#define OBJ_REPR(_obj, _str) (OBJ_T(_obj).f_repr(&_obj, &_str))

#define OBJ_TRUTHY(_obj, _val) { if (_obj.type == EZC_TYPE_BOOL) { _val = _obj.bool_s; } else if (_obj.type == EZC_TYPE_INT) { _val = _obj.int_s != 0; } else { _val = false; } }

#define TYPE_NAME(_obj) (ctx->type_names[_obj.type])


/* TYPE DEFINITIONS */

/* none */

EZC_TF_INIT(none) {
    return 0;
}

EZC_TF_FREE(none) {
    // do nothing
    return 0;
}

EZC_TF_REPR(none) {
    ezc_str_from(str, "none", 0);
    return 0;
}

EZC_TF_COPY(none) {
    return 0;
}

/* none */

EZC_TF_INIT(wall) {
    return 0;
}

EZC_TF_FREE(wall) {
    // do nothing
    return 0;
}

EZC_TF_REPR(wall) {
    ezc_str_from(str, "*WALL", 0);
    return 0;
}

EZC_TF_COPY(wall) {
    return 0;
}

/* int */

EZC_TF_INIT(int) {
    obj->int_s = 0;
    return 0;
}

EZC_TF_FREE(int) {
    // do nothing
    return 0;
}

EZC_TF_REPR(int) {
    static const char basestr[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    ezc_int val = obj->int_s;
    int base = 10;

    // output here
    char strs[100];

    int i = 0;
    bool is_neg = false;

    if (val < 0) {
        val = -val;
        strs[i++] = '-';
        is_neg = true;
    }

    do {
        strs[i++] = basestr[val % base];
        val /= base;
    } while (val > 0);

    // now, reverse it
    int len = i;
    for (i = is_neg ? 1 : 0; 2 * i < len; ++i) {
        char tmp = strs[i];
        strs[i] = strs[len-i-1];
        strs[len-i-1] = tmp;
    }

    ezc_str_from(str, strs, len);

    return 0;
}

EZC_TF_COPY(int) {
    obj->int_s = from->int_s;
    return 0;
}

/* bool */

EZC_TF_INIT(bool) {
    obj->bool_s = false;
    return 0;
}

EZC_TF_FREE(bool) {
    // do nothing
    return 0;
}

EZC_TF_REPR(bool) {
    if (obj->bool_s) {
        ezc_str_from(str, "true", 0);
    } else {
        ezc_str_from(str, "false", 0);
    }
    return 0;
}

EZC_TF_COPY(bool) {
    obj->bool_s = from->bool_s;
    return 0;
}
/* real */

EZC_TF_INIT(real) {
    obj->real_s = 0;
    return 0;
}

EZC_TF_FREE(real) {
    // do nothing
    return 0;
}

EZC_TF_REPR(real) {
    char strs[100];
    sprintf(strs, "%lf", obj->real_s);
    ezc_str_from(str, strs, 0);
    return 0;
}

EZC_TF_COPY(real) {
    obj->real_s = from->real_s;
    return 0;
}

/* str */

EZC_TF_INIT(str) {
    obj->str_s = EZC_STR_NULL;
    return 0;
}

EZC_TF_FREE(str) {
    ezc_str_free(&obj->str_s);
    return 0;
}

EZC_TF_REPR(str) {
    ezc_str_copy(str, obj->str_s);
    return 0;
}

EZC_TF_COPY(str) {
    obj->str_s = EZC_STR_NULL;
    ezc_str_copy(&obj->str_s, from->str_s);
    return 0;
}

/* block type */

EZC_TF_INIT(block) {
    obj->custom_s = NULL;
    return 0;
}

EZC_TF_FREE(block) {
    // do nothing
    return 0;
}

EZC_TF_REPR(block) {
    char strs[100];
    sprintf(strs, "{...[%d]}", ((struct ezci_block*)obj->custom_s)->n);
    ezc_str_from(str, strs, 0);
    return 0;
}

EZC_TF_COPY(block) {
    obj->custom_s = from->custom_s;
    return 0;
}

/* functions in this module */

EZC_FUNC(none) {
    ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_NONE });
    return 0;
}

EZC_FUNC(wall) {
    ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_WALL });
    return 0;
}

EZC_FUNC(del) {
    REQ_N(1);
    ezc_obj popped = ezc_stk_pop(&ctx->stk);
    OBJ_FREE(popped);
    return 0;
}

EZC_FUNC(funcdef) {
    REQ_N(1);
    ezc_obj f_name = ezc_stk_pop(&ctx->stk);
    ezc_obj f_body = ezc_stk_pop(&ctx->stk);

    if (f_name.type == EZC_TYPE_STR) {
        ezci i_body = (ezci){ .type = EZCI_BLOCK };
        i_body._block = *(struct ezci_block*)f_body.custom_s;
        ezc_ctx_addfunc(ctx, f_name.str_s, EZC_FUNC_EZC(i_body));
        OBJ_FREE(f_name);
        return 0;
    } else {
        ezc_error("funcdef name is not 'str'");
        OBJ_FREE(f_name);
        OBJ_FREE(f_body);
        return 1;
    }
}

EZC_FUNC(dup) {
    REQ_N(1);
    ezc_obj top = ezc_stk_peek(&ctx->stk);

    ezc_obj new_obj = (ezc_obj){ .type = top.type };
    OBJ_COPY(new_obj, top);
    ezc_stk_push(&ctx->stk, new_obj);
    return 0;
}

EZC_FUNC(under) {
    REQ_N(2);
    ezc_obj under = ezc_stk_peekn(&ctx->stk, 1);

    ezc_obj new_obj = EZC_OBJ_NONE;
    OBJ_COPY(new_obj, under);
    ezc_stk_push(&ctx->stk, new_obj);
    return 0;
}

EZC_FUNC(swap) {
    REQ_N(2);
    ezc_stk_swap(&ctx->stk, ctx->stk.n-1, ctx->stk.n-2);
    return 0;
}

EZC_FUNC(exec) {
    REQ_N(1);
    ezc_obj popped = ezc_stk_pop(&ctx->stk);
    int stat = 0;

    if (popped.type == EZC_TYPE_STR) {
        // execute the function byt this name
        int idx = ezc_ctx_getfunci(ctx, popped.str_s);
        if (idx < 0) { 
            ezc_error("Unknown function: '%s'", popped.str_s._);
            OBJ_FREE(popped);
            return 1;
        } else {

            ezc_func to_exec = ctx->funcs[idx];
            //ctx->funcs[idx](ctx);
            if (to_exec.type == EZC_FUNC_TYPE_C) {
                OBJ_FREE(popped);
                return to_exec._c(ctx);
            } else if (to_exec.type == EZC_FUNC_TYPE_EZC) {
                ezcp _prog;
                _prog.src = EZC_STR_NULL;
                _prog.src_name = EZC_STR_NULL;
                _prog.body = to_exec._ezc;
                stat = ezc_exec(ctx, &_prog);
                OBJ_FREE(popped);
                return stat;
            }
        }
    } else if (popped.type == EZC_TYPE_BLOCK) {
        struct ezci_block* block = (struct ezci_block*)popped.custom_s;
        ezcp _prog;
        _prog.src = EZC_STR_NULL;
        _prog.src_name = EZC_STR_NULL;
        _prog.body = (ezci){ .type = EZCI_BLOCK };
        _prog.body._block.children = block->children;
        _prog.body._block.n = block->n;
        //printf("BLOCK:%d\n", block->n);

        stat = ezc_exec(ctx, &_prog);
        return stat;
    } else {
        // TODO: account for blocks and things, but for now, just ezc_error
        ezc_error("Invalid type: '%s'", ctx->type_names[popped.type]._);
        OBJ_FREE(popped);
        return 1;
    }
}

EZC_FUNC(repr) {
    REQ_N(1);
    ezc_obj popped = ezc_stk_pop(&ctx->stk);

    // get the repr
    ezc_obj new_str = (ezc_obj){ .type = EZC_TYPE_STR, .str_s = EZC_STR_NULL };
    OBJ_REPR(popped, new_str.str_s);

    // push it onto the stack
    ezc_stk_push(&ctx->stk, new_str);

    OBJ_FREE(popped);
    return 0;
}

EZC_FUNC(print) {
    REQ_N(1);

    // just go ahead and replace the last object with its representation
    EZC_FUNC_NAME(repr)(ctx);

    ezc_obj popped = ezc_stk_pop(&ctx->stk);

    if (popped.type == EZC_TYPE_STR) {
        ezc_print("%s", popped.str_s._);
        OBJ_FREE(popped);
        return 0;
    } else {
        // repr should always give a string on top of the stack, so this shouldn't happen
        ezc_error("Internal; `repr!` didn't give a string");
        OBJ_FREE(popped);
        return 1;
    }
}

EZC_FUNC(dump) {
    ezc_str str = EZC_STR_NULL;
    int i;
    for (i = ctx->stk.n - 1; i >= 0; --i) {
        ezc_obj cur = ezc_stk_get(&ctx->stk, i);
        OBJ_REPR(cur, str);
        ezc_print("%*d: %s", 2, i, str._);
    }
    ezc_str_free(&str);

    ezc_printr("-----\nstack\n");
}

/* math functions */

EZC_FUNC(add) {
    REQ_N(2);
    // compute A+B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_STR) {
            ezc_str new_str = EZC_STR_NULL;
            ezc_str_concat(&new_str, A.str_s, B.str_s);
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_STR, .str_s = new_str });
            return 0;
        } else if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = A.int_s + B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = A.real_s + B.real_s });
        } else {
            ezc_error("Invalid type for func `add`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `add`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}

EZC_FUNC(sub) {
    REQ_N(2);
    // compute A-B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = A.int_s - B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = A.real_s - B.real_s });
        } else {
            ezc_error("Invalid type for func `sub`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `sub`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}

EZC_FUNC(mul) {
    REQ_N(2);
    // compute A*B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = A.int_s * B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = A.real_s * B.real_s });
        } else {
            ezc_error("Invalid type for func `mul`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `mul`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}

EZC_FUNC(div) {
    REQ_N(2);
    // compute A/B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = A.int_s / B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = A.real_s / B.real_s });
        } else {
            ezc_error("Invalid type for func `div`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `div`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}

EZC_FUNC(mod) {
    REQ_N(2);
    // compute A%B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = A.int_s % B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = fmod(A.real_s, B.real_s) });
        } else {
            ezc_error("Invalid type for func `mod`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `mod`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}

EZC_FUNC(pow) {
    REQ_N(2);
    // compute A^B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            // r = a^b
            ezc_int a = A.int_s;
            ezc_int b = B.int_s;
            ezc_int r = 1;

            if (b < 0) r = 0;
            else if (b == 1) r = a;
            else {
                // a^2^iter
                ezc_int a2iter = a;
                do {
                    if (b & 1) r *= a2iter;
                    a2iter *= a2iter;
                    b >>= 1;
                } while (b > 0);
            }
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_INT, .int_s = r });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_REAL, .real_s = pow(A.real_s, B.real_s) });
        } else {
            ezc_error("Invalid type for func `pow`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `pow`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}


/* comparison functions */

EZC_FUNC(eq) {
    REQ_N(2);
    // compute A==B
    ezc_obj B = ezc_stk_pop(&ctx->stk);
    ezc_obj A = ezc_stk_pop(&ctx->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_BOOL, .bool_s = A.int_s == B.int_s });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&ctx->stk, (ezc_obj){ .type = EZC_TYPE_BOOL, .real_s = A.real_s == B.real_s });
        } else {
            ezc_error("Invalid type for func `eq`: %s", TYPE_NAME(A)._);
            OBJ_FREE(A); OBJ_FREE(B);
            return 1;
        }
        OBJ_FREE(A); OBJ_FREE(B);
        return 0;
    } else {
        ezc_error("Invalid type combo for func `eq`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._);
        OBJ_FREE(A); OBJ_FREE(B);
        return 1;
    }
}


/* control loops */
EZC_FUNC(ifel) {
    REQ_N(3);
    // | condition {code if true} {code if not} ifel!

    ezc_obj b_else = ezc_stk_pop(&ctx->stk);
    ezc_obj b_if = ezc_stk_pop(&ctx->stk);
    ezc_obj cond = ezc_stk_pop(&ctx->stk);

    bool cond_val = false;
    OBJ_TRUTHY(cond, cond_val);

    if (cond_val) {
        ezc_stk_push(&ctx->stk, b_if);
        EZC_FUNC_NAME(exec)(ctx);
    } else {
        ezc_stk_push(&ctx->stk, b_else);
        EZC_FUNC_NAME(exec)(ctx);
    }

    OBJ_FREE(b_else);
    OBJ_FREE(b_if);
    OBJ_FREE(cond);
    return 0;
}

EZC_FUNC(foreach) {
    REQ_N(2);
    // | a b c ... d {body} foreach!
    // runs body for each a, b, c
    ezc_obj body = ezc_stk_pop(&ctx->stk);

    // start another temporary stack
    ezc_stk argstack = EZC_STK_NONE;

    // now, keep going until stack hits wall or is empty
    // pop onto the arg stack, which are the items we are iterating over
    while (ctx->stk.n > 0 && ezc_stk_peek(&ctx->stk).type != EZC_TYPE_WALL) {
        ezc_stk_push(&argstack, ezc_stk_pop(&ctx->stk));
    }

    // consume the wall if used
    if (ezc_stk_peek(&ctx->stk).type == EZC_TYPE_WALL) {
        ezc_stk_pop(&ctx->stk);
    }

    while (argstack.n > 0) {
        // process the current argument
        ezc_stk_push(&ctx->stk, ezc_stk_pop(&argstack));

        // evaluate the block here:
        ezc_obj copy_body = EZC_OBJ_NONE;
        OBJ_COPY(copy_body, body);
        ezc_stk_push(&ctx->stk, copy_body);
        EZC_FUNC_NAME(exec)(ctx);
        // the exec function should take care of it, freeing it
        
    }

    // free the temporary stack
    ezc_stk_free(&argstack);

    return 0;
}

// generators

EZC_FUNC(expand) {
    REQ_N(1);
    ezc_obj arg = ezc_stk_pop(&ctx->stk);

    if (arg.type == EZC_TYPE_INT) {
        // do 0...arg-1
        ezc_int i;
        ezc_obj new_int = EZC_OBJ_NONE;
        new_int.type = EZC_TYPE_INT;
        for (i = arg.int_s - 1; i >= 0; --i) {
            new_int.int_s = i;
            ezc_stk_push(&ctx->stk, new_int);
        }
        OBJ_FREE(arg);
        return 0;        
    } else {
        // TOOD: add strings, etc support
        OBJ_FREE(arg);
        ezc_error("Unsupported type");
        return 2;
    }

}

// register this type
int EZC_FUNC_NAME(register_module)(ezc_ctx* ctx) {

    /* register the built in types. MUST BE IN THIS ORDER */

    EZC_REGISTER_TYPE(ctx, none)
    EZC_REGISTER_TYPE(ctx, wall)
    EZC_REGISTER_TYPE(ctx, bool)
    EZC_REGISTER_TYPE(ctx, int)
    EZC_REGISTER_TYPE(ctx, real)
    EZC_REGISTER_TYPE(ctx, str)
    EZC_REGISTER_TYPE(ctx, block)

    // functions

    EZC_REGISTER_FUNC(ctx, none)
    EZC_REGISTER_FUNC(ctx, wall)
    EZC_REGISTER_FUNC(ctx, del)
    EZC_REGISTER_FUNC(ctx, funcdef)
    EZC_REGISTER_FUNC(ctx, dup)
    EZC_REGISTER_FUNC(ctx, swap)
    EZC_REGISTER_FUNC(ctx, exec)
    EZC_REGISTER_FUNC(ctx, repr)
    EZC_REGISTER_FUNC(ctx, print)
    EZC_REGISTER_FUNC(ctx, dump)

    EZC_REGISTER_FUNC(ctx, add)
    EZC_REGISTER_FUNC(ctx, sub)
    EZC_REGISTER_FUNC(ctx, mul)
    EZC_REGISTER_FUNC(ctx, div)
    EZC_REGISTER_FUNC(ctx, mod)
    EZC_REGISTER_FUNC(ctx, pow)

    EZC_REGISTER_FUNC(ctx, under)
    EZC_REGISTER_FUNC(ctx, eq)

    EZC_REGISTER_FUNC(ctx, ifel)
    EZC_REGISTER_FUNC(ctx, foreach)

    EZC_REGISTER_FUNC(ctx, expand)

}


