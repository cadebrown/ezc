//#define USE_GMP


#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h>

#include "helper.h"


#ifndef MAXSTACKSIZE
 #define MAXSTACKSIZE (100)
#endif


#define EZC_INT long long

#define EZC_STACK long long
#define EZC_FLAG long long
#define EZC_TYPE long long
#define EZC_IDX long long


typedef struct stack_t {
    void **vals[MAXSTACKSIZE];
    EZC_FLAG flags[MAXSTACKSIZE];
    EZC_TYPE type[MAXSTACKSIZE];
} stack_t;

stack_t stk;


long long inptr, ptr;

char *input, *buf;

void end(void);

void fail(char *reason, char *code, long long pos, long long subpos);


void exec_code(char *code, long long start, long long len);

int main(int argc, char *argv[]);

#endif
