/* ezc_mini.c -- main file for EZC mini, a self contained RPN calculator
              -- with (optionally) support for GMP and MPFR

  Copyright 2016-2017 ChemicalDevelopment

  This file is part of the EZC project.

  EZC, EC, EZC Documentation, and any other resources in this project are 
free software; you are free to redistribute it and/or modify them under 
the terms of the GNU General Public License; either version 3 of the 
license, or any later version.

  These programs are hopefully useful and reliable, but it is understood 
that these are provided WITHOUT ANY WARRANTY, or MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GPLv3 or email at 
<info@chemicaldevelopment.us> for more info on this.

  Here is a copy of the GPL v3, which this software is licensed under. You 
can also find a copy at http://www.gnu.org/licenses/.

*/


#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>

#include <math.h>

#include "config.h"

void dump();
void fail(char reason[]);
void eval(char code[]);
void print_idx(int x);
void interperet();

#define E_DOUBLE (0x01)
#define E_MPF    (0x02)
#define E_MPFR   (0x03)

int type = E_DOUBLE;
bool do_interperet = false;

#define STRSIZE(bits) (long long)((bits) / (3.321))
#define BITSIZE(len)  (long long)((len) * 3.321)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifdef USE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

#ifdef USE_GMP
    #ifdef USE_MPFR
        #define EZC_RND MPFR_RNDN
        #define DOIF(ifdouble, ifmpf, ifmpfr) \
        if (type == E_DOUBLE) { \
            ifdouble; \
        } else if (type == E_MPF) { \
            ifmpf; \
        } else if (type == E_MPFR) { \
            ifmpfr; \
        } else { \
            fail("Don't know which type in DOIF"); \
        }
    #else
        #define DOIF(ifdouble, ifmpf, ifmpfr) \
        if (type == E_DOUBLE) { \
            ifdouble; \
        } else if (type == E_MPF) { \
            ifmpf; \
        } else if (type == E_MPFR) { \
            fail("Tried running MPFR code, but not compiled with MPFR (was compiled with GMP"); \
        } else { \
            fail("Don't know which type in DOIF"); \
        }
    #endif    
#else
    #define DOIF(ifdouble, ifmpf, ifmpfr) \
    if (type == E_DOUBLE) { \
        ifdouble; \
    } else if (type == E_MPF) { \
        fail("Tried running MPF code, but not compiled with GMP"); \
    } else if (type == E_MPFR) { \
        fail("Tried running MPFR code, but not compiled with MPFR or GMP"); \
    } else { \
        fail("Don't know which type in DOIF"); \
    }
#endif

#ifdef USE_GMP
    #include <gmp.h>

    #ifdef USE_MPFR
        #include <mpfr.h>
    #endif
#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define STACK_NUM (1000)
#define PSTR_LEN (1000)

#define IS_SPACE(ch) (ch == ' ' || ch == ',')
#define IS_CONST(ch) ((ch - '0' >= 0 && ch - '9' <= 0))
#define IS_ALPHA(ch) ((ch - 'a' >= 0 && ch - 'z' <= 0) || (ch - 'A' >= 0 && ch - 'Z' <= 0))

#define STR_EQ(a, b) (strcmp(a, b) == 0)

#define STK(t, n) ((t *)(*stack))[n]
#define RECENT(t, n) STK(t, stack_ptr - n)
#define INC(n) inc(n)


int stack_ptr = -1;
int max_stack_ptr = -1;
void **stack;

#ifdef USE_GMP
  #ifdef USE_MPFR
    mpfr_t tmp_mpfr;
  #endif
  mpf_t tmp_mpf;
#endif

double tmp_double;

int min_prec = 64;

char PSTR[PSTR_LEN];
char *ccode;
int cptr;

