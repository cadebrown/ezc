// ezc/ezc-std.h - the standard library for EZC, implementing builtin types
//                   and functions, math functionality, etc
//
// This should be included in the `libezc` binary, and all functions have a 
//   prefix of `F_std_`, so the exec function can be executed on an ezc_vm,
//   with `status = F_std_exec(&vm);`
//
// To include this without dynamic linking (which shouldn't be used; std is
//   "special" and is included in libezc), just #define EZC_MODULE_NAME std
//   and include the file `ezc-module.h`
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-19
//

#include "ezc.h"

#define EZC_MODULE_NAME std
#include "ezc-module.h"

// uses a few functions from the standard library, like:
//   * sprintf (for integer/real formatting)
#include <stdio.h>
// uses a few functions from the math library, like:
//   * fmod (for the `%` operator)
//   * pow (for the `^` operator)
//   * sqrt,sin,cos,... (for math library)
#include <math.h>

/* utility macros */

// a macro to require a specific number of arguments, if not, print an error message
//   and return 
#define REQ_N(_fname, _n) if (vm->stk.n < _n) { ezc_error("%d items are required for function '" #_fname "', stack only had %d", _n, vm->stk.n); return 1; }

#define OBJ_T(_obj) (vm->types.vals[_obj.type])
#define OBJ_INIT(_obj) (OBJ_T(_obj).f_init(&_obj))
#define OBJ_FREE(_obj) (OBJ_T(_obj).f_free(&_obj))
#define OBJ_COPY(_obj, _from) { _obj.type = _from.type; OBJ_T(_obj).f_copy(&_obj, &_from); }
#define OBJ_REPR(_obj, _str) (OBJ_T(_obj).f_repr(&_obj, &_str))

#define OBJ_TRUTHY(_obj, _val) { if (_obj.type == EZC_TYPE_BOOL) { _val = _obj._bool; } else if (_obj.type == EZC_TYPE_INT) { _val = _obj._int != 0; } else { _val = false; } }

#define TYPE_NAME(_obj) (vm->types.keys[_obj.type])



/* static constants */

// the characters representings digits in different bases
static const char digitstr[] = "0123456789abcdefghijklmnopqrstuvwxyz";


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
    ezc_str_copy_cp(str, "none", 4);
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
    ezc_str_copy_cp(str, "|", 1);
    return 0;
}

EZC_TF_COPY(wall) {
    return 0;
}

/* int */

EZC_TF_INIT(int) {
    obj->_int = 0;
    return 0;
}

EZC_TF_FREE(int) {
    // do nothing
    return 0;
}

EZC_TF_REPR(int) {
    ezc_int val = obj->_int;
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
        strs[i++] = digitstr[val % base];
        val /= base;
    } while (val > 0);

    // now, reverse it
    int len = i;
    for (i = is_neg ? 1 : 0; 2 * i < len; ++i) {
        char tmp = strs[i];
        strs[i] = strs[len-i-1];
        strs[len-i-1] = tmp;
    }

    ezc_str_copy_cp(str, strs, len);

    return 0;
}

EZC_TF_COPY(int) {
    obj->_int = from->_int;
    return 0;
}

/* bool */

EZC_TF_INIT(bool) {
    obj->_bool = false;
    return 0;
}

EZC_TF_FREE(bool) {
    // do nothing
    return 0;
}

EZC_TF_REPR(bool) {
    if (obj->_bool) {
        ezc_str_copy_cp(str, "true", 4);
    } else {
        ezc_str_copy_cp(str, "false", 5);
    }
    return 0;
}

EZC_TF_COPY(bool) {
    obj->_bool = from->_bool;
    return 0;
}
/* real */

EZC_TF_INIT(real) {
    obj->_real = 0;
    return 0;
}

EZC_TF_FREE(real) {
    // do nothing
    return 0;
}

EZC_TF_REPR(real) {
    char strs[100];

    // using C functions:
    sprintf(strs, "%lf", obj->_real);
    ezc_str_copy_cp(str, strs, strlen(strs));

    // custom impl
    /*
    // what base to use
    int base = 10;

    // integer (whole number part) of the number
    ezc_int ival = (ezc_int)obj->_real;

    // fractional value of the number
    ezc_real fval = obj->_real;
    fval -= ival;

    int i = 0;
    bool is_neg = false;

    if (ival < 0) {
        ival = -ival;
        strs[i++] = '-';
        is_neg = true;
    }

    do {
        strs[i++] = digitstr[ival % base];
        ival /= base;
    } while (ival > 0);

    // now, reverse it
    int len = i;
    for (i = is_neg ? 1 : 0; 2 * i < len; ++i) {
        char tmp = strs[i];
        strs[i] = strs[len-i-1];
        strs[len-i-1] = tmp;
    }
    i = len;
    strs[i++] = '.';

    fval *= base;

    do {
        int cdig = (ezc_int)fval;
        strs[i++] = digitstr[cdig];
        fval -= cdig;
        fval *= base;
        len++;
    } while (fval > 0.0 && i < 40);

    ezc_str_copy_cp(str, strs, i);
    */
    return 0;
}

