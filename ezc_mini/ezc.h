//#define USE_GMP


#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h>

#ifndef MAXSTACKSIZE
 #define MAXSTACKSIZE (1000)
#endif

#ifndef MAXCONSTSIZE
 #define MAXCONSTSIZE (1000)
#endif

#define EZC_INT long long
#define EZC_STR char *

#ifndef USEGMP
 #define EZC_MP mpz_t
#else
 #define EZC_MP err
#endif

#define EZC_FLAG long long
#define EZC_TYPE long long
#define EZC_IDX long long
#define EZC_STACK ezc_stack_t *
#define EZC_DICT ezc_dict_t *

typedef struct ezc_stack_t {
    long long ptr;
    
    void **vals[MAXSTACKSIZE];
    EZC_FLAG flags[MAXSTACKSIZE];
    EZC_TYPE type[MAXSTACKSIZE];
} ezc_stack_t;

typedef struct ezc_dict_t {
    long long ptr;

    void **keys[MAXSTACKSIZE];

    void **vals[MAXSTACKSIZE];
    EZC_FLAG flags[MAXSTACKSIZE];
    EZC_TYPE type[MAXSTACKSIZE];
} ezc_dict_t;


extern EZC_STACK global_stk;
extern EZC_DICT global_dict;

extern long long globalstop;

extern char tmpbuf[10000];


void end(EZC_STACK stk);

void fail(char *reason, EZC_STACK stk, char *code, long long pos);


void exec_code(EZC_STACK stk, char *code);

int main(int argc, char *argv[]);

#endif
