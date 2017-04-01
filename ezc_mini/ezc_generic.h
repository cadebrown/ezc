#ifndef __EZC_GENERIC_H__
#define __EZC_GENERIC_H__

#include "ezc.h"

#define EZC_CONST_FLAGS           (0x0001)
#define EZC_STR_FLAGS             (0x0002)

#define EZC_SPECIAL_FLAGS         (0x0100)
#define EZC_SPECIAL_STOP_FLAGS    (0x0001 | EZC_SPECIAL_FLAGS)
#define EZC_SPECIAL_POINT_FLAGS   (0x0002 | EZC_SPECIAL_FLAGS)

#define MEETS_FLAG(v, f)          (v == f)
#define MEETS_GENFLAG(v, f)       ((v & f) == f)

#ifndef MAXSTACKSIZE
 #define MAXSTACKSIZE (100)
#endif


#define EZC_STACK_TYPE long long
#define EZC_FLAG_TYPE long long


#define GET(type, n) ((type *)(&stk.vals))[n]
#define GET_F(n) stk.flags[n]
#define GET_T(n) stk.type[n]

#define RECENT(type, n) GET(type, ptr-n)
#define RECENT_F(n) GET_F(ptr-n)
#define RECENT_T(n) GET_T(ptr-n)


/*
   These are handled in ezc_generic.c
*/

void move_ahead(int num);

void gen_setup(void);
void gen_dump(void);
void gen_end(void);

void gen_ret_ll(char *val, long long *idx, long long *out);

void gen_ret_special(char *out, char *val, long long *start);
void gen_ret_operator(char *out, char *val, long long *start);
void gen_ret_subgroup(char *val, long long *idx, long long *start, long long *len);

void gen_operator(char *op);
void gen_special(char *op);

void gen_control(char *control, char val[]);

void get_const_str(char *out, char *code, long long *start);
void push_dupe(void);
void push_str(char *val);

/*
  These should be implemented per C file, for example, GMP v LL
*/

void reset_val(EZC_STACK_TYPE *ret);

void from_str(EZC_STACK_TYPE *ret, EZC_FLAG_TYPE *flags, char *val);
void to_str(char *ret, EZC_STACK_TYPE val, EZC_FLAG_TYPE flags);

void print_single(FILE *stream, EZC_STACK_TYPE val, EZC_FLAG_TYPE flags);

// functions

void __gt(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __lt(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);

void __swap(EZC_STACK_TYPE *op0, EZC_STACK_TYPE *op1);

void __add(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __sub(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __mul(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __div(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __mod(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);
void __pow(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1);


#endif