#ifdef USE_GMP
    #ifdef USE_MPFR
        // uses mpfr_const_pi to calculate it using smarter methods
        void __ezc_mpf_pi(mpf_t x) {
            mpfr_set_prec(tmp_mpfr, mpf_get_prec(x));
            mpfr_const_pi(tmp_mpfr, EZC_RND);
            mpfr_get_f(x, tmp_mpfr, EZC_RND);
        }

        void __ezc_mpf_mod(mpf_t r, mpf_t x, mpf_t y) {
            mpfr_set_prec(tmp_mpfr, mpf_get_prec(r));
            mpfr_t xfr, yfr;
            mpfr_init2(xfr, mpf_get_prec(x));
            mpfr_init2(yfr, mpf_get_prec(y));
            mpfr_set_f(xfr, x, EZC_RND);
            mpfr_set_f(yfr, y, EZC_RND);
            mpfr_fmod(tmp_mpfr, xfr, yfr, EZC_RND);
            mpfr_get_f(r, tmp_mpfr, EZC_RND);
            mpfr_clear(xfr);
            mpfr_clear(yfr);
        }
    #else
        // if compiled without mpfr support, this 'works', although is not very fast (fast enough though)
        void __ezc_mpf_pi(mpf_t x) {
            mpf_t result, con, A, B, F, sum;
            mpz_t a, b, c, d, e;

            unsigned long int k, threek;
            unsigned long iterations = (mpf_get_prec(x)*0.02122)+1;
            unsigned long precision_bits;

            // roughly compute how many bits of precision we need for
            // this many digit:
            precision_bits = (mpf_get_prec(x)) + 1;

            mpf_set_default_prec(precision_bits);

            // allocate GMP variables
            mpf_inits(result, con, A, B, F, sum, NULL);
            mpz_inits(a, b, c, d, e, NULL);

            mpf_set_ui(sum, 0); // sum already zero at this point, so just FYI

            // first the constant sqrt part
            mpf_sqrt_ui(con, 10005);
            mpf_mul_ui(con, con, 426880);

            // now the fun bit
            for (k = 0; k < iterations; k++) {
                threek = 3*k;

                mpz_fac_ui(a, 6*k);  // (6k)!

                mpz_set_ui(b, 545140134); // 13591409 + 545140134k
                mpz_mul_ui(b, b, k);
                mpz_add_ui(b, b, 13591409);

                mpz_fac_ui(c, threek);  // (3k)!

                mpz_fac_ui(d, k);  // (k!)^3
                mpz_pow_ui(d, d, 3);

                mpz_ui_pow_ui(e, 640320, threek); // -640320^(3k)
                if ((threek&1) == 1) { mpz_neg(e, e); }

                // numerator (in A)
                mpz_mul(a, a, b);
                mpf_set_z(A, a);

                // denominator (in B)
                mpz_mul(c, c, d);
                mpz_mul(c, c, e);
                mpf_set_z(B, c);

                // result
                mpf_div(F, A, B);

                // add on to sum
                mpf_add(sum, sum, F);
            }

            // final calculations (solve for pi)
            mpf_ui_div(sum, 1, sum); // invert result
            mpf_mul(x, sum, con); // multiply by constant sqrt part

            // free GMP variables
            mpf_clears(result, con, A, B, F, sum, NULL);
            mpz_clears(a, b, c, d, e, NULL);
        }

        // a % b = a - (b*(a//b))
        void __ezc_mpf_mod(mpf_t r, mpf_t x, mpf_t y) {
            mpf_set_prec(tmp_mpf, mpf_get_prec(r));
            mpf_div(tmp_mpf, x, y);
            mpf_trunc(tmp_mpf, tmp_mpf);
            mpf_mul(tmp_mpf, tmp_mpf, y);
            mpf_sub(r, x, tmp_mpf);
        }
    #endif

#endif

void fail(char reason[]) {
    printf(KRED "fail: %s" KMAG, reason);

    printf("\n" KBLU "At:" KNRM "\n");
    printf(KCYN "%s\n" KNRM, ccode);
    int i = 0;
    while (i < cptr) {
        printf(" ");
        i++;
    }
    printf(KYEL "^" KNRM "\n");

    printf("Final stack:\n");
    dump();

    printf(KRED "PROGRAM FAILED\n" KNRM);
    exit (2);
    return;
}

void inc(int n) {
    if (n > 0) {
        int i = stack_ptr + 1;
        stack_ptr += n;
        while (i <= stack_ptr) {
            if (i > max_stack_ptr) {
                DOIF(
                    STK(double, i) = 0; ,
                    mpf_init2(STK(mpf_t, i), min_prec); ,
                    mpfr_init2(STK(mpfr_t, i), min_prec);
                );
                max_stack_ptr = i;
            }
            i++;
        }
    } else if (n < 0) {
        stack_ptr += n;
    }

}

void print_idx(int x) {
    DOIF(
        printf(KCYN "%.17g" KNRM, STK(double, x));  ,
        gmp_printf(KCYN "%.*Ff" KNRM, STRSIZE(MAX(min_prec, mpf_get_prec(STK(mpf_t, x)))), STK(mpf_t, x)); ,
        mpfr_printf(KCYN "%.*Rf" KNRM, STRSIZE(MAX(min_prec, mpfr_get_prec(STK(mpfr_t, x)))), STK(mpfr_t, x));
    );
}

