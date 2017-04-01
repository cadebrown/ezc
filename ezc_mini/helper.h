#ifndef __EZC_HELPER_H__
#define __EZC_HELPER_H__

#include <string.h>

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define STR_STARTS(st, va, off) (str_startswith(st, va, off))
#define SS(st, va, off) STR_STARTS(st, va, off)

#define IS_REPEAT(x) (x == '&')
#define IS_DIGIT(x) ((x) - '0' >= 0 && (x) - '0' <= 9)
#define IS_OP(s, x) ( \
 SS(s, "<", x) || SS(s, ">", x) || SS(s, "<<", x) || SS(s, ">>", x) || \
 SS(s, "+", x) || SS(s, "-", x) ||  SS(s, "*", x) ||  SS(s, "/", x) || \
 SS(s, "%", x) || SS(s, "^", x) ||  SS(s, "!", x) ||  SS(s, "$", x) || \
 SS(s, ":", x)                                                         \
) 

#define IS_ALPHA(x) ((x - 'a' >= 0 && x - 'z' <= 0) || (x - 'A' >= 0 && x - 'Z' <= 0))
#define IS_SPEC(x) (x == '|' || x == '#')
#define IS_SPACE(x) (x == ' ' || x == ',')
#define IS_SIGN(x) (x == '-')

#define SKIP_WHITESPACE(code, itr) while (IS_SPACE(code[itr])) { itr++; }

#define DO_ITER(src, i, ct, maxiter, code)                                  \
 ct = 0; maxiter = 1;                                                       \
 if (IS_REPEAT(src[i])) {                                                   \
   maxiter = 0; i++;                                                        \
   if (IS_DIGIT(src[i])) {                                                  \
     gen_ret_ll(src, &i, &maxiter);                                         \
   } else if (src[i] == '[') {                                              \
     EZC_INT _s = 0, _l = 0;                                                \
     gen_ret_subgroup(src, &i, &_s, &_l);                                   \
     exec_code(src, _s, _l);                                                \
     maxiter = gen_pop_int();                                               \
   }                                                                        \
 }                                                                          \
 while (ct < maxiter || maxiter == -1 && ptr > 0) {                         \
   code                                                                     \
   ct++;                                                                    \
 }                                                                          \

#define NEXT(ptr, num) next_valid(ptr, num)

long long str_startswith(char *str, char *val, long long offset);

long long next_valid(long long val, long long num);


#endif
