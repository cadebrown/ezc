
#ifndef __EZC_H__
#define __EZC_H__


#define DEFAULT_ARRAY_LEN 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define STR_STARTS(st, va, off) (str_startswith(st, va, off))
#define SS(st, va, off) STR_STARTS(st, va, off)

#define IS_REPEAT(x) (x == '&')
#define IS_DIGIT(x) ((x) - '0' >= 0 && (x) - '0' <= 9)
#define IS_OP(s, x) ( \
 SS(s, "<", x) || SS(s, ">", x) || SS(s, "<<", x) || SS(s, ">>", x) ||      \
 SS(s, "+", x) || SS(s, "-", x) ||  SS(s, "*", x) ||  SS(s, "/", x) ||      \
 SS(s, "%", x) || SS(s, "^", x) ||  SS(s, "!", x) ||  SS(s, "$", x) ||      \
 SS(s, ":", x) || SS(s, "==", x) || SS(s, "_", x)                           \
) 

#define IS_ALPHA(x) ((x - 'a' >= 0 && x - 'z' <= 0) || (x - 'A' >= 0 && x - 'Z' <= 0))
#define IS_SPEC(x) (x == '|' || x == '#')
#define IS_SPACE(x) (x == ' ' || x == ',')
#define IS_SIGN(x) (x == '-')

#define SKIP_WHITESPACE(code, itr) while (IS_SPACE(code[itr])) { itr++; }

long long str_startswith(char *str, char *val, long long offset);


/* objects */
#define EZC_OBJ ezc_obj_t *


typedef struct ezc_obj_t {
	EZC_TYPE type;

	void *val;
} ezc_obj_t;


/* stacks */
#define EZC_STACK ezc_stk_t *

typedef struct ezc_stk_t {
	EZC_IDX ptr;
	EZC_IDX size;

	EZC_OBJ *vals;
} ezc_stk_t;


void stk_init(EZC_STACK stk, EZC_INT len);
void stk_resize(EZC_STACK stk, EZC_INT len);

EZC_OBJ stk_pop(EZC_STACK stk);
EZC_OBJ stk_get(EZC_STACK stk, EZC_INT pos);

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


void dict_init(EZC_DICT stk, EZC_INT len);
void dict_resize(EZC_DICT stk, EZC_INT len);

void dict_set(EZC_DICT dict, EZC_STR key, EZC_OBJ val);

EZC_OBJ dict_get(EZC_DICT dict, EZC_STR key);



void exec(EZC_STR code, EZC_DICT dict, EZC_STACK stk);

int main(int argc, char *argv[]);

#endif