void dump() {
    long long x = 0;
    while (x <= stack_ptr) {
        print_idx(x);
        if (x != stack_ptr) {
            printf(KBLU ", " KNRM);
        }
        x++;
    }
    printf("\n");
}

void eval(char *code) {
    ccode = code;
    cptr = 0;
    int i = 0, j;
    while (i < strlen(code)) {
        while (IS_SPACE(code[i])) {
            i++;
        }
        if (i >= strlen(code)) {
            return; 
        }
        cptr = i;
        
        if (IS_CONST(code[i]) || (code[i] == '-' && IS_CONST(code[i+1])) || (code[i] == '.' && IS_CONST(code[i+1]))) {
            j = 0;
            if (code[i] == '-') {
                PSTR[j++] = code[i++];
            }
            while (IS_CONST(code[i]) || code[i] == '.') {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
            INC(1);
            DOIF(
                RECENT(double, 0) = strtod(PSTR, NULL)                 ,

                mpf_set_prec(RECENT(mpf_t, 0), MAX(min_prec, BITSIZE(strlen(PSTR))));
                mpf_set_str(RECENT(mpf_t, 0), PSTR, 10);               ,

                mpfr_set_prec(RECENT(mpfr_t, 0), MAX(min_prec, BITSIZE(strlen(PSTR))));
                mpfr_set_str(RECENT(mpfr_t, 0), PSTR, 10, EZC_RND)
            );
        } else if (code[i] == '+') {
            DOIF(
                RECENT(double, 1) = RECENT(double, 1) + RECENT(double, 0),
                mpf_add(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_add(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '-') {
            DOIF(
                RECENT(double, 1) = RECENT(double, 1) - RECENT(double, 0),
                mpf_sub(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_sub(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '*') {
            DOIF(
                RECENT(double, 1) = RECENT(double, 1) * RECENT(double, 0),
                mpf_mul(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_mul(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '/') {
            DOIF(
                RECENT(double, 1) = RECENT(double, 1) / RECENT(double, 0),
                mpf_div(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_div(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '^') {
            DOIF(
                RECENT(double, 1) =  pow(RECENT(double, 1), RECENT(double, 0)),
                fail("MPF does not support ^"),
                mpfr_pow(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '%') {
            DOIF(
                RECENT(double, 1) = fmod(RECENT(double, 1), RECENT(double, 0)),
                __ezc_mpf_mod(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_fmod(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '>') {
            INC(1);
            i++;
        } else if (code[i] == '<') {
            INC(-1);
            i++;
        } else if (code[i] == ':') {
            DOIF(
                tmp_double = RECENT(double, 0); RECENT(double, 0) = RECENT(double, 1); RECENT(double, 1) = tmp_double;,
                mpf_swap(RECENT(mpf_t, 0), RECENT(mpf_t, 1));,
                mpfr_swap(RECENT(mpfr_t, 0), RECENT(mpfr_t, 1));
            )
            i++;
        } else if (code[i] == '.') {
            INC(1);
            DOIF(
                RECENT(double, 0) = RECENT(double, 1);,
                mpf_set(RECENT(mpf_t, 0), RECENT(mpf_t, 1));,
                mpfr_set(RECENT(mpfr_t, 0), RECENT(mpfr_t, 1), EZC_RND);
            )
            i++;
        } else if (code[i] == '$') {
            DOIF(
                RECENT(double, 0) = STK(double, (int)floor(RECENT(double, 0)));,
                mpf_set(RECENT(mpf_t, 0), STK(mpf_t, mpf_get_ui(RECENT(mpf_t, 0))));,
                mpfr_set(RECENT(mpfr_t, 0), STK(mpfr_t, mpfr_get_ui(RECENT(mpfr_t, 0), MPFR_RNDD)), EZC_RND);
            )
            i++;
        } else if (code[i] == '_') {
            INC(1);
            DOIF(
                RECENT(double, 0) = stack_ptr,
                mpf_set_ui(RECENT(mpf_t, 0), stack_ptr);,
                mpfr_set_ui(RECENT(mpfr_t, 0), stack_ptr, EZC_RND);
            )
            i++;
        } else if (IS_ALPHA(code[i])) {
            j = 0;
            while (IS_ALPHA(code[i])) {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
            if (STR_EQ(PSTR, "p") || STR_EQ(PSTR, "print")) {
                print_idx(stack_ptr);
                printf("\n");
            } else if (STR_EQ(PSTR, "s") || STR_EQ(PSTR, "stk")) {
                dump();
            } else if (STR_EQ(PSTR, "pi")) {
                INC(1);
                DOIF(
                    RECENT(double, 0) = M_PI,
                    __ezc_mpf_pi(RECENT(mpf_t, 0)),
                    mpfr_const_pi(RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "sin")) {
                DOIF(
                    RECENT(double, 0) = sin(RECENT(double, 0)),
                    fail("MPF does not support sin"),
                    mpfr_sin(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "cos"))  {
                DOIF(
                    RECENT(double, 0) = cos(RECENT(double, 0)),
                    fail("MPF does not support cos"),
                    mpfr_cos(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "tan")) {
                DOIF(
                    RECENT(double, 0) = tan(RECENT(double, 0)),
                    fail("MPF does not support tan"),
                    mpfr_tan(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "sqrt")) {
                DOIF(
                    RECENT(double, 0) = sqrt(RECENT(double, 0)),
                    mpf_sqrt(RECENT(mpf_t, 0), RECENT(mpf_t, 0)),
                    mpfr_sqrt(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "log"))  {
                DOIF(
                    RECENT(double, 0) = log(RECENT(double, 0)),
                    fail("MPF does not support log"),
                    mpfr_log(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND)
                )
            } else if (STR_EQ(PSTR, "logb")) {
                DOIF(
                    RECENT(double, 1) = log(RECENT(double, 1)) / log(RECENT(double, 0)); INC(-1),
                    fail("MPF does not support logb");,
                    mpfr_log(RECENT(mpfr_t, 0), RECENT(mpfr_t, 0), EZC_RND); 
                    mpfr_log(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), EZC_RND);
                    mpfr_div(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND);
                    INC(-1);
                )
            } else {
                char estr[PSTR_LEN];
                sprintf(estr, "Unknown function %s\n", PSTR);
                fail(estr);
            }
        } else {
            fail("Unexpected character");
            i++;
        }
    }
}

void interperet() {
	printf(PACKAGE_NAME " (mini)\n");
    #ifdef USE_READLINE
        char *inpt;
        while (true) {
            inpt = readline( "> " );
            eval(inpt);
            add_history(inpt);
        }
    #else
    	char line[MAX_INTERPERET_LEN];
        char cc;
        printf(" > ");
        while (fgets(line, sizeof(line), stdin)) {
            if (line[strlen(line)-1] == '\n') {
                line[strlen(line)-1] = 0;
            }
            eval(line);
            printf(" > ");
        }
    #endif

}

int main(int argc, char *argv[]) {
    
    int i = 1;
    while (i < argc) {
        if (STR_EQ(argv[i], "-h") || STR_EQ(argv[i], "--help")) {
	        printf("Usage: %s [- | -e [EXPR]] [OPTIONS ...]\n\n", argv[0]);
            printf("    -                        Run a shell\n");
            printf("    -e, --expr               Run an expression\n");
            return 0;
        } else if (STR_EQ(argv[i], "-")) {
            do_interperet = true;
        } else if (STR_EQ(argv[i], "-e") || STR_EQ(argv[i], "--expr")) {
            do_interperet = false;
        }
        i++;
    }
    #ifdef USE_GMP
        mpf_set_default_prec(min_prec);
    #endif
    #ifdef USE_MPFR 
        mpfr_set_default_prec(min_prec);
    #endif

    stack = (void *)malloc(sizeof(void *));

    DOIF(
        *stack = (double *)malloc(sizeof(double) * STACK_NUM),
        *stack = (mpf_t *)malloc(sizeof(mpf_t) * STACK_NUM),
        *stack = (mpfr_t *)malloc(sizeof(mpfr_t) * STACK_NUM)
    );

    #ifdef USE_GMP
        #ifdef USE_MPFR
            mpfr_init2(tmp_mpfr, min_prec);
        #endif
        mpf_init2(tmp_mpf, min_prec);
    #endif

    if (do_interperet) {
        interperet();
    } else {
        i = 1;
        while (i < argc) {
            if (STR_EQ(argv[i], "-e") || STR_EQ(argv[i], "--expr")) {
                eval(argv[i+1]);
                i++;
            }
            i++;
        }
    }
    dump();
}

