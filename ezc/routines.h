#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include "ezctypes.h"

#define EXCEPTION_LEN 16384

obj_t obj_copy(obj_t);

void obj_free(obj_t *);

void obj_construct(type_t type, obj_t * to, obj_t from);

void obj_parse(type_t type, obj_t * to, char * from);

void obj_representation(type_t type, obj_t * from, char ** to);

void init_runtime(runtime_t * runtime);

void runnable_init_str(runnable_t * runnable, char * src);

bool str_obj_force(char ** out, obj_t in);

#endif
