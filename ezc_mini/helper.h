#ifndef __EZC_HELPER_H__
#define __EZC_HELPER_H__

#include <string.h>

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define STR_STARTS(st, va, off) (str_startswith(st, va, off))
#define SS(st, va, off) STR_STARTS(st, va, off)

#define IS_DIGIT(x) ((x) - '0' >= 0 && (x) - '0' <= 9)
#define IS_OP(s, x) ( \
 SS(s, "<", x) || SS(s, ">", x) || SS(s, "<<", x) || SS(s, ">>", x) || \
 SS(s, "+", x) || SS(s, "-", x) ||  SS(s, "*", x) ||  SS(s, "/", x) || \
 SS(s, "%", x) || SS(s, "^", x) ||  SS(s, "!", x) ||  SS(s, "$", x) || \
 SS(s, ":", x)                                                         \
) 

#define IS_ALPHA(x) (x - 'a' >= 0 && x - 'z' <= 0)
#define IS_SPEC(x) (x == '|' || x == '#')
#define IS_SPACE(x) (x == ' ' || x == ',')
#define IS_SIGN(x) (x == '-')

#define SKIP_WHITESPACE(code, itr) while (IS_SPACE(code[itr])) { itr++; }

#define DO_ITER(src, i, ct, maxiter, code)                                  \
 ct = 0; maxiter = 1;                                                       \
 if (EZC_REPEAT(src[i])) {                                                  \
   maxiter = 0; i++; if (IS_DIGIT(src[i])) gen_ret_ll(src, &i, &maxiter);   \
 }                                                                          \
 while (ct < maxiter || maxiter == 0 && ptr > 0) {                          \
   code                                                                     \
   ct++;                                                                    \
 }                                                                          \


long long str_startswith(char *str, char *val, long long offset);



#endif
