
#define MODULE_NAME "ezc.operators"

#define MODULE_DESCRIPTION "defines standard operators for ezc"


#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#ifdef HAVE_GMP
#include <gmp.h>
#endif

#ifdef HAVE_MPFR
#include <mpfr.h>
#endif


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

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}


obj_t __op_add(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    
    type_t int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");

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
        } 
#ifdef HAVE_GMP
        else if (ISTYPE(a, mpz_type)) {
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            mpz_add(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t), OBJ_AS_STRUCT(b, mpz_t));
        } else if (ISTYPE(a, mpf_type)) {
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpf_t * rf = OBJ_AS_POINTER(r, mpf_t), * af = OBJ_AS_POINTER(a, mpf_t), * bf = OBJ_AS_POINTER(b, mpf_t);
            mpf_init2(*rf, max(mpf_get_prec(*af), mpf_get_prec(*bf)));
            mpf_add(*rf, *af, *bf);
        }
#endif
#ifdef HAVE_MPFR
        else if (ISTYPE(a, mpfr_type)) {
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_t * rf = OBJ_AS_POINTER(r, mpfr_t), * af = OBJ_AS_POINTER(a, mpfr_t), * bf = OBJ_AS_POINTER(b, mpfr_t);

            mpfr_init2(*rf, max(mpfr_get_prec(*af), mpfr_get_prec(*bf)));
            mpfr_add(*rf, *af, *bf, EZC_RND);
        }
#endif
        else {
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
        }
#ifdef HAVE_MPFR
        // if there is an mpf, that takes precedence
        if (ISTYPE(a, mpfr_type) || ISTYPE(b, mpfr_type)) {
            obj_t * mpfrobj, *oobj;
            if (ISTYPE(a, mpfr_type)) {
                mpfrobj = &a;
                oobj = &b;
            } else {
                oobj = &a;
                mpfrobj = &b;
            }
            
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_init2(OBJ_AS_STRUCT(r, mpfr_t), mpfr_get_prec(OBJ_AS_STRUCT(*mpfrobj, mpfr_t)));
            
            if (ISTYPE(*oobj, int_type)) {
                mpfr_add_si(OBJ_AS_STRUCT(r, mpfr_t), OBJ_AS_STRUCT(*mpfrobj, mpfr_t), OBJ_AS_STRUCT(*oobj, int), EZC_RND);
                return r;
            } else if (ISTYPE(*oobj, float_type)) {
                mpfr_add_d(OBJ_AS_STRUCT(r, mpfr_t), OBJ_AS_STRUCT(*mpfrobj, mpfr_t), OBJ_AS_STRUCT(*oobj, float), EZC_RND);
                return r;
            } else if (ISTYPE(*oobj, mpf_type)) {
                mpfr_t oobj_as_mpfr;
                mpfr_init2(oobj_as_mpfr, mpf_get_prec(OBJ_AS_STRUCT(*oobj, mpf_t)));
                mpfr_set_f(oobj_as_mpfr, OBJ_AS_STRUCT(*oobj, mpf_t), EZC_RND);
                mpfr_add(OBJ_AS_STRUCT(r, mpfr_t), OBJ_AS_STRUCT(*mpfrobj, mpfr_t), oobj_as_mpfr, EZC_RND);
                return r;
            } else if (ISTYPE(*oobj, mpz_type)) {
                mpfr_add_z(OBJ_AS_STRUCT(r, mpfr_t), OBJ_AS_STRUCT(*mpfrobj, mpfr_t), OBJ_AS_STRUCT(*oobj, mpz_t), EZC_RND);
                return r;
            }
        }
