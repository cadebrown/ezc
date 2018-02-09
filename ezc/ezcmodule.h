
#ifndef __EZCMODULE_H__
#define __EZCMODULE_H__

#ifndef MODULE_NAME
#error when including 'ezcmodule.h', define MODULE_NAME
#endif

#ifndef MODULE_DESCRIPTION
#error when including 'ezcmodule.h', define MODULE_DESCRIPTION
#endif

#include "ezcsymboldefs.h"
#include "ezctypes.h"
#include "ezcmacros.h"
#include "ezclang.h"


#include "estack.h"
//#include "estack.c"

#include "dict.h"
//#include "dict.c"

#include "routines.h"
//#include "routines.c"

#include "log.h"


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dlfcn.h>


module_export_t exported;
lib_t _lib;

int init(int, lib_t);

module_init_t module_init = { init };


// So that these functions work

#define dlsym _lib.dlsym

#define estack_init _lib.estack_init
#define estack_push _lib.estack_push
#define estack_pop _lib.estack_pop
#define estack_get _lib.estack_get
#define estack_set _lib.estack_set
#define estack_swaptop _lib.estack_swaptop

#define dict_init _lib.dict_index
#define dict_set _lib.dict_set
#define dict_get _lib.dict_get
#define dict_index _lib.dict_index

#define obj_copy _lib.obj_copy
#define obj_free _lib.obj_free
#define obj_construct _lib.obj_construct
#define obj_parse _lib.obj_parse
#define obj_representation _lib.obj_representation

#define type_exists_name _lib.type_exists_name
#define type_exists_id _lib.type_exists_id
#define type_from_name _lib.type_from_name
#define type_from_id _lib.type_from_id
#define type_name_from_id _lib.type_name_from_id
#define type_id_from_name _lib.type_id_from_name

#define function_exists_name _lib.function_exists_name
#define function_exists_id _lib.function_exists_id
#define function_from_name _lib.function_from_name
#define function_from_id _lib.function_from_id
#define function_name_from_id _lib.function_name_from_id
#define function_id_from_name _lib.function_id_from_name

#define str_obj_force _lib.str_obj_force
#define runnable_init_str _lib.runnable_init_str

#define load_sharedlib _lib.load_sharedlib
#define import_module _lib.import_module

#define registered_types (*_lib.types)
#define num_registered_types (*_lib.num_types)

#define registered_functions (*_lib.functions)
#define num_registered_functions (*_lib.num_functions)

#define registered_modules (*_lib.modules)
#define num_registered_modules (*_lib.num_modules)

#define run_runnable _lib.run_runnable
#define runnable_free _lib.runnable_free
#define log_log _lib.log_log


#define UNKNOWN_CONVERSION(tname, fname) sprintf(to_raise, "don't know how to convert '%s' into '%s'", fname, tname); raise_exception(to_raise, 1);

#define UNKNOWN_TYPE(tname) sprintf(to_raise, "wrong argument type: '%s'", tname); raise_exception(to_raise, 1);

#define ASSURE_NONEMPTY_STACK if (runtime->stack.len <= 0) { \
        raise_exception("no objects on stack", 1); \
    }


#define ASSURE_STACK_LEN(n) if (runtime->stack.len < n) { \
        raise_exception("not enough objects on the stack", 1); \
    }


// pre allocated exception buffer
char to_raise[16384];

#define has_exception _lib.has_exception
#define raise_exception(reason, code) _lib.raise_exception(reason, code)


#define LIB_FUNC(libobj, rt, fname, ...) ((rt (*)(__VA_ARGS__))dlsym(libobj, fname))


int cur_id;

void init_exported(int id_start, lib_t __lib) {
    cur_id = id_start;
    exported.num_types = 0;
    exported.num_functions = 0;
    exported.types = NULL;
    exported.functions = NULL;
    exported.description = MODULE_DESCRIPTION;
    _lib = __lib;
}


void add_function(char * name, char * desc, raw_function_t * function) {
    exported.num_functions++;
    if (exported.functions == NULL) {
        exported.functions = malloc(sizeof(function_t) * exported.num_functions);
    } else {
        exported.functions = realloc(exported.functions, sizeof(function_t) * exported.num_functions);
    }
    function_t made_function;
    made_function.name = malloc(strlen(name) + 1);
    made_function.id = cur_id++;
    made_function.description = malloc(strlen(desc) + 1);
    strcpy(made_function.name, name);
    strcpy(made_function.description, desc);
    made_function.function = function;
    exported.functions[exported.num_functions - 1] = made_function;
}


void add_type(char * name, char * desc, constructor_t * constructor, copier_t * copier, parser_t * parser, representation_t * representation, destroyer_t * destroyer) {
    exported.num_types++;
    if (exported.types == NULL) {
        exported.types = malloc(sizeof(type_t) * exported.num_types);
    } else {
        exported.types = realloc(exported.types, sizeof(type_t) * exported.num_types);
    }
    type_t made_type;
    made_type.id = cur_id++;
    made_type.name = malloc(strlen(name) + 1);
    strcpy(made_type.name, name);
    made_type.description = malloc(strlen(desc) + 1);
    strcpy(made_type.description, desc);
    made_type.constructor = constructor;
    made_type.copier = copier;
    made_type.parser = parser;
    made_type.representation = representation;
    made_type.destroyer = destroyer;
    exported.types[exported.num_types - 1] = made_type;
}

#endif

