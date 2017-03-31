//#define USE_GMP

#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezc_generic.h"

#define ISDIGIT(x) (x - '0' >= 0 && x - '0' <= 9)
#define ISCONTROL(x) (x == '<' || x == '>')
#define ISOP(x) (x == '+' || x == '-' || x == '*' || x == '/' || x == '^' || x == '%' || x == '!')
#define ISALPHA(x) (x - 'a' >= 0 && x - 'z' <= 0)
#define ISFUNC(x) (ISALPHA(x) || x == ':')
#define ISSPECIAL(x) (x == '|' || x == '#')
#define ISSPACE(x) (x == ' ' || x == ',')

#define ERROR(s) fprintf(stderr, s); exit (1);

#define cc (input[inptr])

EZC_STACK_TYPE vals[MAXSTACKSIZE];
EZC_FLAG_TYPE flags[MAXSTACKSIZE];

long long inptr, ptr;

char *input;


void skip_whitespace(void);
void terminate_main(void);
void fail(char reason[]);

int main(int argc, char *argv[]);

#endif