EZC_TF_COPY(real) {
    obj->_real = from->_real;
    return 0;
}

/* str */

EZC_TF_INIT(str) {
    obj->_str = EZC_STR_NULL;
    return 0;
}

EZC_TF_FREE(str) {
    ezc_str_free(&obj->_str);
    return 0;
}

EZC_TF_REPR(str) {
    ezc_str_copy(str, obj->_str);
    return 0;
}

EZC_TF_COPY(str) {
    obj->_str = EZC_STR_NULL;
    ezc_str_copy(&obj->_str, from->_str);
    return 0;
}

/* block type */

EZC_TF_INIT(block) {
    obj->_block = EZCI_EMPTY;
    return 0;
}

EZC_TF_FREE(block) {
    // do nothing
    return 0;
}

EZC_TF_REPR(block) {
    char strs[100];
    sprintf(strs, "{...[%d]}", obj->_block.type);
    ezc_str_copy_cp(str, strs, strlen(strs));
    return 0;
}

EZC_TF_COPY(block) {
    obj->_block = from->_block;
    return 0;
}

/* functions in this module */

EZC_FUNC(none) {
    ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_NONE });
    return 0;
}

EZC_FUNC(wall) {
    ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_WALL });
    return 0;
}

EZC_FUNC(del) {
    REQ_N(del, 1);
    ezc_obj popped = ezc_stk_pop(&vm->stk);
    OBJ_FREE(popped);
    return 0;
}

EZC_FUNC(funcdef) {
    REQ_N(funcdef, 2);
    ezc_obj f_name = ezc_stk_pop(&vm->stk);
    ezc_obj f_body = ezc_stk_pop(&vm->stk);

    if (f_name.type == EZC_TYPE_STR) {
        if (f_body.type == EZC_TYPE_BLOCK) {
            ezc_vm_addfunc(vm, f_name._str, EZC_FUNC_EZC(f_body._block));
            OBJ_FREE(f_name);
            return 0;
        } else {
            ezc_stk_push(&vm->stk, f_body);
            ezc_stk_push(&vm->stk, f_name);
            ezc_error("`body` is not type `block` in:\n[body] [name] funcdef!");
            return -1;
        }
    } else {
        ezc_stk_push(&vm->stk, f_body);
        ezc_stk_push(&vm->stk, f_name);
        ezc_error("`name` is not type `str` in:\n[body] [name] funcdef!");
        return -1;
    }
}

EZC_FUNC(dup) {
    REQ_N(dup, 1);
    ezc_obj top = ezc_stk_peek(&vm->stk);

    ezc_obj new_obj = (ezc_obj){ .type = top.type };
    OBJ_COPY(new_obj, top);
    ezc_stk_push(&vm->stk, new_obj);
    return 0;
}

EZC_FUNC(under) {
    REQ_N(under, 2);
    ezc_obj under = ezc_stk_peekn(&vm->stk, 1);

    ezc_obj new_obj = EZC_OBJ_EMPTY;
    OBJ_COPY(new_obj, under);
    ezc_stk_push(&vm->stk, new_obj);
    return 0;
}

EZC_FUNC(get) {
    REQ_N(get, 1);
    ezc_obj idx = ezc_stk_pop(&vm->stk);

    if (idx.type == EZC_TYPE_INT) {
        if (idx._int >= 0) {
            ezc_obj peeked = ezc_stk_get(&vm->stk, idx._int);
            ezc_obj new_obj = EZC_OBJ_EMPTY;
            OBJ_COPY(new_obj, peeked);
            ezc_stk_push(&vm->stk, new_obj);
        } else if (idx._int < 0) {
            ezc_obj peeked = ezc_stk_get(&vm->stk, vm->stk.n + idx._int);
            ezc_obj new_obj = EZC_OBJ_EMPTY;
            OBJ_COPY(new_obj, peeked);
            ezc_stk_push(&vm->stk, new_obj);
        } else {
            ezc_stk_push(&vm->stk, idx);
            ezc_error("`idx` must be >=0 in:\n[idx] get! (or $)");
            return -1;
        }
    } else {
        ezc_stk_push(&vm->stk, idx);
        ezc_error("`idx` is not type `int` in:\n[idx] get! (or $)");
        return -1;
    }

    //ezc_obj new_obj = EZC_OBJ_EMPTY;
    //OBJ_COPY(new_obj, under);
    //ezc_stk_push(&vm->stk, new_obj);
    OBJ_FREE(idx);
    return 0;
}

