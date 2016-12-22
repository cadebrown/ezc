#EZC_LIB="-I%s/include/ %s/lib/libmpfr.a %s/lib/libgmp.a"

import os

EZC_DIR=os.path.dirname(__file__)
EZC_LIB="-lmpfr -lgmp"

EZC_C="""
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>

int ezc_size_consts = 50, ezc_consts_id = 0, ezc_consts_ov = 0;
gmp_randstate_t ezc_rand_state;
mpfr_t *ezc_consts;

#define EZC_RND MPFR_RNDN
#define EZC_MIN_PREC 20

long long ezc_prec;
#define ezc_bprec (long long)(ezc_prec * 3.3219281)

mpfr_t NaN, INF, NINF;

int _argc;
char **_argv;

void ezc_prec_literal(long long x) {
	if (x < EZC_MIN_PREC) x = EZC_MIN_PREC;
	ezc_prec = x;
	mpfr_set_default_prec((long long)ezc_bprec);
}
void ezc_prec_index(int index) {
	if (index < _argc) {
		ezc_prec_literal(strtoll(_argv[index], NULL, 10));
	}
}
void ezc_prec_f(mpfr_t x) {
	ezc_prec_literal((long long)mpfr_get_si(x, EZC_RND));
}
void ezc_prec_init() {
	if (getenv("EZC_PREC") == NULL) {
		if (_argc > 1) {
			long long _t_max = EZC_MIN_PREC, _i;
			for (_i = 1; _i < _argc; ++_i) {
				if (strlen(_argv[_i]) > _t_max) {
					_t_max = strlen(_argv[_i]);
				}
			}
			ezc_prec_literal(_t_max);
		} else {
			ezc_prec_literal(EZC_MIN_PREC);
		}
	} else {
		ezc_prec_literal(strtoll(getenv("EZC_PREC"), NULL, 10));
	}	
}
mpfr_ptr ezc_next_const_base(char _set_val[], int base) {
	if (ezc_consts_id >= ezc_size_consts) {
		ezc_consts_ov = 1;
		ezc_consts_id %= ezc_size_consts;
	}
	if (ezc_consts_ov == 0) {
		mpfr_init(ezc_consts[ezc_consts_id]);
	}
	mpfr_set_str(ezc_consts[ezc_consts_id], _set_val, base, EZC_RND);
	return (mpfr_ptr) ezc_consts[ezc_consts_id++];
}
mpfr_ptr ezc_next_const(char _set_val[]) {
	return ezc_next_const_base(_set_val, 10);
}
mpfr_ptr ezc_get_arg_base(int val, int base) {
	if (val >= _argc) {
		return ezc_next_const_base("0.0", base);
	} else {
		return ezc_next_const_base(_argv[val], base);
	}
}
mpfr_ptr ezc_get_arg(int val) {
	return ezc_get_arg_base(val, 10);
}
void ezc_init(int __argc, char *__argv[]) {
	_argc = __argc; _argv = __argv;
	mpfr_set_default_rounding_mode(EZC_RND);
	srand(time(NULL));
	gmp_randinit_default(ezc_rand_state);
	gmp_randseed_ui(ezc_rand_state, rand());

	ezc_prec_init();

	ezc_consts = (mpfr_t *)malloc(sizeof(mpfr_t) * ezc_size_consts);

	mpfr_init(NaN); mpfr_set_nan(NaN);
	mpfr_init(INF); mpfr_set_inf(INF, 1);
	mpfr_init(NINF); mpfr_set_inf(NINF, 0);
}
void ezc_add(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_add(r, a, b, EZC_RND); 
}
void ezc_sub(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_sub(r, a, b, EZC_RND); 
}
void ezc_mul(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_mul(r, a, b, EZC_RND); 
}
void ezc_div(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_div(r, a, b, EZC_RND);
}
void ezc_pow(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_pow(r, a, b, EZC_RND); 
}
void ezc_mod(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_fmod(r, a, b, EZC_RND);
}
void ezc_near_2(mpfr_t r, mpfr_t a) { 
	mpfr_round(r, a); 
}
void ezc_near_3(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t _fnear; mpfr_init(_fnear);
	mpfr_div_ui(_fnear, a, 2, EZC_RND);
	mpfr_add(r, _fnear, b, EZC_RND);
	mpfr_fmod(_fnear, r, a, EZC_RND);
	mpfr_sub(r, r, _fnear, EZC_RND);

}
void ezc_trunc_2(mpfr_t r, mpfr_t a) { 
	mpfr_trunc(r, a); 
}
void ezc_trunc_3(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __ftrunc_mult_tmp; mpfr_init(__ftrunc_mult_tmp);
	mpfr_fmod(__ftrunc_mult_tmp, a, b, EZC_RND);
	mpfr_sub(r, a, __ftrunc_mult_tmp, EZC_RND);
	mpfr_clear(__ftrunc_mult_tmp);
}
void ezc_initset(mpfr_t a, char val[]) { 
	mpfr_init(a); 
	mpfr_set_str(a, val, 10, EZC_RND); 
}
void ezc_set_str(mpfr_t a, char val[]) { 
	mpfr_set_str(a, val, 10, EZC_RND); 
}
void ezc_set(mpfr_t a, mpfr_t b) { 
	mpfr_set(a, b, EZC_RND); 
}

void ezc_max(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_max(r, a, b, EZC_RND); 
}
void ezc_min(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_min(r, a, b, EZC_RND); 
}
int ezc_sgn(mpfr_t r) { 
	return mpfr_sgn(r); 
}
void ezc_rand_1(mpfr_t r) { 
	mpfr_urandomb(r, ezc_rand_state); 
}
void ezc_rand_2(mpfr_t r, mpfr_t a) { 
	mpfr_urandomb(r, ezc_rand_state);
	mpfr_mul(r, r, a, EZC_RND);
}
void ezc_rand_3(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_t _bma; mpfr_init(_bma);
	mpfr_sub(_bma, b, a, EZC_RND);
	ezc_rand_2(r, _bma);
	mpfr_add(r, r, a, EZC_RND);
	mpfr_clear(_bma);
}
void ezc_randgauss(mpfr_t r) { 
	mpfr_grandom(r, NULL, ezc_rand_state, EZC_RND); 
}
void ezc_sqrt(mpfr_t r, mpfr_t a) { 
	mpfr_sqrt(r, a, EZC_RND); 
}
void ezc_cbrt(mpfr_t r, mpfr_t a) { 
	mpfr_cbrt(r, a, EZC_RND); 
}
void ezc_hypot(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_hypot(r, a, b, EZC_RND); 
}
void ezc_exp(mpfr_t r, mpfr_t a) { 
	mpfr_exp(r, a, EZC_RND); 
}
void ezc_log(mpfr_t r, mpfr_t a) { 
	mpfr_log(r, a, EZC_RND); 
}
void ezc_logb(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __flogb_tmp; mpfr_init(__flogb_tmp);
	ezc_log(__flogb_tmp, b);
	ezc_log(r, a);
	ezc_div(r, r, __flogb_tmp);
	mpfr_clear(__flogb_tmp);
}
void ezc_agm(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_agm(r, a, b, EZC_RND);
}
void ezc_gamma(mpfr_t r, mpfr_t a) { 
	mpfr_gamma(r, a, EZC_RND); 
}
void ezc_fact(mpfr_t r, mpfr_t a) { 
	mpfr_add_ui(r, a, 1, EZC_RND);
	ezc_gamma(r, r);
}
void ezc_zeta(mpfr_t r, mpfr_t a) {
	mpfr_zeta(r, a, EZC_RND);
}
void ezc_echo(char msg[]) { 
	printf("%s\\n", msg); 
}
void ezc_echoraw(char msg[]) { 
	printf("%s", msg); 
}
void ezc_varb(mpfr_t a, int base) {
	if (base < 2) base = 2;
	if (base > 62) base = 62; 
	printf(" ");
	mpfr_out_str(stdout, base, 0, a, EZC_RND);
	printf(" ");
}
void ezc_var(mpfr_t a) { 
	mpfr_printf("%.*Rf\\n",ezc_prec, a); 
}
void ezc_intvar(mpfr_t a) { 
	mpz_t _t_; mpz_init(_t_);
	mpfr_get_z(_t_, a, GMP_RNDN);
	mpfr_printf("%Zd\\n", _t_); 
}

void ezc_file(mpfr_t a, char name[]) { 
	FILE *fp = fopen(name, "w+");
	mpfr_fprintf(fp, "%.*Rf ", ezc_prec, a); 
	fclose(fp);
}

void ezc_pi(mpfr_t r) {
	mpfr_const_pi(r, EZC_RND);
}

int get_use_deg() {
	char *x = getenv("EZC_DEG");
	if (x == NULL) return 0;
	if ((strcmp(x, "no") == 0) || (strcmp(x, "NO") == 0) || (strcmp(x, "0") == 0) || (strcmp(x, "-1") == 0)) return 0;
	return 1;
}

void ezc_deg(mpfr_t r, mpfr_t a) {
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_const_pi(__tmp, EZC_RND);
	mpfr_mul_ui(r, a, 180, EZC_RND);
	mpfr_div(r, r, __tmp, EZC_RND);

	mpfr_clear(__tmp);
}
void ezc_rad(mpfr_t r, mpfr_t a) {
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_const_pi(__tmp, EZC_RND);
	mpfr_mul(r, a, __tmp, EZC_RND);
	mpfr_div_ui(r, r, 180, EZC_RND);

	mpfr_clear(__tmp);
}
void _TIN(mpfr_t r, mpfr_t a) {
	if (get_use_deg()) { ezc_rad(r, a); } else { ezc_set(r, a); }
}
void ezc_sin(mpfr_t r, mpfr_t a) {
	_TIN(r, a); 
	mpfr_sin(r, r, EZC_RND); 
}
void ezc_csc(mpfr_t r, mpfr_t a) { 
	_TIN(r, a); 
	mpfr_csc(r, r, EZC_RND); 
}
void ezc_cos(mpfr_t r, mpfr_t a) { 
	_TIN(r, a); 
	mpfr_cos(r, r, EZC_RND); 
}
void ezc_sec(mpfr_t r, mpfr_t a) { 
	_TIN(r, a); 
	mpfr_sec(r, r, EZC_RND); 
}
void ezc_tan(mpfr_t r, mpfr_t a) { 
	_TIN(r, a); 
	mpfr_tan(r, r, EZC_RND); 
}
void ezc_cot(mpfr_t r, mpfr_t a) { 
	_TIN(r, a); 
	mpfr_cot(r, r, EZC_RND); 
}

void ezc_asin(mpfr_t r, mpfr_t a) { 
	mpfr_asin(r, a, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_acsc(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_asin(r, r, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_acos(mpfr_t r, mpfr_t a) { 
	mpfr_acos(r, a, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_asec(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_acos(r, r, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_atan(mpfr_t r, mpfr_t a) { 
	mpfr_atan(r, a, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_atan2(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_atan2(r, a, b, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}
void ezc_acot(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_atan(r, r, EZC_RND); 
	if (get_use_deg()) ezc_deg(r, r); 
}

void ezc_sinh(mpfr_t r, mpfr_t a) {
	mpfr_sinh(r, a, EZC_RND);
}
void ezc_csch(mpfr_t r, mpfr_t a) {
	mpfr_csch(r, a, EZC_RND);
}
void ezc_cosh(mpfr_t r, mpfr_t a) {
	mpfr_cosh(r, a, EZC_RND);
}
void ezc_sech(mpfr_t r, mpfr_t a) {
	mpfr_sech(r, a, EZC_RND);
}
void ezc_htanh(mpfr_t r, mpfr_t a) {
	mpfr_tanh(r, a, EZC_RND);
}
void ezc_hcoth(mpfr_t r, mpfr_t a) {
	mpfr_coth(r, a, EZC_RND);
}


void ezc_asinh(mpfr_t r, mpfr_t a) {
	mpfr_asinh(r, a, EZC_RND);
	mpfr_div_ui(r, a, 180, EZC_RND);
}
void ezc_acsch(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_asinh(r, r, EZC_RND);
}
void ezc_acosh(mpfr_t r, mpfr_t a) {
	mpfr_acosh(r, a, EZC_RND);
}
void ezc_asech(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_acosh(r, r, EZC_RND);
}
void ezc_atanh(mpfr_t r, mpfr_t a) {
	mpfr_atanh(r, a, EZC_RND);
}
void ezc_acoth(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_atanh(r, r, EZC_RND);
}
"""