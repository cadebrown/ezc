//#define USE_GMP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXSTACKSIZE
 #define MAXSTACKSIZE (100)
#endif

#ifdef USE_GMP
 #include <gmp.h>
 #define EZC_STACK_TYPE mpz_t
#else
 #define EZC_STACK_TYPE long long
#endif

#define ISDIGIT(x) (x - '0' >= 0 && x - '0' <= 9)
#define ISCONTROL(x) (x == '<' || x == '>')
#define ISOP(x) (x == '<' || x == '>' || x == '+' || x == '-' || x == '*' || x == '/' || x == '^' || x == '%' || x == '!')
#define ISALPHA(x) (x - 'a' >= 0 && x - 'z' <= 0)

EZC_STACK_TYPE vals[MAXSTACKSIZE];

long long ptr, usr_ptr, op;

char *input;

int main(int argc, char *argv[]);
