//#define USE_GMP

#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h>

#include "ezc_generic.h"
#include "helper.h"

#define EZC_REPEAT(x) (x == '&')

typedef struct stack_t {
    void **vals[MAXSTACKSIZE];
    EZC_FLAG flags[MAXSTACKSIZE];
    EZC_TYPE type[MAXSTACKSIZE];
} stack_t;

stack_t stk;


long long inptr, ptr;

char *input, *buf;

void end(void);

void fail(char *code, char *reason, long long pos);


void exec_code(char *code, long long start, long long len);

int main(int argc, char *argv[]);

#endif
