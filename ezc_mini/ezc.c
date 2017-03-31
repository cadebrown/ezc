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
#define ISOP(x) (x == '+' || x == '-' || x == '*' || x == '/' || x == '^' || x == '!' || x == '%')


EZC_STACK_TYPE vals[MAXSTACKSIZE];

long long ptr, usr_ptr = 0, op;

char *input;

#ifdef USE_GMP
void handleparse() {
    int off;    
    if (op == '+') {
        mpz_add(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
        off = 1;        
    } else if (op == '-') {
        mpz_sub(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
        off = 1;
    } else if (op == '*') {
        mpz_mul(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
        off = 1;
    } else if (op == '/') {
        mpz_div(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
        off = 1;
    } else if (op == '^') {
        mpz_pow_ui(vals[ptr-2], vals[ptr-2], mpz_get_ui(vals[ptr-1]));
        off = 1;
    } else if (op == '!') {
        mpz_fac_ui(vals[ptr-1], mpz_get_ui(vals[ptr-1]));
        off = 0;
    } else if (op == '%') {
        mpz_mod(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
        off = 1;
    }
    ptr = ptr - off;
    while (off > 0) {
        mpz_set_ui(vals[ptr], 0);
        off--;
    }
}
#else

EZC_STACK_TYPE __pow(EZC_STACK_TYPE a, EZC_STACK_TYPE b) {
    if (b == 0) {
        return 1;
    } else if (b == 1) {
        return a;
    } else if (b == 2) {
        return a * a;
    }
    if (a == 0 || a == 1) {
        return a;
    }
    return ((b & 1)*(a-1)+1) * __pow(a*a, b>>1);
}


EZC_STACK_TYPE __fact(EZC_STACK_TYPE a) {
    EZC_STACK_TYPE i, ret = 1;
    i = 1;
    while (i <= a) {
        ret = ret * i;
        i++;
    }
    return ret;
}


void handleparse() {
    int off;
    if (op == '+') {
        vals[ptr-2] = vals[ptr-2] + vals[ptr-1];
        off = 1;
    } else if (op == '-') {
        vals[ptr-2] = vals[ptr-2] - vals[ptr-1];
        off = 1;
    } else if (op == '*') {
        vals[ptr-2] = vals[ptr-2] * vals[ptr-1];
        off = 1;
    } else if (op == '/') {
        vals[ptr-2] = vals[ptr-2] / vals[ptr-1];
        off = 1;
    } else if (op == '^') {
        vals[ptr-2] = __pow(vals[ptr-2], vals[ptr-1]);
        off = 1;
    } else if (op == '!') {
        vals[ptr-1] = __fact(vals[ptr-1]);
        off = 0;
    } else if (op == '%') {
        vals[ptr-2] = vals[ptr-2] % vals[ptr-1];
        off = 1;
    }
    ptr = ptr - off;
    while (off > 0) {
        vals[ptr] = 0;
        off--;
    }
}
#endif

int main(int argc, char *argv[]) {
    input = argv[1];
    char *buf = (char *)malloc(1000);
    long long i, bufptr;
    #ifdef USE_GMP
     for (i = 0; i < MAXSTACKSIZE; ) {
         mpz_init(vals[i++]);
     }
    #endif
    for (i = 0; i < strlen(input); ) {
        while (input[i] == ' ' || input[i] == ',') {
            i++;
        }
        op = input[i];
        if (ISCONTROL(op)) {
            if (op == '>') {
                ptr++;
            } else if (op == '<') {
                ptr--;
                #ifdef USE_GMP
                 mpz_set_ui(vals[ptr], 0);
                #else
                 vals[ptr] = 0;
                #endif
            }
            i++;
        } else if (ISOP(op)) {
            handleparse();
            i++;
        } else if (ISDIGIT(op)) {
            bufptr = 0;
            while (ISDIGIT(input[i])) {
                buf[bufptr++] = input[i++];
            }
            buf[bufptr] = 0;
            #ifdef USE_GMP
             mpz_set_str(vals[ptr], buf, 10);
            #else
             vals[ptr] = strtoll(buf, NULL, 10);
            #endif
            ptr++;
        }
    }
    #ifdef USE_GMP
     for (i = 0; i < ptr; ++i) {
         gmp_printf("%Zd, ", vals[i]);
     }
     printf("\n");
    #else
     for (i = 0; i < ptr; ++i) {
         printf("%lld, ", vals[i]);
     }
     printf("\n");
    #endif
}

