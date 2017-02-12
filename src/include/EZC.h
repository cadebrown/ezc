/*

C 2017 ChemicalDevelopment (GPL v3)

EZC.h 1.4.0

Contains header for EZC.c
*/


#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>

mpfr_t NaN, INF, NINF;

long long ezc_prec;

void ezc_getprec(mpfr_t r);

void ezc_prec_literal(long long x);
void ezc_prec_index(int index);
void ezc_prec_f(mpfr_t x);
void ezc_prec_init();

void ezc_stime();
void ezc_etime();
void ezc_time(mpfr_t r);

void ezc_wait(mpfr_t w);


void ezc_free(mpfr_t r);
void ezc_prompt(mpfr_t r, char *prompt);
mpfr_ptr ezc_next_const_base(char _set_val[], int base);
mpfr_ptr ezc_next_const(char _set_val[]);
mpfr_ptr ezc_get_arg_base(int val, int base);
mpfr_ptr ezc_get_arg(int val);
mpz_ptr ezc_mpz(mpz_t r, mpfr_t a);
void ezc_init(int __argc, char *__argv[]);
void ezc_add(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_sub(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_mul(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_div(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_pow(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_mod(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_near_2(mpfr_t r, mpfr_t a);
void ezc_near_3(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_trunc_2(mpfr_t r, mpfr_t a);
void ezc_trunc_3(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_initset(mpfr_t a, char val[]);
void ezc_set_str(mpfr_t a, char val[]);
void ezc_set(mpfr_t a, mpfr_t b);

void ezc_max(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_min(mpfr_t r, mpfr_t a, mpfr_t b);
int ezc_sgn(mpfr_t r);
void ezc_rand_1(mpfr_t r);
void ezc_rand_2(mpfr_t r, mpfr_t a);
void ezc_rand_3(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_randgauss(mpfr_t r);
void ezc_sqrt(mpfr_t r, mpfr_t a);
void ezc_cbrt(mpfr_t r, mpfr_t a);
void ezc_hypot(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_exp(mpfr_t r, mpfr_t a);
void ezc_log_2(mpfr_t r, mpfr_t a);
void ezc_log_3(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_agm(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_gamma(mpfr_t r, mpfr_t a);
void ezc_fact(mpfr_t r, mpfr_t a);
void ezc_erf(mpfr_t r, mpfr_t a);
void ezc_binomcoef(mpfr_t r, mpfr_t n, mpfr_t k);
void ezc_zeta(mpfr_t r, mpfr_t a);

char* ezc_mpzstr(mpfr_t r);
void ezc_echo(char msg[]);
void ezc_printf(char msg[]);
void ezc_varb(mpfr_t a, int base);
void ezc_var(mpfr_t a);
void ezc_svar(mpfr_t a);
void ezc_rawvar(mpfr_t a);
void ezc_intvar(mpfr_t a);
void ezc_rawintvar(mpfr_t a);
void ezc_file(mpfr_t a, char name[]);

void ezc_pi(mpfr_t r);

int get_use_deg();

void ezc_deg(mpfr_t r, mpfr_t a);
void ezc_rad(mpfr_t r, mpfr_t a);
void _TIN(mpfr_t r, mpfr_t a);
void ezc_sin(mpfr_t r, mpfr_t a);
void ezc_csc(mpfr_t r, mpfr_t a);
void ezc_cos(mpfr_t r, mpfr_t a);
void ezc_sec(mpfr_t r, mpfr_t a);
void ezc_tan(mpfr_t r, mpfr_t a);
void ezc_cot(mpfr_t r, mpfr_t a);

void ezc_asin(mpfr_t r, mpfr_t a);
void ezc_acsc(mpfr_t r, mpfr_t a);
void ezc_acos(mpfr_t r, mpfr_t a);
void ezc_asec(mpfr_t r, mpfr_t a);
void ezc_atan(mpfr_t r, mpfr_t a);
void ezc_atan2(mpfr_t r, mpfr_t a, mpfr_t b);
void ezc_acot(mpfr_t r, mpfr_t a);

void ezc_sinh(mpfr_t r, mpfr_t a);
void ezc_csch(mpfr_t r, mpfr_t a);
void ezc_cosh(mpfr_t r, mpfr_t a);
void ezc_sech(mpfr_t r, mpfr_t a);
void ezc_htanh(mpfr_t r, mpfr_t a);
void ezc_hcoth(mpfr_t r, mpfr_t a);


void ezc_asinh(mpfr_t r, mpfr_t a);
void ezc_acsch(mpfr_t r, mpfr_t a);
void ezc_acosh(mpfr_t r, mpfr_t a);
void ezc_asech(mpfr_t r, mpfr_t a);
void ezc_atanh(mpfr_t r, mpfr_t a);
void ezc_acoth(mpfr_t r, mpfr_t a);