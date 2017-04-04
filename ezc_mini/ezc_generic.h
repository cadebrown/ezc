#ifndef __EZC_GENERIC_H__
#define __EZC_GENERIC_H__

#include "ezc.h"
#include "helper.h"
#include "ezc_generic.h"

#define TYPE_DEF                  (0x0000)
#define TYPE_EXT                  (0x0300)

#define TYPE_NIL                  (0x0000 | TYPE_DEF)
#define TYPE_INT                  (0x0001 | TYPE_DEF)
#define TYPE_STR                  (0x0002 | TYPE_DEF)
#define TYPE_MPI                  (0x0003 | TYPE_EXT)


#define FLAG_DEFAULT              (0x0000)
#define FLAG_SPECIAL              (0x0100)

#define FLAG_STOP                 (0x0001 | FLAG_SPECIAL)
#define FLAG_POINT                (0x0002 | FLAG_SPECIAL)


#define IS_TYPE(v, f)             (v == f)

#define MEETS_FLAG(v, f)          (v == f)
#define MEETS_GENFLAG(v, f)       ((v & f) == f)

#define GET(stk, type, n) ((type *)(&(*stk).vals))[n]
#define GET_F(stk, n) (*stk).flags[n]
#define GET_T(stk, n) (*stk).type[n]

#define RECENT(stk, type, n) GET(stk, type, (*stk).ptr-n)
#define RECENT_F(stk, n) GET_F(stk, (*stk).ptr-n)
#define RECENT_T(stk, n) GET_T(stk, (*stk).ptr-n)

#define move_ahead(stk, n) (*stk).ptr+=n 

/*
   These are handled in ezc_generic.c
*/

void setup();
void print_single(EZC_STACK stk, EZC_IDX pos);
void dump(EZC_STACK stk);



void push_copy(EZC_STACK stk, EZC_IDX pos);

void push_int(EZC_STACK stk, EZC_INT val);
void push_str(EZC_STACK stk, char *val);

EZC_INT pop_int(EZC_STACK stk);


void swap(EZC_STACK stk, EZC_IDX pos0, EZC_IDX pos1);

void ret_const(char *out, char *code, long long *start);
void ret_subgroup(char *out, char *val, EZC_IDX *idx);
void ret_ll(char *val, long long *idx, long long *out);

void ret_function(char *out, char *code, long long *start);
void ret_special(char *out, char *val, long long *start);
void ret_operator(char *out, char *val, long long *start);

void run_function(EZC_STACK stk, char *func);
void run_operator(EZC_STACK stk, char *op);
void run_special(EZC_STACK stk, char *op);

/*
  These should be implemented per C file, for example, GMP v LL
*/



void __int_push(EZC_STACK stk, EZC_INT ret);
void __int_reset(EZC_STACK stk, EZC_IDX ret);


void __int_function(EZC_STACK stk, char *func);
void __int_op(EZC_STACK stk, char *op);


void __int_from_str(EZC_STACK stk, EZC_IDX ret, char *val);
void __int_to_str(EZC_STACK stk, char *ret, EZC_IDX val);


// impl functions expected by compiler
void __int_print(EZC_STACK stk, EZC_IDX pos);

void __int_gt(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_lt(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_et(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);

void __int_add(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_sub(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_mul(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_div(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_mod(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_pow(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);

void __int_sqrt(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0);


#endif
