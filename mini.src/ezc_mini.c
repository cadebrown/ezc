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

#define SUB2F(f) a = pop(); push(f(pop(), a))
#define SUB1F(f) push(f(pop()))
#define SUBO(o) a = pop(); push(pop() o a)

/* math functions */
#define csc(x) (1.0 / sin(x))
#define sec(x) (1.0 / cos(x))
#define cot(x) (1.0 / tan(x))
#define acsc(x) (asin(1.0 / x))
#define asec(x) (acos(1.0 / x))
#define acot(x) (atan(1.0 / x))
#define logbase(x, y) (log(x) / log(y))

int stack_ptr = -1;
double stack[STACK_NUM];
char PSTR[PSTR_LEN];
char *ccode;
int cptr;
double a, b;


void push(double x) {
    stack[++stack_ptr] = x;
}

double pop() {
    return stack[stack_ptr--];
}

double get(int idx) {
    return stack[idx];
}

void dump() {
    long long x = 0;
    while (x <= stack_ptr) {
        printf(KCYN "%.17g" KNRM, stack[x]);
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
            push(strtod(PSTR, NULL));
        } else if (code[i] == '+') {
            SUBO(+);
            i++;
        } else if (code[i] == '-') {
            SUBO(-);
            i++;
        } else if (code[i] == '*') {
            SUBO(*);
            i++;
        } else if (code[i] == '/') {
            SUBO(/);
            i++;
        } else if (code[i] == '^') {
            SUB2F(pow);
            i++;
        } else if (code[i] == '>') {
            stack_ptr++;
            i++;
        } else if (code[i] == '<') {
            stack_ptr--;
            i++;
        } else if (code[i] == ':') {
            a = pop();
            b = pop();
            push(a);
            push(b);
            i++;
        } else if (code[i] == '.') {
            push(get(stack_ptr));
            i++;
        } else if (code[i] == '$') {
            SUB1F(get);
            i++;
        } else if (IS_ALPHA(code[i])) {
            j = 0;
            while (IS_ALPHA(code[i])) {
                PSTR[j++] = code[i++];
            }
            PSTR[j] = 0;
            /*  */ if (STR_EQ(PSTR, "sin")) { SUB1F(sin);
            } else if (STR_EQ(PSTR, "cos")) { SUB1F(cos);
            } else if (STR_EQ(PSTR, "tan")) { SUB1F(tan);
            } else if (STR_EQ(PSTR, "csc")) { SUB1F(csc);
            } else if (STR_EQ(PSTR, "sec")) { SUB1F(sec);
            } else if (STR_EQ(PSTR, "cot")) { SUB1F(cot);
            } else if (STR_EQ(PSTR, "asin")) { SUB1F(asin);
            } else if (STR_EQ(PSTR, "acos")) { SUB1F(acos);
            } else if (STR_EQ(PSTR, "atan")) { SUB1F(atan);
            } else if (STR_EQ(PSTR, "acsc")) { SUB1F(acsc);
            } else if (STR_EQ(PSTR, "asec")) { SUB1F(asec);
            } else if (STR_EQ(PSTR, "acot")) { SUB1F(acot);
            /* hyperbolic                                */
            } else if (STR_EQ(PSTR, "sinh")) { SUB1F(sinh);
            } else if (STR_EQ(PSTR, "cosh")) { SUB1F(cosh);
            } else if (STR_EQ(PSTR, "tanh")) { SUB1F(tanh);
            /* most of math.h                            */
            } else if (STR_EQ(PSTR, "sqrt")) { SUB1F(sqrt);
            } else if (STR_EQ(PSTR, "log")) { SUB1F(log);
            } else if (STR_EQ(PSTR, "logb")) { SUB2F(logbase);
            /* err                                       */
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
    eval(argv[1]);
    dump();
}

