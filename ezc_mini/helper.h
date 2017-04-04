#ifndef __EZC_HELPER_H__
#define __EZC_HELPER_H__

#include <string.h>
#include "ezc.h"

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

#define DO_ITER(stk, src, i, ct, maxiter, code)                             \
 ct = 0; maxiter = 1;                                                       \
 if (IS_REPEAT(src[i])) {                                                   \
   maxiter = -1; i++;                                                       \
   if (IS_DIGIT(src[i])) {                                                  \
     ret_ll(src, &i, &maxiter);                                             \
   } else if (src[i] == '[') {                                              \
     char *__subgroup = (char *)malloc(1000);                               \
      ret_subgroup(__subgroup, src, &i);                                    \
      exec_code(stk, __subgroup);                                           \
     maxiter = pop_int(stk);                                                \
   }                                                                        \
 }                                                                          \
 while (ct < maxiter || maxiter == -1 && (*stk).ptr >= 0 &&                 \
       RECENT_F(stk, 0) != FLAG_STOP && !globalstop) {                      \
   code                                                                     \
   ct++;                                                                    \
 }                                                                          \

#define NEXT(stk, ptr, num) next_valid(stk, ptr, num)

long long str_startswith(char *str, char *val, long long offset);

long long next_valid(EZC_STACK stk, EZC_IDX val, EZC_INT num);


#endif
