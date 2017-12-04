
#define MODULE_NAME "ezc.operators"

#define MODULE_DESCRIPTION "defines standard operators for ezc"


#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



#define OPERATOR_MISMATCH(op, t0, t1) sprintf(to_raise, "type mismatch for operator '%s': types %s and %s not a valid combination", op, t0.name, t1.name); raise_exception(to_raise, 1);



// quick method from: https://stackoverflow.com/questions/34035169/fastest-way-to-reverse-a-string-in-c/34035474
// make sure that this is a new string, as it alters str in place!
void __reverse_str_inplace(char *str){
    int len = strlen(str);
    char *p1 = str;
    char *p2 = str + len - 1;

    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
}


obj_t __op_add(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) + OBJ_AS_STRUCT(b, int);
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) + OBJ_AS_STRUCT(b, float);
        } else if (ISTYPE(a, str_type)) {
            // strlen(concat(a, b)) = strlen(a) + strlen(b) - 1
            OBJ_ALLOC_BYTES(r, a.data_len + b.data_len - 1);
            r.type_id = str_type.id;
            
            strcpy(OBJ_AS_POINTER(r, char), OBJ_AS_POINTER(a, char));
            strcat(OBJ_AS_POINTER(r, char), OBJ_AS_POINTER(b, char));
        }  else {
            UNKNOWN_TYPE(at.name);
            return NULL_OBJ;    
        }
        return r;
    } else {
        bool is_float = false;
        float rv = 0;
        if (ISTYPE(a, int_type) && ISTYPE(b, float_type)) {
            rv = OBJ_AS_STRUCT(a, int) + OBJ_AS_STRUCT(b, float);
            is_float = true;
        } else if (ISTYPE(a, float_type) && ISTYPE(b, int_type)) {
            rv = OBJ_AS_STRUCT(a, float) + OBJ_AS_STRUCT(b, int);
            is_float = true;
        }

        if (is_float) {
            OBJ_ALLOC_STRUCT(r, float);
            OBJ_AS_STRUCT(r, float) = rv;
            r.type_id = float_type.id;
            return r;
        } else {
            OPERATOR_MISMATCH("+", at, bt);
        }
    }

    return NULL_OBJ;
}


obj_t __op_sub(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) - OBJ_AS_STRUCT(b, int);
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) - OBJ_AS_STRUCT(b, float);
        }  else {
            UNKNOWN_TYPE(at.name);
            return NULL_OBJ;    
        }
        return r;
    } else {
        bool is_float = false;
        float rv = 0;
        if (ISTYPE(a, int_type) && ISTYPE(b, float_type)) {
            rv = OBJ_AS_STRUCT(a, int) - OBJ_AS_STRUCT(b, float);
            is_float = true;
        } else if (ISTYPE(a, float_type) && ISTYPE(b, int_type)) {
            rv = OBJ_AS_STRUCT(a, float) - OBJ_AS_STRUCT(b, int);
            is_float = true;
        }

        if (is_float) {
            OBJ_ALLOC_STRUCT(r, float);
            OBJ_AS_STRUCT(r, float) = rv;
            r.type_id = float_type.id;
            return r;
        } else {
            OPERATOR_MISMATCH("-", at, bt);
        }
    }

    return NULL_OBJ;
}


