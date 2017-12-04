#ifndef __EXEC_H__
#define __EXEC_H__


#include "ezctypes.h"


void set_curline(int);

void set_curchar(int);

void raise_exception(char *, int);

void run_runnable(runtime_t * runtime, runnable_t * runnable);

type_t implicit_type(char * entered);

void run_str(runtime_t * runtime, char * ezc_source_code);

void run_interactive_fallback(runtime_t * runtime);

#ifdef HAVE_READLINE

char ** cmd_completion(const char *, int, int);
char * cmd_generator(const char *, int);

#endif

void run_interactive(runtime_t * runtime);


#endif
