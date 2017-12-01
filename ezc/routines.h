#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include "ezctypes.h"

#define EXCEPTION_LEN 16384

obj_t obj_copy(obj_t);

void obj_free(obj_t *);

void obj_construct(type_t type, obj_t * to, obj_t from);

void init_runtime(runtime_t * runtime);

void runnable_init_str(runnable_t * runnable, char * src);

bool str_obj_force(char ** out, obj_t in);

void run_runnable(runtime_t * runtime, runnable_t * runnable);

void run_str(runtime_t * runtime, char * ezc_source_code, runnable_t * runnable, int linenum);

#endif
