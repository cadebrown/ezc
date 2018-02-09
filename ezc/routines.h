#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include "ezctypes.h"

#define EXCEPTION_LEN 16384

obj_t obj_copy(obj_t);

void obj_free(obj_t *);

obj_t obj_construct(type_t type, obj_t from);

obj_t obj_parse(type_t type, char * from);

char * obj_representation(type_t type, obj_t * from);

void dump_runtime(runtime_t * runtime);

bool valid_function_name(char * fn);

bool valid_module_name(char * fn);

bool valid_type_name(char * fn);


void init_runtime(runtime_t * runtime);

void runnable_init_str(runnable_t * runnable, char * src);

void runnable_add_str(runnable_t * runnable, char * _src);

void str_obj_force(char ** out, obj_t in);

void runnable_free(runnable_t * runnable);

#endif
