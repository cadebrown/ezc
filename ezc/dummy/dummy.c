
#define MODULE_NAME dummy

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void type_dummy_parser(obj_t * ret, char * value) {
    ret->data = malloc(strlen(value));
    ret->data_len = strlen(value);
    ret->type_id = utils.type_id_from_name("dummy");
    strcpy(ret->data, value);
}

void type_dummy_constructor(obj_t * ret, obj_t input) {
    type_t str_type = utils.type_from_name("str"), input_type = utils.type_from_id(input.type_id);
    bool has_str = utils.type_exists_name("str");

    // if passed in a string, parse it
    if (has_str && str_type.id == input_type.id) {
        ret->type_id = utils.type_id_from_name("dummy");
        type_dummy_parser(ret, (char *)input.data);
    } else {
        ret->type_id = 0;
        printf("Don't know how to create 'dummy' from type '%s'\n", input_type.name);
    }
    /*
    type_t h_this;
    bool worked = (*utils.get_type_name)(&h_this, "dummy");
    */
}

void type_dummy_representation(obj_t * obj, char ** ret) {
    *ret = malloc(obj->data_len);
    strcpy(*ret, obj->data);
}


int init (int id, module_utils_t utils) {
    init_exported(id, utils);

    add_type("dummy", type_dummy_constructor, type_dummy_parser, type_dummy_representation);

    return 0;
}