EZC_FUNC(swap) {
    REQ_N(swap, 2);
    ezc_stk_swap(&vm->stk, vm->stk.n-1, vm->stk.n-2);
    return 0;
}

EZC_FUNC(exec) {
    REQ_N(exec, 1);
    ezc_obj popped = ezc_stk_pop(&vm->stk);
    int stat = 0;

    if (popped.type == EZC_TYPE_STR) {
        // execute the function byt this name
        int idx = ezc_vm_getfunci(vm, popped._str);
        if (idx < 0) { 
            ezc_stk_push(&vm->stk, popped);
            ezc_error("Unknown function: '%s'", popped._str._);
            return -1;
        } else {

            ezc_func to_exec = vm->funcs.vals[idx];
            //vm->funcs[idx](vm);
            if (to_exec.type == EZC_FUNC_TYPE_C) {
                OBJ_FREE(popped);
                return to_exec._c(vm);
            } else if (to_exec.type == EZC_FUNC_TYPE_EZC) {
                ezcp _prog = EZCP_EMPTY;
                _prog.body = to_exec._ezc;
                _prog.src = to_exec._ezc.m_prog->src;
                _prog.src_name = to_exec._ezc.m_prog->src_name;
                stat = ezc_vm_exec(vm, _prog);
                OBJ_FREE(popped);
                return stat;
            }
        }
    } else if (popped.type == EZC_TYPE_BLOCK) {
        ezcp _prog = EZCP_EMPTY;
        _prog.body = popped._block;
        _prog.src =  popped._block.m_prog->src;
        _prog.src_name =  popped._block.m_prog->src_name;
        stat = ezc_vm_exec(vm, _prog);
        OBJ_FREE(popped);
        return stat;
    } else {
        // TODO: account for blocks and things, but for now, just ezc_error
        ezc_stk_push(&vm->stk, popped);
        ezc_error("Invalid type for `!` / `exec`: '%s'", TYPE_NAME(popped)._);
        return -1;
    }
}

EZC_FUNC(repr) {
    REQ_N(repr, 1);
    ezc_obj popped = ezc_stk_pop(&vm->stk);

    // get the repr
    ezc_obj new_str = (ezc_obj){ .type = EZC_TYPE_STR, ._str = EZC_STR_NULL };
    OBJ_REPR(popped, new_str._str);

    // push it onto the stack
    ezc_stk_push(&vm->stk, new_str);

    OBJ_FREE(popped);
    return 0;
}

EZC_FUNC(print) {
    REQ_N(print, 1);

    // just go ahead and replace the last object with its representation
    EZC_FUNC_NAME(repr)(vm);

    ezc_obj popped = ezc_stk_pop(&vm->stk);

    if (popped.type == EZC_TYPE_STR) {
        printf("%s\n", popped._str._);
        OBJ_FREE(popped);
        return 0;
    } else {
        // repr should always give a string on top of the stack, so this shouldn't happen
        ezc_stk_push(&vm->stk, popped);
        ezc_error("Internal; the item on the top of the stack was not a string! (this means repr! didn't work)");
        return -1;
    }
}

EZC_FUNC(printall) {
    // just go ahead and replace the last object with its representation
    //EZC_FUNC_NAME(repr)(vm);
    ezc_str repr_str = EZC_STR_NULL;
    int i;
    for (i = 0; i < vm->stk.n; ++i) {
        OBJ_REPR(vm->stk.base[i], repr_str);
        printf("%s ", repr_str._);
    }
    printf("\n");

    ezc_str_free(&repr_str);
    return 0;
}



EZC_FUNC(dump) {
    ezc_str str = EZC_STR_NULL;
    int i;
    for (i = vm->stk.n - 1; i >= 0; --i) {
        ezc_obj cur = ezc_stk_get(&vm->stk, i);
        OBJ_REPR(cur, str);
        printf("%*d: %s\n", 2, i, str._);
    }
    ezc_str_free(&str);

    printf("-----\nstack\n");
    return 0;
}

/* math functions */

