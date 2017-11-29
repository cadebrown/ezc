
#define MODULE_NAME dummy

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int num_types = 1;
type_t * types;

void type_dummy_constructor(obj_t * ret, char * value) {
    ret->data = malloc(strlen(value));
    ret->data_len = strlen(value);
    ret->type_id = types[0].id;
    strcpy(ret->data, value);
}

void type_dummy_representation(char ** ret, obj_t obj) {
    *ret = malloc(obj.data_len);
    strcpy(*ret, obj.data);
}


int init (int type_id) {
    printf("Module 'dummy' says hello\n");

    types = malloc(num_types * sizeof(type_t));

    types[0].name = "dummy";
    types[0].id = type_id;
    types[0].constructor = type_dummy_constructor;
    types[0].representation = type_dummy_representation;

    return 0;
}