#endif
#ifdef HAVE_GMP
        // if there is an mpf, that takes precedence
        if (ISTYPE(a, mpf_type) || ISTYPE(b, mpf_type)) {
            obj_t * mpfobj, *oobj;
            if (ISTYPE(a, mpf_type)) {
                mpfobj = &a;
                oobj = &b;
            } else {
                oobj = &a;
                mpfobj = &b;
            }
            
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpf_init2(OBJ_AS_STRUCT(r, mpf_t), mpf_get_prec(OBJ_AS_STRUCT(*mpfobj, mpf_t)));
            
            if (ISTYPE(*oobj, int_type)) {
                int v = OBJ_AS_STRUCT(*oobj, int);
                if (v >= 0) {
                    mpf_add_ui(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*mpfobj, mpf_t), v);
                } else {
                    mpf_sub_ui(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*mpfobj, mpf_t), abs(v));
                }
                return r;
            
            } else if (ISTYPE(*oobj, float_type)) {
                mpf_set_d(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*oobj, float));
                mpf_add(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*mpfobj, mpf_t), OBJ_AS_STRUCT(r, mpf_t));
                return r;
            } else if (ISTYPE(*oobj, mpz_type)) {
                mpf_set_z(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*oobj, mpz_t));
                mpf_add(OBJ_AS_STRUCT(r, mpf_t), OBJ_AS_STRUCT(*mpfobj, mpf_t), OBJ_AS_STRUCT(r, mpf_t));
                return r;
            }
        } else if (ISTYPE(a, mpz_type) || ISTYPE(b, mpz_type)) {
            obj_t * mpzobj, *oobj;
            if (ISTYPE(a, mpz_type)) {
                mpzobj = &a;
                oobj = &b;
            } else {
                oobj = &a;
                mpzobj = &b;
            }
            
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            
            if (ISTYPE(*oobj, int_type)) {
                int v = OBJ_AS_STRUCT(*oobj, int);
                if (v >= 0) {
                    mpz_add_ui(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(*mpzobj, mpz_t), v);
                } else {
                    mpz_sub_ui(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(*mpzobj, mpz_t), abs(v));
                }
                return r;
            } else if (ISTYPE(*oobj, float_type)) {
                int v = (int)floor(OBJ_AS_STRUCT(*oobj, float));
                if (v >= 0) {
                    mpz_add_ui(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(*mpzobj, mpz_t), v);
                } else {
                    mpz_sub_ui(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(*mpzobj, mpz_t), abs(v));
                }
                return r;
            }
        }

#endif
        OPERATOR_MISMATCH("+", at, bt);
    }

    return NULL_OBJ;
}

obj_t __op_sub(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    type_t int_type = TYPE("int"), float_type = TYPE("float");

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");


    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) - OBJ_AS_STRUCT(b, int);
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) - OBJ_AS_STRUCT(b, float);
            
        }
#ifdef HAVE_GMP
        else if (ISTYPE(a, mpz_type)) {
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            mpz_sub(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t), OBJ_AS_STRUCT(b, mpz_t));
        } else if (ISTYPE(a, mpf_type)) {
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpf_t * rf = OBJ_AS_POINTER(r, mpf_t), * af = OBJ_AS_POINTER(a, mpf_t), * bf = OBJ_AS_POINTER(b, mpf_t);
            mpf_init2(*rf, max(mpf_get_prec(*af), mpf_get_prec(*bf)));
            mpf_sub(*rf, *af, *bf);
        }
#endif
#ifdef HAVE_MPFR
        else if (ISTYPE(a, mpfr_type)) {
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_t * rf = OBJ_AS_POINTER(r, mpfr_t), * af = OBJ_AS_POINTER(a, mpfr_t), * bf = OBJ_AS_POINTER(b, mpfr_t);

            mpfr_init2(*rf, max(mpfr_get_prec(*af), mpfr_get_prec(*bf)));
            mpfr_sub(*rf, *af, *bf, EZC_RND);
        }
#endif
        else {
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

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");


    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = OBJ_AS_STRUCT(a, int) * OBJ_AS_STRUCT(b, int);
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = OBJ_AS_STRUCT(a, float) * OBJ_AS_STRUCT(b, float);
        }
#ifdef HAVE_GMP
        else if (ISTYPE(a, mpz_type)) {
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            mpz_mul(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t), OBJ_AS_STRUCT(b, mpz_t));
        } else if (ISTYPE(a, mpf_type)) {
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpf_t * rf = OBJ_AS_POINTER(r, mpf_t), * af = OBJ_AS_POINTER(a, mpf_t), * bf = OBJ_AS_POINTER(b, mpf_t);
            mpf_init2(*rf, max(mpf_get_prec(*af), mpf_get_prec(*bf)));
            mpf_mul(*rf, *af, *bf);
        }
#endif
#ifdef HAVE_MPFR
        else if (ISTYPE(a, mpfr_type)) {
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_t * rf = OBJ_AS_POINTER(r, mpfr_t), * af = OBJ_AS_POINTER(a, mpfr_t), * bf = OBJ_AS_POINTER(b, mpfr_t);

            mpfr_init2(*rf, max(mpfr_get_prec(*af), mpfr_get_prec(*bf)));
            mpfr_mul(*rf, *af, *bf, EZC_RND);
        }
