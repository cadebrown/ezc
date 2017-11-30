
#define MODULE_NAME std_func

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


obj_t repr_obj(obj_t obj) {
    type_t str_type = utils.type_from_name("str");
    type_t obj_type = utils.type_from_id(obj.type_id);

    obj_t res;

    char * repr_str;

    obj_type.representation(&obj, &repr_str);

    str_type.parser(&res, repr_str);

    free(repr_str);

    return res;
}

void repr(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        printf("repr: error: no items on stack\n");
        return;
    }

    if (!utils.type_exists_name("str")) {
        printf("repr: error: no 'str' type\n");
        return;
    }


    estack_push(&runtime->stack, repr_obj(estack_pop(&runtime->stack)));
}

void dump(runtime_t * runtime) {
    int i;

    printf("globals[%d]:\n", runtime->globals.len);
    obj_t c_repr;
    type_t c_type;
    for (i = 0; i < runtime->globals.len; ++i) {
        c_repr = repr_obj(runtime->globals.vals[i]);
        c_type = utils.type_from_id(runtime->globals.vals[i].type_id);
        printf ("  '%s': '%s':%s\n", runtime->globals.keys[i], (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }

    printf("stack[%d]:\n", runtime->stack.len);

    for (i = 0; i < runtime->stack.len; ++i) {
        c_repr = repr_obj(runtime->stack.vals[i]);
        c_type = utils.type_from_id(runtime->stack.vals[i].type_id);
        printf ("  %d: '%s':%s\n", i, (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }
}

void delete_last_item(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        printf("no items to delete\n");
        return;
    }
    obj_t last_item = estack_pop(&runtime->stack);
    obj_free(&last_item);
}

void eval_last_item(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        printf("no item to run as code\n");
        return;
    }
    obj_t last_item = estack_pop(&runtime->stack);
    type_t str_type = utils.type_from_name("str");
    if (last_item.type_id != str_type.id) {
        printf("error: trying to eval non-'str' object as code\n");
        return;
    }

    run_code(runtime, (char *)last_item.data);
}

void concat(runtime_t * runtime) {
    if (runtime->stack.len <= 1) {
        printf("no items to concatenate\n");
        return;
    }
    obj_t a, b, r;
    type_t str_type = utils.type_from_name("str");
    b = estack_pop(&runtime->stack);
    a = estack_pop(&runtime->stack);
    char * res = malloc(a.data_len + b.data_len);
    strcpy(res, a.data);
    strcat(res, b.data);
    str_type.parser(&r, res);

    free(res);

    estack_push(&runtime->stack, r);
}

void concat_repeat(runtime_t * runtime) {
    while (runtime->stack.len >= 2) {
        concat(runtime);
    }
}



int init (int id, module_utils_t utils) {
    init_exported(id, utils);

    add_function("repr", repr);
    add_function("dump", dump);
    add_function("del", delete_last_item);
    add_function("eval", eval_last_item);
    add_function("concat", concat);
    add_function("concat*", concat_repeat);

    return 0;
}



