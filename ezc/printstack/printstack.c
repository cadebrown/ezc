
#define MODULE_NAME printstack

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void printstack(estack_t * stack) {
    int i;
    obj_t c_obj;
    type_t c_type;
    char * c_repr;
    printf("[%d] ", stack->len);
    for (i = 0; i < stack->len; ++i) {
        c_obj = estack_get(stack, i);
        c_type = utils.type_from_id(c_obj.type_id);
        c_type.representation(&c_obj, &c_repr);
        printf("'%s'", c_repr);
        free(c_repr);
        if (i != stack->len - 1) {
            printf(", ");
        }
    }
    printf("\n");
}


int init (int id, module_utils_t utils) {
    init_exported(id, utils);

    //add_type("dummy", type_dummy_constructor, type_dummy_parser, type_dummy_representation);
    add_function("printstack", printstack);

    return 0;
}