obj_t __op_mul(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) * OBJ_AS_STRUCT(b, int);
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) * OBJ_AS_STRUCT(b, float);
        }  else {
            UNKNOWN_TYPE(at.name);
            return NULL_OBJ;    
        }
        return r;
    } else {
        bool is_float = false;
        bool is_str = false;

        char * strv = NULL;
        int ncpy = 0;

        float rv = 0;
        if (ISTYPE(a, int_type) && ISTYPE(b, float_type)) {
            rv = OBJ_AS_STRUCT(a, int) * OBJ_AS_STRUCT(b, float);
            is_float = true;
        } else if (ISTYPE(a, float_type) && ISTYPE(b, int_type)) {
            rv = OBJ_AS_STRUCT(a, float) * OBJ_AS_STRUCT(b, int);
            is_float = true;
        } else if (ISTYPE(a, str_type) && ISTYPE(b, int_type)) {
            is_str = true;
            strv = OBJ_AS_POINTER(a, char);
            ncpy = OBJ_AS_STRUCT(b, int);
        } else if (ISTYPE(b, str_type) && ISTYPE(a, int_type)) {
            is_str = true;
            strv = OBJ_AS_POINTER(b, char);
            ncpy = OBJ_AS_STRUCT(a, int);
        }

        if (is_float) {
            OBJ_ALLOC_STRUCT(r, float);
            OBJ_AS_STRUCT(r, float) = rv;
            r.type_id = float_type.id;
            return r;
        } else if (is_str) {
            if (ncpy < 0) {
                int ancpy = abs(ncpy);
                OBJ_ALLOC_BYTES(r, ancpy * strlen(strv) + 1);
                r.type_id = str_type.id;
                char * strvrv = malloc(strlen(strv) + 1);
                strcpy(strvrv, strv);
                __reverse_str_inplace(strvrv);
                int i;
                for (i = 0; i < ancpy; ++i) strcpy(strlen(strvrv) * i + (char *)r.data, strvrv);
                free(strvrv);
            } else if (ncpy == 1) {
                OBJ_ALLOC_BYTES(r, strlen(strv) + 1);
                r.type_id = str_type.id;
                strcpy((char *)r.data, strv);
            } else {
                OBJ_ALLOC_BYTES(r, ncpy * strlen(strv) + 1);
                r.type_id = str_type.id;
                int i;
                for (i = 0; i < ncpy; ++i) strcpy(strlen(strv) * i + (char *)r.data, strv);
            }
            return r;
        } else {
            OPERATOR_MISMATCH("*", at, bt);
        }
    }

    return NULL_OBJ;
}


obj_t __op_div(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            int bval = OBJ_AS_STRUCT(b, int);
            if (bval == 0) {
                raise_exception("division by '0'", 1);
            }
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) / bval;
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            // TODO: should we return an exception on float division by zero?
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) / OBJ_AS_STRUCT(b, float);
        }  else {
            UNKNOWN_TYPE(at.name);
            return NULL_OBJ;    
        }
        return r;
    } else {
        bool is_float = false;
        float rv = 0;
        if (ISTYPE(a, int_type) && ISTYPE(b, float_type)) {
            rv = OBJ_AS_STRUCT(a, int) / OBJ_AS_STRUCT(b, float);
            is_float = true;
        } else if (ISTYPE(a, float_type) && ISTYPE(b, int_type)) {
            rv = OBJ_AS_STRUCT(a, float) / OBJ_AS_STRUCT(b, int);
            is_float = true;
        }

        if (is_float) {
            OBJ_ALLOC_STRUCT(r, float);
            OBJ_AS_STRUCT(r, float) = rv;
            r.type_id = float_type.id;
            return r;
        } else {
            OPERATOR_MISMATCH("/", at, bt);
        }
    }

    return NULL_OBJ;
}


int __int_pow(int b, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r *= b;
        e >>= 1;
        b *= b;
    }
    return r;
}

float __float_int_pow(float b, int e) {
    float r = 1;
    while (e) {
        if (e & 1) r *= b;
        e >>= 1;
        b *= b;
    }
    return r;
}



obj_t __op_pow(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = __int_pow(OBJ_AS_STRUCT(a, int), OBJ_AS_STRUCT(b, int));
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = pow(OBJ_AS_STRUCT(a, float), OBJ_AS_STRUCT(b, float));
        }  else {
            UNKNOWN_TYPE(at.name);
            return NULL_OBJ;    
        }
        return r;
    } else {
        bool is_float = false;
        float rv = 0;
        if (ISTYPE(a, int_type) && ISTYPE(b, float_type)) {
            rv = pow(OBJ_AS_STRUCT(a, int), OBJ_AS_STRUCT(b, float));
            is_float = true;
            
        } else if (ISTYPE(a, float_type) && ISTYPE(b, int_type)) {
            rv = __float_int_pow(OBJ_AS_STRUCT(a, float), OBJ_AS_STRUCT(b, int));
            is_float = true;
        }

        if (is_float) {
            OBJ_ALLOC_STRUCT(r, float);
            OBJ_AS_STRUCT(r, float) = rv;
            r.type_id = float_type.id;
            return r;
        } else {
            OPERATOR_MISMATCH("^", at, bt);
        }
    }

    return NULL_OBJ;
}



