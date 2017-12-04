
#define MODULE_NAME "ezc.gmp"

#define MODULE_DESCRIPTION "gmplib.org extensions"

#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>

#ifdef HAVE_MPFR
#include <mpfr.h>
#endif

#include <math.h>


int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}



void mpz_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_STRUCT(*ret, mpz_t);
    mpz_init(OBJ_AS_STRUCT(*ret, mpz_t));

    if (gmp_sscanf(value, "%Zd", OBJ_AS_POINTER(*ret, mpz_t)) != 1) {
        sprintf(to_raise, "error parsing mpz from '%s'\n", value);
        raise_exception(to_raise, 1);
    }
}

void mpz_constructor(obj_t * ret, obj_t value) {

    type_t str_type = TYPE("str"), int_type = TYPE("int");

    // if passed in a string, parse it
    if (ISTYPE(value, str_type)) {
        mpz_parser(ret, OBJ_AS_POINTER(value, char));
    } else if (ISTYPE(value, int_type)) {
        OBJ_ALLOC_STRUCT(*ret, mpz_t);
        mpz_init_set_si(OBJ_AS_STRUCT(*ret, mpz_t), OBJ_AS_STRUCT(value, int));
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}


void mpz_representation(obj_t * obj, char ** ret) {
    *ret = mpz_get_str(NULL, 10, OBJ_AS_STRUCT(*obj, mpz_t));
}

void mpz_destroyer(obj_t * ret) {
    mpz_clear(OBJ_AS_STRUCT(*ret, mpz_t));
    OBJ_FREE_MEM(*ret)
}

void mpz_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, mpz_t);
    mpz_init(OBJ_AS_STRUCT(*to, mpz_t));
    mpz_set(OBJ_AS_STRUCT(*to, mpz_t), OBJ_AS_STRUCT(*from, mpz_t));
}


void mpf_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_STRUCT(*ret, mpf_t);
    mpf_init(OBJ_AS_STRUCT(*ret, mpf_t));

    if (gmp_sscanf(value, "%Ff", OBJ_AS_POINTER(*ret, mpf_t)) != 1) {
        sprintf(to_raise, "error parsing mpf from '%s'\n", value);
        raise_exception(to_raise, 1);
    }
}

void mpf_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str"), float_type = TYPE("float");

    type_t mpfr_type = TYPE("mpfr");

    // if passed in a string, parse it
    if (ISTYPE(value, str_type)) {
        mpf_parser(ret, OBJ_AS_POINTER(value, char));
    } else if (ISTYPE(value, float_type)) {
        OBJ_ALLOC_STRUCT(*ret, mpf_t);
        mpf_init_set_d(OBJ_AS_STRUCT(*ret, mpf_t), OBJ_AS_STRUCT(value, float));
    }
#ifdef HAVE_MPFR
    else if (ISTYPE(value, mpfr_type)) {
        OBJ_ALLOC_STRUCT(*ret, mpf_t);
        mpf_init2(OBJ_AS_STRUCT(*ret, mpf_t), mpfr_get_prec(OBJ_AS_STRUCT(value, mpfr_t)));
        mpfr_get_f(OBJ_AS_STRUCT(*ret, mpf_t), OBJ_AS_STRUCT(value, mpfr_t), EZC_RND);
    }
#endif
    else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}



void mpf_representation(obj_t * obj, char ** ret) {
    mp_exp_t exptr;
    char * rgets = mpf_get_str(NULL, &exptr, 10, 0, OBJ_AS_STRUCT(*obj, mpf_t));
    bool isneg = rgets[0] == '-';
    //printf("mpfgetstr: %d,'%s'\n", exptr, rgets);
    if (exptr == 0) {
        *ret = malloc(strlen(rgets) + 3);
        if (isneg) {
            sprintf(*ret, "-0.%s", rgets + 1);
        } else {
            sprintf(*ret, "0.%s", rgets);
        }
    } else if (exptr < 0) {
        *ret = malloc(strlen(rgets) + 4 + abs(exptr));
        if (isneg) {
            sprintf(*ret, "-0.");
        } else {
            sprintf(*ret, "0.");
        }
        int i;
        for (i = 0; i < abs(exptr); ++i) {
            sprintf(*ret + (2 + i + isneg), "0");
        }
        sprintf(*ret + (2 + i + isneg), "%s", rgets + isneg);
    } else {
        if (exptr + isneg > strlen(rgets)) {
            *ret = malloc(exptr + 5);
            sprintf(*ret, "%s", rgets);
            int i;
            for (i = strlen(*ret); i < exptr + isneg; ++i) {
                (*ret)[i] = '0';
            }
            (*ret)[i++] = '.';
            (*ret)[i++] = '0';
            (*ret)[i] = '\0';
        } else if (exptr + isneg < strlen(rgets)) {
            *ret = malloc(strlen(rgets) + 4);
            if (isneg) {
                sprintf(*ret, "-%.*s.%s", (int)exptr, rgets + isneg, rgets + exptr + isneg);
            } else {
                sprintf(*ret, "%.*s.%s", (int)exptr, rgets + isneg, rgets + exptr + isneg);
            }
        } else {
            *ret = malloc(strlen(rgets) + 4);
            sprintf(*ret, "%s.0", rgets);
        }
    }
    free(rgets);
    //*ret = realloc(*ret, strlen(*ret) + 2);
    
}

