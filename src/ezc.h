
#ifndef __EZC_H__
#define __EZC_H__


#define DEFAULT_ARRAY_LEN    (20)
#define ARG_PRINT_OFF        (25)
#define MAX_LINE_LENGTH      (10000)
#define MAX_OP_LENGTH        (5)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"

#define EZC_FAIL(reason) fprintf(stderr, reason); exit(2);

#ifndef USEGMP
 #define EZC_MP mpz_t
#else
 #define EZC_MP err
#endif

#define EZC_INT long long
#define EZC_STR char *

#define EZC_FLAG long long
#define EZC_TYPE long long
#define EZC_IDX long long


/* Helpful macros */

#define CONT_ALIAS(dict, a0, a1) (dict_contains_key(dict, a0) || dict_contains_key(dict, a1))

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define STR_STARTS(st, va, off) (str_startswith(st, va, off))
#define SS(st, va, off) STR_STARTS(st, va, off)

#define IS_REPEAT(x) (x == '&')
#define IS_DIGIT(x) ((x) - '0' >= 0 && (x) - '0' <= 9)
#define IS_OP(s, x) ( \
 SS(s, "<", x) || SS(s, ">", x) ||  SS(s, "==", x) || SS(s, "_", x) ||      \
 SS(s, "+", x) || SS(s, "-", x) ||  SS(s, "*", x) ||  SS(s, "/", x) ||      \
 SS(s, "%", x) || SS(s, "^", x) ||  SS(s, "!", x) ||  SS(s, "$", x) ||      \
 SS(s, ":", x)                                                              \
) 

#define IS_1OP(s, x) ( \
 SS(s, "_", x) || SS(s, "$", x) || SS(s, "!", x)                            \
) 

#define IS_2OP(s, x) ( \
 SS(s, "<", x) || SS(s, ">", x) || SS(s, "+", x) || SS(s, "-", x) ||        \
 SS(s, "*", x) || SS(s, "/", x) || SS(s, "%", x) || SS(s, "^", x) ||        \
 SS(s, "==", x)                                                             \
) 

#define IS_ALPHA(x) ((x - 'a' >= 0 && x - 'z' <= 0) || (x - 'A' >= 0 && x - 'Z' <= 0))
#define IS_SPEC(x) (x == '|' || x == '#')
#define IS_SPACE(x) (x == ' ' || x == ',' || x == '\n')
#define IS_SIGN(x) (x == '-')

#define SKIP_WHITESPACE(code, itr) while (IS_SPACE(code[itr])) { itr++; }

long long str_startswith(char *str, char *val, long long offset);
void ret_operator(char *out, char *val, long long *start);



/* objects */
#define EZC_OBJ ezc_obj_t *
#define MALLOC_OBJ(name) name = (EZC_OBJ)malloc(sizeof(ezc_obj_t) * 1);
#define CREATE_OBJ(name) EZC_OBJ name; MALLOC_OBJ(name);
#define SET_OBJ(tmp, typ, vl) (*tmp).val = (void *)vl; (*tmp).type = typ;

#define TYPE_NIL   (0x0000)
#define TYPE_BOOL  (0x0001)
#define TYPE_INT   (0x0002)
#define TYPE_FLT   (0x0003)
#define TYPE_STR   (0x0004)

typedef struct ezc_obj_t {
	EZC_TYPE type;

	void *val;
} ezc_obj_t;

void obj_cpy(EZC_OBJ a, EZC_OBJ b);
EZC_OBJ obj_from_str(EZC_STR s);
EZC_STR str_from_obj(EZC_OBJ v);


bool obj_eq(EZC_OBJ a, EZC_OBJ b);
EZC_INT obj_cmp(EZC_OBJ a, EZC_OBJ b);

void obj_dump(EZC_OBJ obj);
void obj_dump_fmt(EZC_OBJ obj, bool raw);

/* obj constants */

EZC_OBJ EZC_NIL;



/* stacks */
#define EZC_STACK ezc_stk_t *

typedef struct ezc_stk_t {
	EZC_IDX ptr;
	EZC_IDX size;

	EZC_OBJ *vals;
} ezc_stk_t;


void stk_dump(EZC_STACK stk);

void stk_init(EZC_STACK stk, EZC_INT len);
void stk_resize(EZC_STACK stk, EZC_INT len);

EZC_OBJ stk_pop(EZC_STACK stk);
EZC_OBJ stk_get(EZC_STACK stk, EZC_INT pos);
EZC_OBJ stk_get_recent(EZC_STACK stk, EZC_INT rpos);

void stk_push(EZC_STACK stk, EZC_OBJ val);
void stk_set(EZC_STACK stk, EZC_INT pos, EZC_OBJ val);



/* dictionaries */

#define EZC_DICT ezc_dict_t *

typedef struct ezc_dict_t {
	EZC_IDX ptr;
	EZC_IDX size;

	char **keys;
	EZC_OBJ *vals;
} ezc_dict_t;


void dict_dump(EZC_DICT dict);
void dict_dump_fmt(EZC_DICT dict, bool raw, long long offset);


void dict_init(EZC_DICT dict, EZC_INT len);
void dict_resize(EZC_DICT dict, EZC_INT len);

bool dict_contains_key(EZC_DICT dict, EZC_STR key);
EZC_IDX dict_key_idx(EZC_DICT dict, EZC_STR key);

bool dict_contains_val(EZC_DICT dict, EZC_OBJ val);
EZC_IDX dict_val_idx(EZC_DICT dict, EZC_OBJ val);

void dict_set(EZC_DICT dict, EZC_STR key, EZC_OBJ val);
EZC_OBJ dict_get(EZC_DICT dict, EZC_STR key);

/* evaluating code */

void eval_func(EZC_DICT dict, EZC_STACK stk, EZC_STR func);
void eval_func__int(EZC_DICT dict, EZC_STACK stk, EZC_STR op);


void eval_op(EZC_DICT dict, EZC_STACK stk, EZC_STR op);
void eval_op__int(EZC_DICT dict, EZC_STACK stk, EZC_STR op);
void eval_op__str(EZC_DICT dict, EZC_STACK stk, EZC_STR op);

EZC_INT __int_pow(EZC_INT a, EZC_INT b);


/* argument functions */


void init_args(EZC_DICT dict);
void get_args(EZC_DICT dict, EZC_STR *args, EZC_IDX start, EZC_IDX end);

void help_message();


/* main program functions */

char *EXEC_TITLE;

void fail(EZC_STR reason, EZC_STR code, EZC_DICT dict, EZC_STACK stk, long long pos);

void exec(EZC_STR code, EZC_DICT dict, EZC_STACK stk);

void interperet(EZC_DICT dict, EZC_STACK stk);

int main(int argc, char *argv[]);

#endif