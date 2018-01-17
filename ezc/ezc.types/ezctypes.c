
#define MODULE_NAME "ezc.types"

#define MODULE_DESCRIPTION "defines ezc standard library types"

#include "ezcmodule.h"
#include "../ezcmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef HAVE_GMP
#include <gmp.h>
#endif

#ifdef HAVE_MPFR
#include <mpfr.h>
#endif



void byte_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_STRUCT(*ret, char)

    if (strlen(value) == 1) {
        OBJ_AS_STRUCT(*ret, char) = value[0];
    } else if (strlen(value) > 2 && strlen(value) <= 4 && value[0] == '0' && value[1] == 'x' && strtol(value, NULL, 0) <= 0xff) {
        OBJ_AS_STRUCT(*ret, char) = strtol(value, NULL, 0);
    } else {
        sprintf(to_raise, "error parsing byte from '%s'\n", value);
        raise_exception(to_raise, 1);
    }
}

void byte_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str"), input_type = OBJ_TYPE(value);

    // if passed in a string, parse it
    if (str_type.id == input_type.id) {
        byte_parser(ret, OBJ_AS_POINTER(value, char));
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void byte_representation(obj_t * obj, char ** ret) {
    *ret = malloc(6);
    sprintf(*ret, "0x%x", 0xff & (int)OBJ_AS_STRUCT(*obj, char));
}

void byte_destroyer(obj_t * ret) {
    OBJ_FREE_MEM(*ret)
}

void byte_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, char);
    OBJ_STRUCT_COPY(*to, *from, char);
}



void int_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_STRUCT(*ret, int);
    OBJ_AS_STRUCT(*ret, int) = strtol(value, NULL, 0);
}


int __int_strlen(int _x) {
    int x = 0;
    int l = 0;
    if (_x >= 0) {
        x = _x;
    } else {
        l++;
        x = -_x;
    }
    while (x>9) { 
        l++; 
        x /= 10; 
    }
    return l;
}


// fast method to represent it
void int_representation(obj_t * obj, char ** ret) {
    int v = *((int *)obj->data);
    *ret = malloc(__int_strlen(v) + 1);

    int c_off = 0;
    int esign_off = 0;
    int term_idx = 0;

    if (v < 0) {
        v = -v;
        (*ret)[c_off] = '-';
        esign_off = 1;
    }

    c_off = __int_strlen(v) + esign_off;

    term_idx = c_off + 1;

    while (c_off >= esign_off) {
        (*ret)[c_off--] = (v % 10) + '0';
        v /= 10;
    }

    (*ret)[term_idx] = '\0';
}

void int_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str"), int_type = TYPE("int"), float_type = TYPE("float");

#ifdef HAVE_GMP
    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");
#endif
#ifdef HAVE_MPFR
    type_t mpfr_type = TYPE("mpfr");
#endif

    if (ISTYPE(value, str_type)) {
        int_parser(ret, OBJ_AS_POINTER(value, char));
    } else if (ISTYPE(value, int_type)) {
        OBJ_ALLOC_STRUCT(*ret, int);
        OBJ_STRUCT_COPY(*ret, value, int);
    } else if (ISTYPE(value, float_type)) {
        OBJ_ALLOC_STRUCT(*ret, int);
        OBJ_AS_STRUCT(*ret, int) = OBJ_AS_STRUCT(value, float);
    }
#ifdef HAVE_GMP
    else if (ISTYPE(value, mpz_type)) {
        OBJ_ALLOC_STRUCT(*ret, int);
        OBJ_AS_STRUCT(*ret, int) = mpz_get_si(OBJ_AS_STRUCT(value, mpz_t));
    } else if (ISTYPE(value, mpf_type)) {
        OBJ_ALLOC_STRUCT(*ret, int);
        OBJ_AS_STRUCT(*ret, int) = mpf_get_si(OBJ_AS_STRUCT(value, mpf_t));
    }
#endif
#ifdef HAVE_MPFR
    else if (ISTYPE(value, mpfr_type)) {
        OBJ_ALLOC_STRUCT(*ret, int);
        OBJ_AS_STRUCT(*ret, int) = mpfr_get_si(OBJ_AS_STRUCT(value, mpfr_t), EZC_RND);
    }
#endif
    else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void int_destroyer(obj_t * ret) {
    OBJ_FREE_MEM(*ret);
}

