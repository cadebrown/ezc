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

// These are checked in configure.ac
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>

// TODO: should we make math.h optional?
#include <math.h>

// Optional includes from configure.ac
#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef USE_GMP
    #include <gmp.h>
#endif

#ifdef USE_MPFR
    #include <mpfr.h>
#endif

// readline for completion/history
#ifdef USE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

// color constants for printing
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// constants for max stack size, and the max constant string size.
#define K_STACK  (1000)
#define K_CSTR   (1000)

// enums to say whether we are running doubles, mpf_t s, or mpfr_t s
#define E_DOUBLE (0x01)
#define E_MPF    (0x02)
#define E_MPFR   (0x03)

// function definitions
void dump();
void print_idx(int x);

void show_help();
void show_info();

void fail(char reason[]);

void eval(char code[]);
void interperet();

void end();
int main(int argc, char *argv[]);

// argv[0], but so that show_help() can see it
char *exec_name;

bool do_interperet = false, keep_alive_if_fail = false;

// conditionally set the default type, and (if we have them) by order:
// mpfr_t, mpf_t, double

#ifdef USE_GMP
    #ifdef USE_MPFR
        int type = E_MPFR;
    #else
        int type = E_MPF;
    #endif
#else
    int type = E_DOUBLE;
#endif

// where are we at at the stack?
int stack_ptr = -1;
// what is the maximum that stack_ptr has been at.
// use this for initialization of floats only once.
int max_stack_ptr = -1;

// a stack that can have any type
void **stack;

// minimum precision to operate as (doesn't matter for doubles, and is 53).
int min_prec = 64;

// a constant string, so the function doesn't have to malloc tmp strings.
char PSTR[K_CSTR];

// current code, so fail can use it
char *ccode;
int cptr;

//temporary variables
double tmp_double;
#ifdef USE_GMP
  #ifdef USE_MPFR
    mpfr_t tmp_mpfr;
  #endif
  mpf_t tmp_mpf;
#endif


//useful macros

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define STRSIZE(bits) (long long)((bits) / (3.321))
#define BITSIZE(len)  (long long)((len) * 3.321)

#define IS_SPACE(ch) (ch == ' ' || ch == ',' || ch == '\n')
#define IS_CONST(ch) ((ch - '0' >= 0 && ch - '9' <= 0))
#define IS_ALPHA(ch) ((ch - 'a' >= 0 && ch - 'z' <= 0) || (ch - 'A' >= 0 && ch - 'Z' <= 0))

#define STR_EQ(a, b) (strcmp(a, b) == 0)

#define STK(t, n) ((t *)(*stack))[n]
#define RECENT(t, n) STK(t, stack_ptr - n)
#define INC(n) inc(n)

#define ERR_STR(name) char name[K_CSTR];

// DOIF macro:
// used like this:

/*
DOIF(
    {code if using double},
    {code if using mpf_t},
    {code if using mpfr_t}
)

this handles if you don't have a certain type, and uses `type` to figure out which.
*/
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

