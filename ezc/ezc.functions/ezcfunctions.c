
#define MODULE_NAME "ezc.functions"

#define MODULE_DESCRIPTION "defines functions in the ezc standard library"


#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


obj_t repr_obj(obj_t obj) {
    type_t str_type = TYPE("str");

    obj_t res;

    if (obj.type_id == NULL_TYPE.id) {
        res = obj_parse(str_type, "null");
    } else {
        char * repr_str;

        type_t obj_type = OBJ_TYPE(obj);

        repr_str = obj_representation(obj_type, &obj);

        res = obj_parse(str_type, repr_str);
    
        free(repr_str);
    }

    return res;
}

void repr(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    obj_t cobj = estack_pop(&runtime->stack);
    estack_push(&runtime->stack, repr_obj(cobj));
    obj_free(&cobj);
}

void repr_recursive(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        return;
    }

    int i;
    for (i = 0; i < runtime->stack.len; ++i) {
        obj_t cobj = estack_get(&runtime->stack, i);
        estack_set(&runtime->stack, i, repr_obj(cobj));
        obj_free(&cobj);
    }
}


void _print_obj(obj_t to_print) {
    char * str_repr;

    str_repr = obj_representation(OBJ_TYPE(to_print), &to_print);

    printf("%s", str_repr);

    free(str_repr);
}

void print_obj(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    obj_t last_obj = estack_pop(&runtime->stack);

    _print_obj(last_obj);

    printf("\n");

    obj_free(&last_obj);
}

void swap(runtime_t * runtime) {
    ASSURE_STACK_LEN(2);

    estack_swaptop(&runtime->stack);
}

void print_obj_recursive(runtime_t * runtime) {
    int i;
    for (i = 0; i < runtime->stack.len; ++i) {
        _print_obj(estack_get(&runtime->stack, i));
        if (i != runtime->stack.len - 1) printf(", ");
    }
    printf("\n");
}

void copy(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        raise_exception("no items to copy", 1);
        return;
    }
    obj_t last = estack_get(&runtime->stack, runtime->stack.len - 1);
    obj_t copyof = obj_copy(last);

    estack_push(&runtime->stack, copyof);
}

