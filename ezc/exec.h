#ifndef __EXEC_H__
#define __EXEC_H__


#include "ezctypes.h"


void set_curline(int);

void set_curchar(int);

void raise_exception(char *, int);

void run_runnable(runtime_t * runtime, runnable_t * runnable);

void run_str(runtime_t * runtime, char * ezc_source_code);


#endif
