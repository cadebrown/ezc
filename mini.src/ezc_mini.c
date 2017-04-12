/* ezc_mini.c -- main file for EZC mini

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

#include <math.h>

#include "config.h"


#define STRSIZE(bits) (long long)((bits) / (3.321))
#define BITSIZE(len)  (long long)((len) * 3.321)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))


#ifdef USE_GMP
    #include <gmp.h>
    #ifdef USE_MPFR
        #include <mpfr.h>
        #define FLOAT_TYPE mpfr_t
        #define EZC_RND MPFR_RNDN

        #define INIT(x) mpfr_init(x);
        #define PREC(x) mpfr_get_prec(x)
        #define SET(r, a) mpfr_set(r, a, EZC_RND);
        #define SETPREC(r, a) mpfr_set_prec(r, MAX(a, min_prec));

        #define SWAP(a, b) mpfr_swap(a, b);

        #define ADD(r, a, b) mpfr_add(r, a, b, EZC_RND);
        #define SUB(r, a, b) mpfr_sub(r, a, b, EZC_RND);
        #define MUL(r, a, b) mpfr_mul(r, a, b, EZC_RND);
        #define DIV(r, a, b) mpfr_div(r, a, b, EZC_RND);
        #define POW(r, a, b) mpfr_pow(r, a, b, EZC_RND);

        #define SQRT(r, a) mpfr_sqrt(r, a, EZC_RND);
        #define LOG(r, a) mpfr_log(r, a, EZC_RND);
        #define LOGB(r, a, b) mpfr_log(tmp_float, b, EZC_RND); mpfr_log(r, a, EZC_RND); DIV(r, a, tmp_float);

        #define SIN(r, a) mpfr_sin(r, a, EZC_RND);
        #define COS(r, a) mpfr_cos(r, a, EZC_RND);
        #define TAN(r, a) mpfr_tan(r, a, EZC_RND);
    #else
        #define FLOAT_TYPE mpf_t

        #define INIT(x) mpf_init(x);
        #define PREC(x) mpf_get_prec(x)
        #define SET(r, a) mpf_set(r, a);
        #define SETPREC(r, a) mpf_set_prec(r, MAX(a, min_prec));

        #define SWAP(a, b) mpf_swap(a, b);

        #define ADD(r, a, b) mpf_add(r, a, b);
        #define SUB(r, a, b) mpf_sub(r, a, b);
        #define MUL(r, a, b) mpf_mul(r, a, b);
        #define DIV(r, a, b) mpf_div(r, a, b);
        #define POW(r, a, b) fail("POW not defined (use MPFR)");

        #define SQRT(r, a) mpf_sqrt(r, a);
        #define LOG(r, a) fail("LOG not defined (use MPFR)");
        #define LOGB(r, a, b) fail("LOGB not defined (use MPFR)");

        #define SIN(r, a) fail("SIN not defined (use MPFR)");
        #define COS(r, a) fail("COS not defined (use MPFR)");
        #define TAN(r, a) fail("TAN not defined (use MPFR)");
    #endif

#else
    #define FLOAT_TYPE double
    #define INIT(x) x = 0;
    #define PREC(x) 53
    
    #define SET(r, a) r = a;

    #define ADD(r, a, b) r = a + b
    #define SUB(r, a, b) r = a - b
    #define MUL(r, a, b) r = a * b
    #define DIV(r, a, b) r = a / b
    #define POW(r, a, b) r = pow(a, b)
    #define SWAP(a, b) tmp_float = a; a = b; b = tmp_float;
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

#define STK(n) stack[n]
#define RECENT(n) STK(stack_ptr - n)
#define INC(n) inc(n)


int stack_ptr = -1;
int max_stack_ptr = -1;
FLOAT_TYPE stack[STACK_NUM];

FLOAT_TYPE tmp_float;

int min_prec = 53;

char PSTR[PSTR_LEN];
char *ccode;
int cptr;

void inc(int n) {
    if (n > 0) {
        #ifdef USE_GMP
            int i = stack_ptr + 1;
            stack_ptr += n;
            while (i <= stack_ptr) {
                if (i > max_stack_ptr) {
                    INIT(STK(i));
                    max_stack_ptr = i;
                }
                i++;
            }
        #else
            stack_ptr += n;
        #endif
    } else if (n < 0) {
        stack_ptr += n;
    }

}

void dump() {
    long long x = 0;
    while (x <= stack_ptr) {
        #ifdef USE_GMP
            #ifdef USE_MPFR
                mpfr_printf(KCYN "%.*Rf" KNRM, STRSIZE(PREC(STK(x))), STK(x));
            #else
                gmp_printf(KCYN "%.*Ff" KNRM, STRSIZE(PREC(STK(x))), STK(x));
            #endif
        #else
            printf(KCYN "%.17g" KNRM, STK(x));
        #endif
        if (x != stack_ptr) {
            printf(KBLU ", " KNRM);
        }
        x++;
    }
    printf("\n");
}

void fail(char reason[]) {
    printf(KRED "ERROR: %s" KMAG, reason);

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
            #ifdef USE_GMP
                SETPREC(RECENT(0), BITSIZE(strlen(PSTR)))
                #ifdef USE_MPFR
                    mpfr_set_str(RECENT(0), PSTR, 10, EZC_RND);
                #else
                    mpf_set_str(RECENT(0), PSTR, 10);
                #endif
            #else
                RECENT(0) = strtod(PSTR, NULL);
            #endif
        } else if (code[i] == '+') {
            ADD(RECENT(1), RECENT(1), RECENT(0))
            INC(-1);
            i++;
        } else if (code[i] == '-') {
            SUB(RECENT(1), RECENT(1), RECENT(0))
            INC(-1);
            i++;
        } else if (code[i] == '*') {
            MUL(RECENT(1), RECENT(1), RECENT(0))
            INC(-1);
            i++;
        } else if (code[i] == '/') {
            DIV(RECENT(1), RECENT(1), RECENT(0))
            INC(-1);
            i++;
        } else if (code[i] == '^') {
            POW(RECENT(1), RECENT(1), RECENT(0))
            INC(-1);
            i++;
        } else if (code[i] == '>') {
            INC(1);
            i++;
        } else if (code[i] == '<') {
            INC(-1);
            i++;
        } else if (code[i] == ':') {
            SWAP(RECENT(0), RECENT(1));
            i++;
        } else if (code[i] == '.') {
            INC(1);
            SET(RECENT(0), RECENT(1));
            i++;
        } else if (code[i] == '$') {
            #ifdef USE_GMP
                #ifdef USE_MPFR
                    SET(RECENT(0), STK(mpfr_get_ui(RECENT(0), EZC_RND)));
                #else
                    SET(RECENT(0), STK(mpf_get_ui(RECENT(0))));
                #endif
            #else
                RECENT(0) = STK(RECENT(0));
            #endif
            i++;
        } else if (code[i] == '_') {
            #ifdef USE_GMP
                INC(1);
                #ifdef USE_MPFR
                    mpfr_set_ui(RECENT(0), stack_ptr+1, EZC_RND);
                #else
                    mpf_set_ui(RECENT(0), stack_ptr+1);
                #endif
            #else
                INC(1)
                RECENT(0) = stack_ptr+1;
            #endif
            i++;
        } else if (IS_ALPHA(code[i])) {
            j = 0;
            while (IS_ALPHA(code[i])) {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
             if (STR_EQ(PSTR, "sin"))        { SIN(RECENT(0), RECENT(0));
            } else if (STR_EQ(PSTR, "cos"))  { COS(RECENT(0), RECENT(0));
            } else if (STR_EQ(PSTR, "tan"))  { TAN(RECENT(0), RECENT(0));
            } else if (STR_EQ(PSTR, "sqrt")) { SQRT(RECENT(0), RECENT(0));
            } else if (STR_EQ(PSTR, "log"))  { LOG(RECENT(0), RECENT(0));
            } else if (STR_EQ(PSTR, "logb")) { LOGB(RECENT(1), RECENT(1), RECENT(0)); INC(-1);
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


int main(int argc, char *argv[]) {
    #ifdef USE_GMP
        mpf_set_default_prec(100);
    #endif

    INIT(tmp_float);


    eval(argv[1]);
    dump();
}