void dump(runtime_t * runtime) {
    int i;

    printf("globals[%d]:\n", runtime->globals.len);
    obj_t c_repr;
    type_t c_type;
    for (i = 0; i < runtime->globals.len; ++i) {
        c_repr = repr_obj(runtime->globals.vals[i]);
        c_type = type_from_id(runtime->globals.vals[i].type_id);
        printf ("  '%s': '%s':%s\n", runtime->globals.keys[i], (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }

    printf("stack[%d]:\n", runtime->stack.len);

    for (i = 0; i < runtime->stack.len; ++i) {
        c_repr = repr_obj(runtime->stack.vals[i]);
        c_type = type_from_id(runtime->stack.vals[i].type_id);
        printf ("  %d: '%s':%s\n", i, (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }

    printf("\n");

}

void delete_last_item(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    obj_t last_item = estack_pop(&runtime->stack);
    obj_free(&last_item);
}


void delete_last_item_repeat(runtime_t * runtime) {
    while (runtime->stack.len > 0) {
        delete_last_item(runtime);
    }
}

void eval_last_item(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    obj_t last_obj = estack_pop(&runtime->stack);
    char * code_to_run;
    str_obj_force(&code_to_run, last_obj);
    
    runnable_t last_expr;
    runnable_init_str(&last_expr, code_to_run);
    last_expr.from = "lambda";

    run_runnable(runtime, &last_expr);

    free(code_to_run);
    obj_free(&last_obj);
}

void concat(runtime_t * runtime) {
    ASSURE_STACK_LEN(2);

    obj_t a, b, r;
    type_t str_type = TYPE("str");
    b = estack_pop(&runtime->stack);
    a = estack_pop(&runtime->stack);

    if (a.type_id != str_type.id || b.type_id != str_type.id) {
        raise_exception("arguments are not 'str' objects", 1);
        return;
    }
    char * res = malloc(a.data_len + b.data_len - 1);
    strcpy(res, a.data);
    strcat(res, b.data);

    r = obj_parse(str_type, res);

    free(res);

    obj_free(&a);
    obj_free(&b);

    estack_push(&runtime->stack, r);
}

void concat_repeat(runtime_t * runtime) {
    if (runtime->stack.len <= 1) {
        return;
    }

    type_t str_type = TYPE("str");

    int total_len = 0;

    obj_t cobj;

    int i;
    for (i = 0; i < runtime->stack.len; ++i) {
        cobj = estack_get(&runtime->stack, i);
        if (cobj.type_id != str_type.id) {
            raise_exception("some objects on the stack are not strings, use repr& function to convert them", 1);
        }
        total_len += cobj.data_len - 1;
    }
    
    char * res = malloc(total_len + 1);
    *res = '\0';

    for (i = 0; i < runtime->stack.len; ++i) {
        cobj = estack_get(&runtime->stack, i);
        strcat(res, (char *)cobj.data);

        obj_free(&cobj);
    }

    obj_t ret;

    ret = obj_parse(str_type, res);

    free(res);

    runtime->stack.len = 0;

    estack_push(&runtime->stack, ret);

}


// join takes (... a, b, c) and transforms it into (... "acb")
void join(runtime_t * runtime) {
    ASSURE_STACK_LEN(3);

    obj_t a, b, c, r;
    type_t str_type = TYPE("str");
    c = estack_pop(&runtime->stack);
    b = estack_pop(&runtime->stack);
    a = estack_pop(&runtime->stack);

    if (a.type_id != str_type.id || b.type_id != str_type.id || b.type_id != str_type.id) {
        raise_exception("arguments are not 'str' objects", 1);
        return;
    }

    char * res = malloc(a.data_len + b.data_len + c.data_len - 2);
    strcpy(res, a.data);
    strcat(res, c.data);
    strcat(res, b.data);

    r = obj_parse(str_type, res);

    free(res);

    obj_free(&a);
    obj_free(&b);
    obj_free(&c);

    estack_push(&runtime->stack, r);
}

// takes (n0, n1, n2, n3, c) and ends up with ("n0cn1cn2cn3")

void join_repeat(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    if (runtime->stack.len <= 1) {
        return;
    }

    type_t str_type = TYPE("str");

    int total_len = 0;

    // this is the joiner
    obj_t joiner = estack_pop(&runtime->stack);

    obj_t cobj;

    int i;
    for (i = 0; i < runtime->stack.len; ++i) {
        cobj = estack_get(&runtime->stack, i);
        if (cobj.type_id != str_type.id) {
            raise_exception("some objects on the stack are not strings, use repr& function to convert them", 1);
        }
        total_len += cobj.data_len - 1;
        if (i != runtime->stack.len - 1) total_len += joiner.data_len - 1;
    }
    
    char * res = malloc(total_len + 1);
    *res = '\0';

    for (i = 0; i < runtime->stack.len; ++i) {
        cobj = estack_get(&runtime->stack, i);
        strcat(res, (char *)cobj.data);
        if (i != runtime->stack.len - 1) strcat(res, (char *)joiner.data);

        obj_free(&cobj);
    }

    obj_free(&joiner);

    obj_t ret = obj_parse(str_type, res);

    free(res);

    runtime->stack.len = 0;

    estack_push(&runtime->stack, ret);

}



void import(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    char * module_name;
    obj_t last_obj = estack_pop(&runtime->stack);

    str_obj_force(&module_name, last_obj);

    import_module(module_name, true);

    free(module_name);
    obj_free(&last_obj);
}

void import_repeat(runtime_t * runtime) {
    while (runtime->stack.len >= 1) {
        import(runtime);
    }
}

#define print_func_addr(ijk, fncaddr) printf("0x"); for (ijk=0; ijk<sizeof (fncaddr); ijk++) printf("%.2x", ((unsigned char *)&fncaddr)[ijk]);

bool print_type_id(int id) {
    if (!type_exists_id(id)) {
        raise_exception("unknown type", 1);
        return false;
    }
    type_t type = type_from_id(id);

    printf("%s%s\n", CAST, type.name);
    printf("  %s\n\n", type.description);
    printf("  id: %d\n", type.id);

    size_t ijk; 

    printf("  constructor: ");
    print_func_addr(ijk, type.constructor);
    printf("\n  copier: ");
    print_func_addr(ijk, type.copier);
    printf("\n  parser: ");
    print_func_addr(ijk, type.parser);
    printf("\n  representation: ");
    print_func_addr(ijk, type.representation);
    printf("\n  destroyer: ");
    print_func_addr(ijk, type.destroyer);
    printf("\n");

    return true;
}


bool print_func_id(int id) {
    if (!function_exists_id(id)) {
        raise_exception("unknown function", 1);
        return false;
    }

    function_t func = function_from_id(id);

    printf("%s%s\n", func.name, CALL_FUNCTION);
    printf("  %s\n\n", func.description);
    printf("  id: %d\n", func.id);
    
    size_t ijk;
    printf("  addr: ");
    print_func_addr(ijk, func.function);
    printf("\n");
    
    return true;
}

bool print_module_name(char * name) {
    module_t name_module;

    bool found_module = false;

    int i;
    for (i = 0; i < num_registered_modules; ++i) {
        if (strcmp(registered_modules[i].name, name) == 0) {
            name_module = registered_modules[i];
            found_module = true;
            break;
        }
    }

    if (!found_module) {
        sprintf(to_raise, "unknown module '%s'", name);
        raise_exception(to_raise, 1);
        return false;
    } else {
        printf("[%s]\n", name_module.name);
        printf("  %s\n\n", name_module.exported.description);

        printf("  types[%d]: ", name_module.exported.num_types);
        for (i = 0; i < name_module.exported.num_types; ++i) {
            printf("%s", name_module.exported.types[i].name);
            if (i != name_module.exported.num_types - 1) printf(", ");
        }
        printf("\n");

        printf("  functions[%d]: ", name_module.exported.num_functions);
        for (i = 0; i < name_module.exported.num_functions; ++i) {
            printf("%s", name_module.exported.functions[i].name);
            if (i != name_module.exported.num_functions - 1) printf(", ");
        }
        printf("\n");
        return true;
    }
}

void funcinfo(runtime_t * runtime) {
    ASSURE_NONEMPTY_STACK;

    obj_t last_obj = estack_pop(&runtime->stack);

    char * func_name;

    str_obj_force(&func_name, last_obj);

    int func_id = FUNCTION(func_name).id;

    print_func_id(func_id);

    obj_free(&last_obj);
    free(func_name);

}

void typeinfo(runtime_t * runtime) {
    ASSURE_NONEMPTY_STACK;

    obj_t last_obj = estack_pop(&runtime->stack);

    char * type_name;

    str_obj_force(&type_name, last_obj);

    int type_id = TYPE(type_name).id;

    print_type_id(type_id);

    obj_free(&last_obj);
    free(type_name);
}


void moduleinfo(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    obj_t last_obj = estack_pop(&runtime->stack);

    char * module_name;

    str_obj_force(&module_name, last_obj);


    print_module_name(module_name);

    obj_free(&last_obj);
    free(module_name);
}


void typeinfo_all(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_types; ++i) {
        print_type_id(registered_types[i].id);
        if (i != num_registered_types - 1) printf("\n");
    }
}

void funcinfo_all(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_functions; ++i) {
        print_func_id(registered_functions[i].id);
        if (i != num_registered_functions - 1) printf("\n");
    }
}

void moduleinfo_all(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_modules; ++i) {
        print_module_name(registered_modules[i].name);
        if (i != num_registered_modules - 1) printf("\n");
    }
}



void list_types(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_types; ++i) {
        printf("%s", registered_types[i].name);
        if (i != num_registered_types - 1) printf(", ");
    }
    printf("\n");
}