obj_t __op_neg(obj_t a) {
    obj_t r;

    type_t at = OBJ_TYPE(a);

    type_t int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    if (ISTYPE(a, int_type)) {
        OBJ_ALLOC_STRUCT(r, int);
        OBJ_AS_STRUCT(r, int) = - OBJ_AS_STRUCT(a, int);
        r.type_id = int_type.id;
    } else if (ISTYPE(a, float_type)) {
        // TODO: should we return an exception on float division by zero?
        OBJ_ALLOC_STRUCT(r, float);
        r.type_id = float_type.id;
        OBJ_AS_STRUCT(r, float) = - OBJ_AS_STRUCT(a, float);
    } else if (ISTYPE(a, str_type)) {
        OBJ_ALLOC_BYTES(r, a.data_len);
        r.type_id = str_type.id;
        strcpy(OBJ_AS_POINTER(r, char), OBJ_AS_POINTER(a, char));
        __reverse_str_inplace(OBJ_AS_POINTER(r, char));
    }  else {
        UNKNOWN_TYPE(at.name);
        return NULL_OBJ;    
    }
    return r;

    return NULL_OBJ;
}


obj_t __op_floor(obj_t a) {
    obj_t r;

    type_t at = OBJ_TYPE(a);

    type_t int_type = TYPE("int"), float_type = TYPE("float");

    if (ISTYPE(a, int_type)) {
        OBJ_ALLOC_STRUCT(r, int);
        OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int);
        r.type_id = int_type.id;
    } else if (ISTYPE(a, float_type)) {
        // TODO: should we return an exception on float division by zero?
        OBJ_ALLOC_STRUCT(r, float);
        r.type_id = float_type.id;
        OBJ_AS_STRUCT(r, float) = floor(OBJ_AS_STRUCT(a, float));
    }  else {
        UNKNOWN_TYPE(at.name);
        return NULL_OBJ;    
    }
    return r;

    return NULL_OBJ;
}



#define STACK_OP(opfunc) { \
    ASSURE_STACK_LEN(2); \
    obj_t b = estack_pop(&runtime->stack); \
    obj_t a = estack_pop(&runtime->stack); \
    obj_t r = opfunc(a, b); \
    estack_push(&runtime->stack, r); \
    obj_free(&a); \
    obj_free(&b); \
}


// standard operators
void op_add(runtime_t * runtime) STACK_OP(__op_add)
void op_sub(runtime_t * runtime) STACK_OP(__op_sub)
void op_mul(runtime_t * runtime) STACK_OP(__op_mul)
void op_div(runtime_t * runtime) STACK_OP(__op_div)

void op_pow(runtime_t * runtime) STACK_OP(__op_pow)



#define UNARY_OP(opfunc) { \
    ASSURE_STACK_LEN(1); \
    obj_t a = estack_pop(&runtime->stack); \
    obj_t r = opfunc(a); \
    estack_push(&runtime->stack, r); \
    obj_free(&a); \
}


// unary operator
void op_neg(runtime_t * runtime) UNARY_OP(__op_neg)
void op_floor(runtime_t * runtime) UNARY_OP(__op_floor)


int init (int id, module_utils_t utils) {
    init_exported(id, utils);

    add_function("add", "adds the last two items on the stack", op_add);
    add_function("sub", "subtracts the last two items on the stack ('a b sub!' goes to '(a-b)')", op_sub);
    add_function("mul", "multiplies the last two items on the stack", op_mul);
    add_function("div", "divides the last two items on the stack", op_div);

    add_function("pow", "for an (a, b) on the stack, it pops back on a^b", op_pow);

    add_function("neg", "if given a number, replaces the top of the stack with the negative value (1-x), or if it is a string, pop on the string reversed", op_neg);
    add_function("floor", "floors the last number", op_floor);


    return 0;
}