#define TT_CASE(_Ta, _Tb) (A.type == _Ta && B.type == _Tb)
#define TT_CASES(_Ta) TT_CASE(_Ta, _Ta)
#define TT_RESTORE() { ezc_stk_push(&vm->stk, B); ezc_stk_push(&vm->stk, A); }
#define TT_FREE() { OBJ_FREE(A); OBJ_FREE(B); }
#define TT_TYPE_ER(_fname) { ezc_error("Invalid type combo for func `" #_fname "`: %s, %s", TYPE_NAME(A)._, TYPE_NAME(B)._); TT_RESTORE(); }

EZC_FUNC(add) {
    REQ_N(add, 2);
    // compute A+B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);
    //ezc_stk_push(&vm->stk, B);
    //return 0;
    if (A.type == B.type) {
        if (TT_CASES(EZC_TYPE_STR)) {
            // optimized method, without complete replication
            ezc_str_append(&A._str, B._str);
            ezc_stk_push(&vm->stk, A);
            OBJ_FREE(B);
            return 0;
            // naive method:
            //ezc_str new_str = EZC_STR_NULL;
            //ezc_str_concat(&new_str, A._str, B._str);
            //ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_STR, ._str = new_str });
        } else if (TT_CASES(EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = A._int + B._int });
        } else if (TT_CASES(EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real + B._real });
        } else {
            TT_TYPE_ER(add);
            return -1;
        }
    } else {
        if (TT_CASE(EZC_TYPE_INT, EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._int + B._real });
        } else if (TT_CASE(EZC_TYPE_REAL, EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real + B._int });
        } else {
            TT_TYPE_ER(add);
            return -1;
        }
    }
    TT_FREE();
    return 0;
}

EZC_FUNC(sub) {
    REQ_N(sub, 2);
    // compute A-B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (TT_CASES(EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = A._int - B._int });
        } else if (TT_CASES(EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real - B._real });
        } else {
            TT_TYPE_ER(sub);
            return -1;
        }
    } else {
        if (TT_CASE(EZC_TYPE_INT, EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._int - B._real });
        } else if (TT_CASE(EZC_TYPE_REAL, EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real - B._int });
        } else {
            TT_TYPE_ER(sub);
            return -1;
        }
    }
    TT_FREE();
    return 0;
}

EZC_FUNC(mul) {
    REQ_N(mul, 2);
    // compute A*B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (TT_CASES(EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = A._int * B._int });
        } else if (TT_CASES(EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real * B._real });
        } else {
            TT_TYPE_ER(mul);
            return -1;
        }
    } else {
        if (TT_CASE(EZC_TYPE_INT, EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._int * B._real });
        } else if (TT_CASE(EZC_TYPE_REAL, EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real * B._int });
        } else {
            TT_TYPE_ER(mul);
            return -1;
        }
    }
    TT_FREE();
    return 0;
}

EZC_FUNC(div) {
    REQ_N(div, 2);
    // compute A/B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (TT_CASES(EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = A._int / B._int });
        } else if (TT_CASES(EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real / B._real });
        } else {
            TT_TYPE_ER(div);
            return -1;
        }
    } else {
        if (TT_CASE(EZC_TYPE_INT, EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._int / B._real });
        } else if (TT_CASE(EZC_TYPE_REAL, EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = A._real / B._int });
        } else {
            TT_TYPE_ER(div);
            return -1;
        }
    }
    TT_FREE();
}

EZC_FUNC(mod) {
    REQ_N(mod, 2);
    // compute A%B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (TT_CASES(EZC_TYPE_INT)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = A._int % B._int });
        } else if (TT_CASES(EZC_TYPE_REAL)) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = fmod(A._real, B._real) });
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
    REQ_N(pow, 2);
    // compute A^B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            // r = a^b
            ezc_int a = A._int;
            ezc_int b = B._int;
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
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_INT, ._int = r });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_REAL, ._real = pow(A._real, B._real) });
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
    REQ_N(eq, 2);
    // compute A==B
    ezc_obj B = ezc_stk_pop(&vm->stk);
    ezc_obj A = ezc_stk_pop(&vm->stk);

    if (A.type == B.type) {
        if (A.type == EZC_TYPE_INT) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_BOOL, ._bool = A._int == B._int });
        } else if (A.type == EZC_TYPE_REAL) {
            ezc_stk_push(&vm->stk, (ezc_obj){ .type = EZC_TYPE_BOOL, ._real = A._real == B._real });
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
    REQ_N(ifel, 3);
    // | condition {code if true} {code if not} ifel!

    ezc_obj b_else = ezc_stk_pop(&vm->stk);
    ezc_obj b_if = ezc_stk_pop(&vm->stk);
    ezc_obj cond = ezc_stk_pop(&vm->stk);

    bool cond_val = false;
    OBJ_TRUTHY(cond, cond_val);

    if (cond_val) {
        ezc_stk_push(&vm->stk, b_if);
        EZC_FUNC_NAME(exec)(vm);
    } else {
        ezc_stk_push(&vm->stk, b_else);
        EZC_FUNC_NAME(exec)(vm);
    }

    OBJ_FREE(b_else);
    OBJ_FREE(b_if);
    OBJ_FREE(cond);
    return 0;
}

