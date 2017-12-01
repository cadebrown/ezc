
#define MODULE_NAME "ezc.types"

#define MODULE_DESCRIPTION "defines ezc standard library types"

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void byte_parser(obj_t * ret, char * value) {
    ret->data_len = 1;
    ret->data = malloc(ret->data_len);
    
    if (strlen(value) == 1) {
        ((char *)ret->data)[0] = value[0];
    } else if (strlen(value) > 2 && strlen(value) <= 4 && value[0] == '0' && value[1] == 'x' && strtol(value, NULL, 0) <= 0xff) {
        ((char *)ret->data)[0] = strtol(value, NULL, 0);
    } else {
        sprintf(to_raise, "error parsing byte from '%s'\n", value);
        raise_exception(to_raise, 1);
        return;
    }
}

void byte_constructor(obj_t * ret, obj_t value) {
    type_t str_type = type_from_name("str"), input_type = type_from_id(value.type_id);
    bool has_str = type_exists_name("str");
    
    // if passed in a string, parse it
    if (has_str && str_type.id == input_type.id) {
        byte_parser(ret, (char *)value.data);
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void byte_representation(obj_t * obj, char ** ret) {
    char v = ((char *)obj->data)[0];
    *ret = malloc(6);
    sprintf(*ret, "0x%x", 0xff & (int)v);
}

void byte_destroyer(obj_t * ret) {
    if (ret->data != NULL) {
        free(ret->data);
    }
    *ret = NULL_OBJ;
}

void byte_copier(obj_t * to, obj_t * from) {
    to->data_len = from->data_len;
    to->data = malloc(to->data_len);
    *(char *)to->data = *(char *)from->data;
}



void int_parser(obj_t * ret, char * value) {
    ret->data_len = sizeof(int);
    ret->data = malloc(ret->data_len);

    *((int *)ret->data) = strtol(value, NULL, 0);
}

void int_representation(obj_t * obj, char ** ret) {
    *ret = malloc(44);
    sprintf(*ret, "%d", *((int *)obj->data));
}

void int_constructor(obj_t * ret, obj_t value) {
    type_t input_type = type_from_id(value.type_id);
    type_t str_type = type_from_name("str");

    if (input_type.id == str_type.id) {
        int_parser(ret, value.data);
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void int_destroyer(obj_t * ret) {
    if (ret->data != NULL) {
        free(ret->data);
    }
    *ret = NULL_OBJ;
}

void int_copier(obj_t * to, obj_t * from) {
    to->data_len = from->data_len;
    to->data = malloc(to->data_len);
    *(int *)to->data = *(int *)from->data;
}



void str_parser(obj_t * ret, char * value) {
    ret->data_len = strlen(value) + 1;
    ret->data = malloc(ret->data_len);
    strcpy(ret->data, value);    
    ((char *)ret->data)[strlen(value)] = 0;
    
}

void str_representation(obj_t * obj, char ** ret) {
    *ret = malloc(obj->data_len);
    strcpy(*ret, obj->data);
}

void str_constructor(obj_t * ret, obj_t value) {
    type_t input_type = type_from_id(value.type_id);
    type_t str_type = type_from_name("str");

    if (input_type.id == str_type.id) {
        str_parser(ret, value.data);
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void str_destroyer(obj_t * ret) {
    if (ret->data != NULL) {
        free(ret->data);
    }
    *ret = NULL_OBJ;
}

void str_copier(obj_t * to, obj_t * from) {
    to->data_len = from->data_len;
    to->data = malloc(to->data_len);
    strcpy(to->data, from->data);
    ((char *)to->data)[to->data_len - 1] = 0;
}



int init (int type_id, module_utils_t utils) {

    init_exported(type_id, utils);

    add_type("byte", "1 byte (8 bits) of memory, and this corresponds to the C type 'char'", byte_constructor, byte_copier, byte_parser, byte_representation, byte_destroyer);
    add_type("int", "a system 'int' type, which is normally signed 32 bits", int_constructor, int_copier, int_parser, int_representation, int_destroyer);
    add_type("str", "a 'char *' in C, this is a string type implementation", str_constructor, str_copier, str_parser, str_representation, str_destroyer);

    return 0;
}