#endif
         else {
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

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");

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
        }
#ifdef HAVE_GMP
        else if (ISTYPE(a, mpz_type)) {
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            mpz_div(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t), OBJ_AS_STRUCT(b, mpz_t));
        } else if (ISTYPE(a, mpf_type)) {
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpf_t * rf = OBJ_AS_POINTER(r, mpf_t), * af = OBJ_AS_POINTER(a, mpf_t), * bf = OBJ_AS_POINTER(b, mpf_t);
            mpf_init2(*rf, max(mpf_get_prec(*af), mpf_get_prec(*bf)));
            mpf_div(*rf, *af, *bf);
        }
#endif
#ifdef HAVE_MPFR
        else if (ISTYPE(a, mpfr_type)) {
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_t * rf = OBJ_AS_POINTER(r, mpfr_t), * af = OBJ_AS_POINTER(a, mpfr_t), * bf = OBJ_AS_POINTER(b, mpfr_t);

            mpfr_init2(*rf, max(mpfr_get_prec(*af), mpfr_get_prec(*bf)));
            mpfr_div(*rf, *af, *bf, EZC_RND);
        }
#endif
        else {
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

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_ALLOC_STRUCT(r, int);
            OBJ_AS_STRUCT(r, int) = __int_pow(OBJ_AS_STRUCT(a, int), OBJ_AS_STRUCT(b, int));
            r.type_id = int_type.id;
        } else if (ISTYPE(a, float_type)) {
            OBJ_ALLOC_STRUCT(r, float);
            r.type_id = float_type.id;
            OBJ_AS_STRUCT(r, float) = pow(OBJ_AS_STRUCT(a, float), OBJ_AS_STRUCT(b, float));
        }
#ifdef HAVE_GMP
        else if (ISTYPE(a, mpz_type)) {
            OBJ_ALLOC_STRUCT(r, mpz_t);
            r.type_id = mpz_type.id;
            mpz_init(OBJ_AS_STRUCT(r, mpz_t));
            int pval = mpz_get_si(OBJ_AS_STRUCT(b, mpz_t));
            if (pval < 0) {
                mpz_set_ui(OBJ_AS_STRUCT(r, mpz_t), 0);
            } else {
                mpz_pow_ui(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t), pval);
            }
        } else if (ISTYPE(a, mpf_type)) {
            #ifdef HAVE_MPFR
            OBJ_ALLOC_STRUCT(r, mpf_t);
            r.type_id = mpf_type.id;
            mpfr_t rfr, afr, bfr;
            mpfr_init2(afr, mpf_get_prec(OBJ_AS_STRUCT(a, mpf_t)));
            mpfr_init2(bfr, mpf_get_prec(OBJ_AS_STRUCT(b, mpf_t)));

            mpfr_set_f(afr, OBJ_AS_STRUCT(a, mpf_t), EZC_RND);
            mpfr_set_f(bfr, OBJ_AS_STRUCT(b, mpf_t), EZC_RND);

            mpfr_init2(rfr, max(mpfr_get_prec(afr), mpfr_get_prec(bfr)));

            mpfr_pow(rfr, afr, bfr, EZC_RND);

            mpfr_clear(afr);
            mpfr_clear(bfr);

            mpf_init2(OBJ_AS_STRUCT(r, mpf_t), mpfr_get_prec(rfr));

            mpfr_get_f(OBJ_AS_STRUCT(r, mpf_t), rfr, EZC_RND);

            mpfr_clear(rfr);

            #else
            raise_exception("please use mpfr for pow with floats", 1);
            #endif
        }
#endif
#ifdef HAVE_MPFR
        else if (ISTYPE(a, mpfr_type)) {
            OBJ_ALLOC_STRUCT(r, mpfr_t);
            r.type_id = mpfr_type.id;
            mpfr_t * rf = OBJ_AS_POINTER(r, mpfr_t), * af = OBJ_AS_POINTER(a, mpfr_t), * bf = OBJ_AS_POINTER(b, mpfr_t);

            mpfr_init2(*rf, max(mpfr_get_prec(*af), mpfr_get_prec(*bf)));
            mpfr_pow(*rf, *af, *bf, EZC_RND);
        }