void mpf_destroyer(obj_t * ret) {
    mpf_clear(OBJ_AS_STRUCT(*ret, mpf_t));
    OBJ_FREE_MEM(*ret)
}

void mpf_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, mpf_t);
    mpf_init(OBJ_AS_STRUCT(*to, mpf_t));
    mpf_set(OBJ_AS_STRUCT(*to, mpf_t), OBJ_AS_STRUCT(*from, mpf_t));
}


// MPFR optional

#ifdef HAVE_MPFR

void mpfr_parser(obj_t * ret, char * value) {

    OBJ_ALLOC_STRUCT(*ret, mpfr_t);
    mpfr_init_set_str(OBJ_AS_STRUCT(*ret, mpfr_t), value, 10, EZC_RND);
}

void mpfr_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str"), float_type = TYPE("float"), mpf_type = TYPE("mpf");

    // if passed in a string, parse it
    if (ISTYPE(value, str_type)) {
        mpfr_parser(ret, OBJ_AS_POINTER(value, char));
    } else if (ISTYPE(value, float_type)) {
        mpfr_init_set_d(OBJ_AS_STRUCT(*ret, mpfr_t), OBJ_AS_STRUCT(value, float), EZC_RND);
    } else if (ISTYPE(value, mpf_type)) {
        mpfr_init_set_f(OBJ_AS_STRUCT(*ret, mpfr_t), OBJ_AS_STRUCT(value, mpf_t), EZC_RND);
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}


void mpfr_representation(obj_t * obj, char ** ret) {
    mp_exp_t exptr;
    char * rgets = mpfr_get_str(NULL, &exptr, 10, 0, OBJ_AS_STRUCT(*obj, mpfr_t), EZC_RND);
    bool isneg = rgets[0] == '-';
    //printf("mpfrgetstr: %d,'%s'\n", exptr, rgets);
    if (exptr == 0) {
        *ret = malloc(strlen(rgets) + 3);
        if (isneg) {
            sprintf(*ret, "-0.%s", rgets + 1);
        } else {
            sprintf(*ret, "0.%s", rgets);
        }
    } else if (exptr < 0) {
        *ret = malloc(strlen(rgets) + 4 + abs(exptr));
        if (isneg) {
            sprintf(*ret, "-0.");
        } else {
            sprintf(*ret, "0.");
        }
        int i;
        for (i = 0; i < abs(exptr); ++i) {
            sprintf(*ret + (2 + i + isneg), "0");
        }
        sprintf(*ret + (2 + i + isneg), "%s", rgets + isneg);
    } else {
        if (exptr + isneg > strlen(rgets)) {
            *ret = malloc(exptr + 5);
            sprintf(*ret, "%s", rgets);
            int i;
            for (i = strlen(*ret); i < exptr + isneg; ++i) {
                (*ret)[i] = '0';
            }
            (*ret)[i++] = '.';
            (*ret)[i++] = '0';
            (*ret)[i] = '\0';
        } else if (exptr + isneg < strlen(rgets)) {
            *ret = malloc(strlen(rgets) + 4);
            if (isneg) {
                sprintf(*ret, "-%.*s.%s", (int)exptr, rgets + isneg, rgets + exptr + isneg);
            } else {
                sprintf(*ret, "%.*s.%s", (int)exptr, rgets + isneg, rgets + exptr + isneg);
            }
        } else {
            *ret = malloc(strlen(rgets) + 4);
            sprintf(*ret, "%s.0", rgets);
        }
    }
    free(rgets);
    //*ret = realloc(*ret, strlen(*ret) + 2);
    
}

void mpfr_destroyer(obj_t * ret) {
    mpfr_clear(OBJ_AS_STRUCT(*ret, mpfr_t));
    OBJ_FREE_MEM(*ret)
}

void mpfr_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, mpfr_t);
    mpfr_init2(OBJ_AS_STRUCT(*to, mpfr_t), mpfr_get_prec(OBJ_AS_STRUCT(*from, mpfr_t)));
    mpfr_set(OBJ_AS_STRUCT(*to, mpfr_t), OBJ_AS_STRUCT(*from, mpfr_t), EZC_RND);
}

#endif




int init (int type_id, module_utils_t utils) {

    init_exported(type_id, utils);

    mpf_set_default_prec(GMP_MPF_DEFAULT_PREC);

    add_type("mpz", "multiprecision integer extension of gmp's mpz_t", mpz_constructor, mpz_copier, mpz_parser, mpz_representation, mpz_destroyer);
    add_type("mpf", "multiprecision float extension of gmp's mpf_t", mpf_constructor, mpf_copier, mpf_parser, mpf_representation, mpf_destroyer);
#ifdef HAVE_MPFR
    mpfr_set_default_prec(GMP_MPF_DEFAULT_PREC);
    mpfr_set_default_rounding_mode(EZC_RND);

    add_type("mpfr", "multiprecision float extension of mpfr's mpfr_t", mpfr_constructor, mpfr_copier, mpfr_parser, mpfr_representation, mpfr_destroyer);
#endif

    return 0;
}