EZC_FUNC(foreach) {
    REQ_N(foreach, 1);
    // | a b c ... d {body} foreach!
    // runs body for each a, b, c
    ezc_obj body = ezc_stk_pop(&vm->stk);

    if (body.type != EZC_TYPE_BLOCK) {
        ezc_stk_push(&vm->stk, body);
        ezc_error("Expected the block for `foreach` to be of type `block` (like {...}), but got `%s`", TYPE_NAME(body)._);
        return 1;
    }

    // how many objects to iterate?
    int num_to_iter = 0;

    // now, keep going until stack hits wall or is empty
    while (num_to_iter < vm->stk.n && ezc_stk_peekn(&vm->stk, num_to_iter).type != EZC_TYPE_WALL) {
        num_to_iter++;
    }

    // quit out early
    if (num_to_iter < 1) {
        OBJ_FREE(body);
        return 0;
    }

    // offset of where to start iterating
    int stk_offset = vm->stk.n - num_to_iter;

    // start another temporary stack
    ezc_stk argstack = EZC_STK_EMPTY;
    ezc_stk_resize(&argstack, num_to_iter);


    // copy over the results
    int i;
    for (i = 0; i < num_to_iter; ++i) {
        argstack.base[i] = vm->stk.base[i + stk_offset];
    }

    // remove the popped off objects
    vm->stk.n -= num_to_iter;

    // consume the wall if used
    if (ezc_stk_peek(&vm->stk).type == EZC_TYPE_WALL) {
        ezc_stk_pop(&vm->stk);
    }


    // create the program to run on each iteration
    ezcp _prog = EZCP_EMPTY;
    _prog.body = body._block;
    _prog.src = body._block.m_prog->src;
    _prog.src_name = body._block.m_prog->src_name;

    // find the status of the evaluation
    int status = 0;

    for (i = 0; i < num_to_iter; ++i) {
        ezc_stk_push(&vm->stk, argstack.base[i]);
        status = ezc_vm_exec(vm, _prog);
        if (status != 0) return status;
    }

    // free the temporary stack
    ezc_stk_free(&argstack);

    return 0;
}

// generators

// X == 'expand'
EZC_FUNC(X) {
    REQ_N(X, 1);
    ezc_obj arg = ezc_stk_pop(&vm->stk);

    if (arg.type == EZC_TYPE_INT) {
        // do 0...arg-1
        ezc_int i;
        ezc_obj new_int = EZC_OBJ_EMPTY;
        new_int.type = EZC_TYPE_INT;
        for (i = 0; i < arg._int; ++i) {
            new_int._int = i;
            ezc_stk_push(&vm->stk, new_int);
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
int EZC_FUNC_NAME(register_module)(ezc_vm* vm) {

    /* register the built in types. MUST BE IN THIS ORDER */

    EZC_REGISTER_TYPE(none)
    EZC_REGISTER_TYPE(wall)
    EZC_REGISTER_TYPE(int)
    EZC_REGISTER_TYPE(bool)
    EZC_REGISTER_TYPE(real)
    EZC_REGISTER_TYPE(str)
    EZC_REGISTER_TYPE(block)

    // functions

    EZC_REGISTER_FUNC(none)
    EZC_REGISTER_FUNC(wall)
    EZC_REGISTER_FUNC(del)
    EZC_REGISTER_FUNC(funcdef)
    EZC_REGISTER_FUNC(dup)
    EZC_REGISTER_FUNC(swap)
    EZC_REGISTER_FUNC(exec)
    EZC_REGISTER_FUNC(repr)
    EZC_REGISTER_FUNC(print)
    EZC_REGISTER_FUNC(printall)
    EZC_REGISTER_FUNC(dump)

    EZC_REGISTER_FUNC(add)
    EZC_REGISTER_FUNC(sub)
    EZC_REGISTER_FUNC(mul)
    EZC_REGISTER_FUNC(div)
    EZC_REGISTER_FUNC(mod)
    EZC_REGISTER_FUNC(pow)

    EZC_REGISTER_FUNC(under)
    EZC_REGISTER_FUNC(get)
    EZC_REGISTER_FUNC(eq)

    EZC_REGISTER_FUNC(ifel)
    EZC_REGISTER_FUNC(foreach)

    EZC_REGISTER_FUNC(X)

}


