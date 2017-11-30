#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include "ezctypes.h"

obj_t obj_copy(obj_t);

void obj_free(obj_t *);

void init_runtime(runtime_t * runtime);

void run_code(runtime_t * runtime, char * ezc_source_code);

#endif