void int_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, *from);
    OBJ_STRUCT_COPY(*to, *from, int);
}



void float_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_STRUCT(*ret, float);
    
    if (strlen(value) >= 1) {
        OBJ_AS_STRUCT(*ret, float) = strtof(value, NULL);
    } else {
        sprintf(to_raise, "error parsing float from '%s'\n", value);
        raise_exception(to_raise, 1);
    }
}

void float_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str"), int_type = TYPE("int"), float_type = TYPE("float");
    
#ifdef HAVE_GMP
    type_t mpz_type = TYPE("mpz"), mpf_type = TYPE("mpf");
#endif
#ifdef HAVE_MPFR
    type_t mpfr_type = TYPE("mpfr");
#endif

    // if passed in a string, parse it
    if (ISTYPE(value, str_type)) {
        float_parser(ret, (char *)value.data);
    } else if (ISTYPE(value, float_type)) {
        OBJ_ALLOC_STRUCT(*ret, float);
        OBJ_STRUCT_COPY(*ret, value, float);
    } else if (ISTYPE(value, int_type)) {
        OBJ_ALLOC_STRUCT(*ret, float);
        OBJ_AS_STRUCT(*ret, float) = OBJ_AS_STRUCT(value, int);
    } 
#ifdef HAVE_GMP
    else if (ISTYPE(value, mpz_type)) {
        OBJ_ALLOC_STRUCT(*ret, float);
        OBJ_AS_STRUCT(*ret, float) = mpz_get_si(OBJ_AS_STRUCT(value, mpz_t));
    } else if (ISTYPE(value, mpf_type)) {
        OBJ_ALLOC_STRUCT(*ret, float);
        OBJ_AS_STRUCT(*ret, float) = mpf_get_d(OBJ_AS_STRUCT(value, mpf_t));
    }
#endif
#ifdef HAVE_MPFR
    else if (ISTYPE(value, mpfr_type)) {
        OBJ_ALLOC_STRUCT(*ret, float);
        OBJ_AS_STRUCT(*ret, float) = mpfr_get_d(OBJ_AS_STRUCT(value, mpfr_t), EZC_RND);
    }
#endif
    else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void float_representation(obj_t * obj, char ** ret) {
    float v = OBJ_AS_STRUCT(*obj, float);
    const char float_fmt[] = "%f";
    int sz_needed = snprintf(NULL, 0, float_fmt, v);
    
    *ret = malloc(sz_needed + 1);

    sprintf(*ret, float_fmt, v);
}

void float_copier(obj_t * to, obj_t * from) {
    OBJ_ALLOC_STRUCT(*to, float);
    OBJ_STRUCT_COPY(*to, *from, float);
}


void float_destroyer(obj_t * ret) {
    OBJ_FREE_MEM(*ret);
}



void str_parser(obj_t * ret, char * value) {
    OBJ_ALLOC_BYTES(*ret, strlen(value) + 1);
    strcpy(ret->data, value);
}

void str_representation(obj_t * obj, char ** ret) {
    *ret = malloc(obj->data_len);
    strcpy(*ret, obj->data);
}


void str_constructor(obj_t * ret, obj_t value) {
    type_t str_type = TYPE("str");

    if (ISTYPE(value, str_type)) {
        str_parser(ret, OBJ_AS_POINTER(value, char));
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void str_destroyer(obj_t * ret) {
    OBJ_FREE_MEM(*ret);
}

void str_copier(obj_t * to, obj_t * from) {
    to->data_len = from->data_len;
    to->data = malloc(to->data_len);
    strcpy(to->data, from->data);
}


int init (int type_id, lib_t _lib) {

    init_exported(type_id, _lib);

    add_type("byte", "1 byte (8 bits) of memory, and this corresponds to the C type 'char'", byte_constructor, byte_copier, byte_parser, byte_representation, byte_destroyer);
    add_type("int", "a system 'int' type, which is normally signed 32 bits", int_constructor, int_copier, int_parser, int_representation, int_destroyer);
    add_type("float", "a system 'float' type, which is normally 32 bits", float_constructor, float_copier, float_parser, float_representation, float_destroyer);
    add_type("str", "a 'char *' in C, this is a string type implementation", str_constructor, str_copier, str_parser, str_representation, str_destroyer);

    return 0;
}



