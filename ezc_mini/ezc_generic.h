#ifndef __EZC_GENERIC_H__
#define __EZC_GENERIC_H__

#include "ezc.h"

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
void gen_ret_function(char *out, char *code, long long *start);
void gen_ret_special(char *out, char *val, long long *start);
void gen_ret_operator(char *out, char *val, long long *start);
void gen_ret_subgroup(char *val, long long *idx, long long *start, long long *len);

void gen_operator(char *op);
void gen_special(char *op);

void gen_control(char *control, char val[]);

void get_const_str(char *out, char *code, long long *start);

EZC_INT gen_pop_int(void);

void gen_push_int(EZC_INT val);
void gen_push_dupe(void);
void gen_push_str(char *val);

/*
  These should be implemented per C file, for example, GMP v LL
*/


void __int_function(char *func);

void __int_op(char *op);


void __int_push(EZC_INT ret);
void __int_reset(EZC_IDX ret);

void __int_from_str(EZC_IDX ret, char *val);
void __int_to_str(char *ret, EZC_IDX val);

void __int_print(EZC_IDX pos);

void __int_gt(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_lt(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);

void __int_add(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_sub(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_mul(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_div(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_mod(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);
void __int_pow(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1);

void __int_sqrt(EZC_IDX ret, EZC_IDX p0);


#endif
