
#define MODULE_NAME systypes

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void byte_parser(obj_t * ret, char * value) {
    ret->data_len = 1;
    ret->data = malloc(ret->data_len);
    
    if (strlen(value) == 1) {
        ((char *)ret->data)[0] = value[0];
        ret->type_id = utils.type_id_from_name("byte");
    } else if (strlen(value) > 2 && strlen(value) <= 4 && value[0] == '0' && value[1] == 'x' && strtol(value, NULL, 0) <= 0xff) {
        ((char *)ret->data)[0] = strtol(value, NULL, 0);
        ret->type_id = utils.type_id_from_name("byte");
    } else {
        ret->type_id = 0;
        printf("error constructing byte from '%s'\n", value);
    }
}

void byte_constructor(obj_t * ret, obj_t value) {
    type_t str_type = utils.type_from_name("str"), input_type = utils.type_from_id(value.type_id);
    bool has_str = utils.type_exists_name("str");
    
    // if passed in a string, parse it
    if (has_str && str_type.id == input_type.id) {
        byte_parser(ret, (char *)value.data);
    } else {
        ret->type_id = 0;
        printf("Don't know how to create 'byte' from type '%s'\n", input_type.name);
    }
}


void byte_representation(obj_t * obj, char ** ret) {
    char v = ((char *)obj->data)[0];
    *ret = malloc(5);
    (*ret)[0] = '0';
    (*ret)[1] = 'x';
    (*ret)[2] = (v >> 4) + '0';
    (*ret)[3] = (v & 15) + '0';
    (*ret)[4] = 0;
}


void str_parser(obj_t * ret, char * value) {
    ret->data_len = strlen(value);
    ret->data = malloc(ret->data_len);
    ret->type_id = utils.type_id_from_name("str");
    strcpy(ret->data, value);    
}

void str_representation(obj_t * obj, char ** ret) {
    *ret = malloc(obj->data_len);
    strcpy(*ret, obj->data);
}

void str_constructor(obj_t * ret, obj_t value) {
    type_t input_type = utils.type_from_id(value.type_id);
    char * repr;
    str_representation(&value, &repr);
    str_parser(ret, repr);
}



int init (int type_id, module_utils_t utils) {

    init_exported(type_id, utils);

    add_type("byte", byte_constructor, byte_parser, byte_representation);
    add_type("str", str_constructor, str_parser, str_representation);

    return 0;
}



