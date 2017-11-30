
#ifndef __EZCMODULE_H__
#define __EZCMODULE_H__

#ifndef MODULE_NAME
#error when including 'ezcmodule.h', define MODULE_NAME
#endif

#include "ezcsymboldefs.h"

#include "ezctypes.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "estack.h"
#include "estack.c"

#include "routines.h"
#include "routines.c"


#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y)  PASTER(x,y)

#define STRINGER(x) #x

#define MODULE_FUNCTION(name) EVALUATOR(MODULE_NAME, name)

module_export_t exported;
module_utils_t utils;

int cur_id;

void init_exported(int id_start, module_utils_t _utils) {
    cur_id = id_start;
    exported.num_types = 0;
    exported.num_functions = 0;
    exported.types = NULL;
    exported.functions = NULL;
    utils = _utils;
}


void add_function(char * name, raw_function_t * function) {
    exported.num_functions++;
    if (exported.functions == NULL) {
        exported.functions = malloc(sizeof(function_t) * exported.num_functions);
    } else {
        exported.functions = realloc(exported.functions, sizeof(function_t) * exported.num_functions);
    }
    function_t made_function;
    made_function.name = malloc(strlen(name));
    made_function.id = cur_id++;
    strcpy(made_function.name, name);
    made_function.function = function;
    exported.functions[exported.num_functions - 1] = made_function;
}


void add_type(char * name, constructor_t * constructor, parser_t * parser, representation_t * representation, destroyer_t * destroyer) {
    exported.num_types++;
    if (exported.types == NULL) {
        exported.types = malloc(sizeof(type_t) * exported.num_types);
    } else {
        exported.types = realloc(exported.types, sizeof(type_t) * exported.num_types);
    }
    type_t made_type;
    made_type.id = cur_id++;
    made_type.name = malloc(strlen(name));
    strcpy(made_type.name, name);
    made_type.constructor = constructor;
    made_type.parser = parser;
    made_type.representation = representation;
    made_type.destroyer = destroyer;
    exported.types[exported.num_types - 1] = made_type;
}

#endif