#endif
        else {
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

    type_t int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

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
    }
#ifdef HAVE_GMP
    else if (ISTYPE(a, mpz_type)) {
        OBJ_ALLOC_STRUCT(r, mpz_t);
        r.type_id = mpz_type.id;
        mpz_init(OBJ_AS_STRUCT(r, mpz_t));
        mpz_neg(OBJ_AS_STRUCT(r, mpz_t), OBJ_AS_STRUCT(a, mpz_t));
    } else if (ISTYPE(a, mpf_type)) {
        OBJ_ALLOC_STRUCT(r, mpf_t);
        r.type_id = mpf_type.id;
        mpf_t * rf = OBJ_AS_POINTER(r, mpf_t), * af = OBJ_AS_POINTER(a, mpf_t);
        mpf_init2(*rf, mpf_get_prec(*af));
        mpf_neg(*rf, *af);
    }
#endif
    else {
        UNKNOWN_TYPE(type_name_from_id(a.type_id));
        return NULL_OBJ;    
    }
    return r;

    return NULL_OBJ;
}

/*

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

*/
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


// greater than
obj_t __op_gt(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    
    type_t bool_type = TYPE("bool"), int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");
    
    OBJ_ALLOC_STRUCT(r, bool);
    r.type_id = bool_type.id;

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_AS_STRUCT(r, bool) = OBJ_AS_STRUCT(a, int) > OBJ_AS_STRUCT(b, int);
        } else if (ISTYPE(a, float_type)) {
            OBJ_AS_STRUCT(r, bool) = OBJ_AS_STRUCT(a, float) > OBJ_AS_STRUCT(b, float);
        } else if (ISTYPE(a, str_type)) {
            OBJ_AS_STRUCT(r, bool) = strcmp(OBJ_AS_POINTER(b, char), OBJ_AS_POINTER(a, char)) > 0;
        } 
        return r;

    } else {
        OPERATOR_MISMATCH(">", at, bt);
    }

    return NULL_OBJ;
}



// greater than
obj_t __op_lt(obj_t a, obj_t b) {
    obj_t r;

    type_t at = OBJ_TYPE(a), bt = OBJ_TYPE(b);

    
    type_t bool_type = TYPE("bool"), int_type = TYPE("int"), float_type = TYPE("float"), str_type = TYPE("str");

    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");

    type_t mpfr_type = TYPE("mpfr");
    
    OBJ_ALLOC_STRUCT(r, bool);
    r.type_id = bool_type.id;

    if (at.id == bt.id) {
        if (ISTYPE(a, int_type)) {
            OBJ_AS_STRUCT(r, bool) = OBJ_AS_STRUCT(a, int) < OBJ_AS_STRUCT(b, int);
        } else if (ISTYPE(a, float_type)) {
            OBJ_AS_STRUCT(r, bool) = OBJ_AS_STRUCT(a, float) < OBJ_AS_STRUCT(b, float);
        } else if (ISTYPE(a, str_type)) {
            OBJ_AS_STRUCT(r, bool) = strcmp(OBJ_AS_POINTER(b, char), OBJ_AS_POINTER(a, char)) < 0;
        } 
        return r;

    } else {
        OPERATOR_MISMATCH(">", at, bt);
    }

    return NULL_OBJ;
}


void op_gt(runtime_t * runtime) STACK_OP(__op_gt)
void op_lt(runtime_t * runtime) STACK_OP(__op_lt)



/*

void op_floor(runtime_t * runtime) UNARY_OP(__op_floor)


*/

int init (int id, lib_t _lib) {
    init_exported(id, _lib);

    add_function("gt", "greater than", op_gt);
    add_function("lt", "less than", op_lt);

    add_function("add", "adds the last two items on the stack", op_add);
    add_function("sub", "subtracts the last two items on the stack ('a b sub!' goes to '(a-b)')", op_sub);

    add_function("mul", "multiplies the last two items on the stack", op_mul);

    add_function("div", "divides the last two items on the stack", op_div);

    add_function("pow", "for an (a, b) on the stack, it pops back on a^b", op_pow);

    add_function("neg", "if given a number, replaces the top of the stack with the negative value (1-x), or if it is a string, pop on the string reversed", op_neg);

    /*

    add_function("floor", "floors the last number", op_floor);
*/
    return 0;
}