// here are some methods to supplement mpf_t s.
// mpf s are kind of legacy, and if mpfr is available,
// we basically use the mpfr functions to find it,
// or we implement our own (somewhat slower) methods
// to emulate these functions
#ifdef USE_GMP
    #ifdef USE_MPFR
        // uses mpfr_const_pi to calculate it using smarter methods (and cache it)
        void __ezc_mpf_pi(mpf_t x) {
            mpfr_set_prec(tmp_mpfr, mpf_get_prec(x));
            mpfr_const_pi(tmp_mpfr, EZC_RND);
            mpfr_get_f(x, tmp_mpfr, EZC_RND);
        }

        // I'm debating using this one or just sticking with our hack below.
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

            precision_bits = (mpf_get_prec(x)) + 4;

            mpf_set_default_prec(precision_bits);

            mpf_inits(result, con, A, B, F, sum, NULL);
            mpz_inits(a, b, c, d, e, NULL);

            mpf_set_ui(sum, 0);

            mpf_sqrt_ui(con, 10005);
            mpf_mul_ui(con, con, 426880);

            for (k = 0; k < iterations; k++) {
                threek = 3*k;

                mpz_fac_ui(a, 6*k);

                mpz_set_ui(b, 545140134);
                mpz_mul_ui(b, b, k);
                mpz_add_ui(b, b, 13591409);

                // c = (3k)!
                mpz_fac_ui(c, threek);

                // d = (k!)^3
                mpz_fac_ui(d, k);
                mpz_pow_ui(d, d, 3);

                // e = -640320^(3k)
                mpz_ui_pow_ui(e, 640320, threek);
                if (threek & 1 == 1) { 
                    mpz_neg(e, e); 
                }

                // A = a * b
                mpz_mul(a, a, b);
                mpf_set_z(A, a);

                // B = c * e * d
                mpz_mul(c, c, d);
                mpz_mul(c, c, e);
                mpf_set_z(B, c);

                // S_n = A / B, SS = sum(S_n)
                mpf_div(F, A, B);
                mpf_add(sum, sum, F);
            }

            // pi = (con / SS)
            mpf_ui_div(sum, 1, sum);
            mpf_mul(x, sum, con);

            // free GMP variables
            mpf_clears(result, con, A, B, F, sum, NULL);
            mpz_clears(a, b, c, d, e, NULL);
        }

        // x % y = x - (y*(x//y))
        void __ezc_mpf_mod(mpf_t r, mpf_t x, mpf_t y) {
            mpf_set_prec(tmp_mpf, mpf_get_prec(r));

            // tmp_mpf = y*(x//y)
            mpf_div(tmp_mpf, x, y);
            mpf_trunc(tmp_mpf, tmp_mpf);
            mpf_mul(tmp_mpf, tmp_mpf, y);

            // r = x - tmp_mpf
            mpf_sub(r, x, tmp_mpf);
        }
    #endif
#endif

void fail(char reason[]) {
    // print our code
    printf(KBLU "At:" KNRM "\n");
    printf(KCYN);

    // print a `^` pointing to the error
    int s_i, i = 0, j;
    while (i < strlen(ccode)) {
        s_i = i;
        while (i < strlen(ccode)) {
            if (ccode[i] == '\n') {
                i++;
                break;
            } else {
                printf("%c", ccode[i]);
            }
            i++;
        }
        printf("\n");
        if ((cptr >= j && cptr < i) || (i >= strlen(ccode))) {
            printf(KYEL);
            j = s_i;
            while (j < i) {
                if (j == cptr) {
                    printf("^");
                } else {
                    printf(" ");
                }
                j++;
            }
            printf(KCYN "\n");
        }
    }
    printf(KNRM "\n");

    // dump our info
    printf("Stack:\n");
    dump();

    printf(KRED "\nerror: " KMAG "%s\n" KNRM, reason);

    // if keep_alive_if_fail, then we just return, and keep executing code (although it throws away the rest of the current line)
    if (keep_alive_if_fail) {
        return;
    } else {
        exit (2);
    }
}

// increments by n
// TODO: perhaps free or change the precision back to min?
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

// prints a single index
void print_idx(int x) {
    DOIF(
        printf(KCYN "%.17g" KNRM, STK(double, x));  ,
        gmp_printf(KCYN "%.*Ff" KNRM, STRSIZE(MAX(min_prec, mpf_get_prec(STK(mpf_t, x)))), STK(mpf_t, x)); ,
        mpfr_printf(KCYN "%.*Rf" KNRM, STRSIZE(MAX(min_prec, mpfr_get_prec(STK(mpfr_t, x)))), STK(mpfr_t, x));
    );
}

// ends, and exits disregarding keep_alive_if_fail
void end() {
    printf("Stack:\n");
    dump();
    exit (0);
}