void list_functions(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_functions; ++i) {
        printf("%s", registered_functions[i].name);
        if (i != num_registered_functions - 1) printf(", ");
    }
    printf("\n");
}

void list_modules(runtime_t * runtime) {
    int i;
    for (i = 0; i < num_registered_modules; ++i) {
        printf("%s", registered_modules[i].name);
        if (i != num_registered_modules - 1) printf(", ");
    }
    printf("\n");
}

void set_global_dict(runtime_t * runtime) {
    if (runtime->stack.len <= 1) {
        raise_exception("not enough items on stack (need value, key)", 1);
        return;
    }
    
    obj_t _key, _val;

    _key = estack_pop(&runtime->stack);

    char * _key_str;

    str_obj_force(&_key_str, _key);

    _val = estack_pop(&runtime->stack);

    dict_set(&runtime->globals, _key_str, _val);

    obj_free(&_key);
    free(_key_str);

}

void get_global_dict(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        raise_exception("not enough items on stack (need key)", 1);
        return;
    }
    
    obj_t _key;

    _key = estack_pop(&runtime->stack);

    char * _key_str;

    str_obj_force(&_key_str, _key);

    obj_t val = obj_copy(dict_get(&runtime->globals, _key_str));

    estack_push(&runtime->stack, val);

    obj_free(&_key);
    free(_key_str);
}

