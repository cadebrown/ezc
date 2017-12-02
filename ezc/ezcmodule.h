
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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

module_export_t exported;
module_utils_t utils;

int init(int, module_utils_t);

module_init_t module_init = { init };


// So that these functions work
#define type_exists_name utils.type_exists_name
#define type_exists_id utils.type_exists_id
#define type_from_name utils.type_from_name
#define type_from_id utils.type_from_id
#define type_name_from_id utils.type_name_from_id
#define type_id_from_name utils.type_id_from_name

#define function_exists_name utils.function_exists_name
#define function_exists_id utils.function_exists_id
#define function_from_name utils.function_from_name
#define function_from_id utils.function_from_id
#define function_name_from_id utils.function_name_from_id
#define function_id_from_name utils.function_id_from_name

#define import_module utils.import_module

#define registered_types (*utils.types)
#define num_registered_types (*utils.num_types)

#define registered_functions (*utils.functions)
#define num_registered_functions (*utils.num_functions)

#define registered_modules (*utils.modules)
#define num_registered_modules (*utils.num_modules)

#define run_runnable utils.run_runnable


#define UNKNOWN_CONVERSION(tname, fname) sprintf(to_raise, "don't know how to convert '%s' into '%s'", fname, tname); raise_exception(to_raise, 1); return;

#define UNKNOWN_TYPE(tname) sprintf(to_raise, "wrong argument type: '%s'", tname); raise_exception(to_raise, 1); return;

#define ASSURE_NONEMPTY_STACK if (runtime->stack.len <= 0) { \
        raise_exception("no objects on stack", 1); \
    }



// pre allocated exception buffer
char to_raise[16384];

#define has_exception utils.has_exception
#define raise_exception(reason, code) utils.raise_exception(reason, code)

#include "estack.h"
#include "estack.c"

#include "dict.h"
#include "dict.c"

#include "routines.h"
#include "routines.c"


int cur_id;

void init_exported(int id_start, module_utils_t _utils) {
    cur_id = id_start;
    exported.num_types = 0;
    exported.num_functions = 0;
    exported.types = NULL;
    exported.functions = NULL;
    exported.description = MODULE_DESCRIPTION;
    utils = _utils;
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