// dumps the stack out to stdout
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
    // set current code to this
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
        
        if (    IS_CONST(code[i]) || 
                (code[i] == '-' && IS_CONST(code[i+1])) ||
                (code[i] == '.' && IS_CONST(code[i+1]))) {
            // parse constant
            j = 0;
            if (code[i] == '-') {
                PSTR[j++] = code[i++];
            }
            while (IS_CONST(code[i]) || code[i] == '.') {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
            INC(1);
            // conditional add
            DOIF(
                // if using doubles
                RECENT(double, 0) = strtod(PSTR, NULL)                                 ,

                // if using mpf
                mpf_set_prec(RECENT(mpf_t, 0), MAX(min_prec, BITSIZE(strlen(PSTR))));
                mpf_set_str(RECENT(mpf_t, 0), PSTR, 10);                               ,

                // if using mpfr
                mpfr_set_prec(RECENT(mpfr_t, 0), MAX(min_prec, BITSIZE(strlen(PSTR))));
                mpfr_set_str(RECENT(mpfr_t, 0), PSTR, 10, EZC_RND)
            );
        // operators, simple mapping (maybe should introduce macro for this)
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
                // consider a compatability layer to just use doubles here
                fail("MPF does not support ^"),
                mpfr_pow(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        } else if (code[i] == '%') {
            DOIF(
                RECENT(double, 1) = fmod(RECENT(double, 1), RECENT(double, 0)),
                // hacked in here
                __ezc_mpf_mod(RECENT(mpf_t, 1), RECENT(mpf_t, 1), RECENT(mpf_t, 0)),
                mpfr_fmod(RECENT(mpfr_t, 1), RECENT(mpfr_t, 1), RECENT(mpfr_t, 0), EZC_RND)
            )
            INC(-1);
            i++;
        // simple commands
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
        // this is a function
        } else if (IS_ALPHA(code[i])) {
            j = 0;
            while (IS_ALPHA(code[i])) {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
            if (STR_EQ(PSTR, "p") || STR_EQ(PSTR, "print")) {
            } else if (STR_EQ(PSTR, "q") || STR_EQ(PSTR, "quit")) {
                end();
            } else if (STR_EQ(PSTR, "s") || STR_EQ(PSTR, "stk")) {
                dump();
            // math constants are a function as well
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
                ERR_STR(estr);
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
    keep_alive_if_fail = true;
    
    #ifdef USE_READLINE
        char *inpt;
        while (true) {
            inpt = readline(" > " );
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

    keep_alive_if_fail = false;
}

void show_help() {
    	printf(KCYN "Usage: " KGRN "%s" KCYN " [- | -e [EXPR] | -f [FILE]] [OPTIONS ...]" KNRM "\n", exec_name);
        printf("\n");
        printf("  -                                  Run an interactive shell\n");
        printf("  -e, --expr                         Run an expression\n");
        printf("  -f, --file                         Run file contents\n");
        printf("\n");
        printf("  -t, --type                         Set the default type (double, mpf, mpfr)\n");
        printf("  -v, --version, -i, --info          Print version and info, then quit\n");
        printf("\n");
        printf("<" PACKAGE_BUGREPORT ">\n");
}

void show_info() {
        printf(PACKAGE_NAME " (mini)\n");
        printf("Version: " PACKAGE_VERSION "\n");
        printf("\n");
        printf("Compiled with:\n");
        #ifdef USE_GMP
        printf("    GMP (version %s)\n", gmp_version);
        #endif
        #ifdef USE_MPFR
        printf("    MPFR (version " MPFR_VERSION_STRING ")\n");
        #endif
        #ifdef USE_READLINE
        printf("    Readline\n");
        #endif

        printf("\n");
        printf("<" PACKAGE_BUGREPORT ">\n");
}

int main(int argc, char *argv[]) {
    exec_name = argv[0];
    int i = 1;
    while (i < argc) {
        if (STR_EQ(argv[i], "-h") || STR_EQ(argv[i], "--help")) {
            show_help();
            return 0;
        } else if (STR_EQ(argv[i], "-v") || STR_EQ(argv[i], "--version") || 
                   STR_EQ(argv[i], "-i") || STR_EQ(argv[i], "--info")) {
            show_info();
            return 0;
        } else if (STR_EQ(argv[i], "-e") || STR_EQ(argv[i], "-f")) {
            // these are only used later, so just ignore them (and the argument right after them).
            i += 2;
            continue;
        } else if (STR_EQ(argv[i], "-")) {
            // all we need to do is make sure we interperet.
            do_interperet = true;
        } else if (STR_EQ(argv[i], "-t") || STR_EQ(argv[i], "--type")) {
            // sets the type to execute as. (this directly affects the macro DOIF)
            i++;
            if (STR_EQ(argv[i], "double")) {
                type = E_DOUBLE;
            } else if (STR_EQ(argv[i], "mpf")) {
                // logic to print an error message if not compiled to support mpf
                #ifdef USE_GMP
                    type = E_MPF;
                #else
                    printf("Set type as mpf, but not compiled with GMP\n");
                    exit (3);
                #endif
            } else if (STR_EQ(argv[i], "mpfr")) {
                // logic to print an error message if not compiled to support mpfr, and tell if it has GMP
                #ifdef USE_GMP
                    #ifdef USE_MPFR
                        type = E_MPFR;
                    #else
                        printf("Set type as mpfr, but not compiled with MPFR (does have GMP)\n");
                        exit (4);
                    #endif
                #else
                    printf("Set type as mpfr, but not compiled with GMP or MPFR\n");
                    exit (3);
                #endif       
            } else {
                printf("Incorrect type\n");
                exit (2);
            }
        } else {
            // fail here
            printf(KRED "Unrecognized option[s]: `%s`\n" KNRM, argv[i]);
            printf("\n");
            show_help();
            return 1;         
        }
        i++;
    }

    // set minimum precisions (only if compiled with them.)
    #ifdef USE_GMP
        mpf_set_default_prec(min_prec);
    #endif
    #ifdef USE_MPFR 
        mpfr_set_default_prec(min_prec);
    #endif

    // malloc the pointer
    stack = (void *)malloc(sizeof(void *));

    // now malloc the actual array
    DOIF(
        *stack = (double *)malloc(sizeof(double) * K_STACK),
        *stack = (mpf_t *)malloc(sizeof(mpf_t) * K_STACK),
        *stack = (mpfr_t *)malloc(sizeof(mpfr_t) * K_STACK)
    );

    // initialization of temporary variables
    #ifdef USE_GMP
        mpf_init2(tmp_mpf, min_prec);
    #endif
    #ifdef USE_MPFR
        mpfr_init2(tmp_mpfr, min_prec);
    #endif

    i = 1;
    while (i < argc) {
        if (STR_EQ(argv[i], "-e") || STR_EQ(argv[i], "--expr")) {
            i++;
            if (i < argc) {
                eval(argv[i]);
            } else {
                ERR_STR(err_str);
                sprintf(err_str, "%s is last argument, and expected something after it\n", argv[i-1]);
                fail(err_str);
            }
        } else if (STR_EQ(argv[i], "-f") || STR_EQ(argv[i], "--file")) {
            if (i < argc) {
                i++;
                FILE *fp = fopen(argv[i], "r");
                if (fp) {
                    fseek (fp, 0, SEEK_END);
                    int clength = ftell (fp);
                    fseek (fp, 0, SEEK_SET);
                    char *cline = (char *)malloc(clength);
                    fread(cline, 1, clength, fp);
                    fclose(fp);
                    eval(cline);
                } else {
                    ERR_STR(err_str);
                    sprintf(err_str, "Couldn't open file \"%s\"\n", argv[i]);
                    fail(err_str);
                }
            } else {
                ERR_STR(err_str);
                sprintf(err_str, "Don't know what to do with this option: %s\n", argv[i]);
                fail(err_str);
            }
        }
        i++;
    }

    if (do_interperet) {
        interperet();
    }

    dump();
}