void put_null(runtime_t * runtime) {
    estack_push(&runtime->stack, NULL_OBJ);
}

void ezc_exit(runtime_t * runtime) {
    int exitcode = 0;
    if (runtime->stack.len > 0 && type_exists_name("int")) {
        obj_t last_obj = estack_pop(&runtime->stack);
        type_t int_type = TYPE("int");

        if (ISTYPE(last_obj, int_type)) {
            exitcode = OBJ_AS_STRUCT(last_obj, int);
        }
    }

    exit(exitcode);
}


int init (int id, lib_t _lib) {
    init_exported(id, _lib);

    add_function("repr", "pops on a string representation of an object to the stack", repr);
    add_function("repr&", "replaces the stack with the string representation of each object", repr_recursive);
    add_function("print", "prints the last object on the stack", print_obj);
    add_function("print&", "prints all objects on the stack", print_obj_recursive);
    add_function("swap", "swaps the last two objects on the stack", swap);
    add_function("copy", "copies the last item on the stack", copy);
    add_function("dump", "prints out the global dictionary and the stack", dump);
    add_function("eval", "evaluates the last item on the stack as ezc source code", eval_last_item);

    add_function("del", "deletes the last item on the stack, clearing any associated memory", delete_last_item);
    add_function("del&", "deletes all items on the stack", delete_last_item_repeat);
    add_function("concat", "concatenates the last two objects on the stack", concat);
    add_function("concat&", "concatentates all objects on the stack", concat_repeat);
    add_function("join", "in a stack with (... a, b, c), pops off a, b, c and pops on a and b joined with 'c' in the middle", join);
    add_function("join&", "replaces the stack with all objects joined with the last item", join_repeat);


    add_function("import", "imports a module by name", import);
    add_function("import&", "imports each object on the stack as a module name", import_repeat);

    add_function("type", "prints out info about the type specified by the last object on the stack", typeinfo);
    add_function("type&", "prints out info about all types", typeinfo_all);
    add_function("types", "prints out the names of all types", list_types);

    add_function("function", "prints out info about the function that the previous item on the stack names", funcinfo);
    add_function("function&", "prints out info about all functions", funcinfo_all);

    add_function("functions", "prints out the names of all functions", list_functions);

    add_function("module", "prints out info about the module with the previous item as a name", moduleinfo);
    add_function("module&", "prints out info about all imported modules", moduleinfo_all);
    add_function("modules", "prints out the names of all imported modules", list_modules);


    add_function("dictset", "sets the global dictionary from the last values on the stack being (val, key)", set_global_dict);
    add_function("dictget", "gets the global dictionary from the last value on the stack being (key)", get_global_dict);


    add_function("exit", "exits the program, popping off an exitcode if possible", ezc_exit);
    add_function("pushnull", "puts a null object on the stack", put_null);



    return 0;
}



