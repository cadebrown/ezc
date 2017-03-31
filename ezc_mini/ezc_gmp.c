
#ifdef USE_GMP
#include "ezc.h"

void setup(void) {
     for (i = 0; i < MAXSTACKSIZE; ) {
         mpz_init(vals[i++]);
     }
}

void end(void) {
    long long i;
    for (i = 0; i < ptr; ++i) {
       gmp_printf("%Zd, ", vals[i]);
    }
    printf("\n");
}

void handle_constant(char val[]) {
    mpz_set_str(vals[ptr], val, 10);
    ptr++;
}

void handle_operator(int op) {
	switch (op) {
		case '+':
            mpz_add(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
            break;
        case '-':
            mpz_sub(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
            break;
        case '*':
            mpz_mul(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
            break;
        case '/':
            mpz_div(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
            break;
        case '%':
            mpz_mod(vals[ptr-2], vals[ptr-2], vals[ptr-1]);
            break;
        case '^':
            mpz_pow_ui(vals[ptr-2], vals[ptr-2], mpz_get_ui(vals[ptr-1]));
            break;
	}
    mpz_set_ui(vals[ptr], 0);
    ptr--;
}

#endif
