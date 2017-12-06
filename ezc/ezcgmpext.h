#ifndef __EZCGMPEXT_H__
#define __EZCGMPEXT_H__

// this basically defines the GMP/MPFR functions using the LIB_FUNC macro (provided by ezcmodule.h)

#define GMP_EXPORT_PREFIX "__g"
#define GMP_FUNC(rt, fname, ...) LIB_FUNC(libgmp, rt, GMP_EXPORT_PREFIX fname, __VA_ARGS__)

#define mpz_init GMP_FUNC(void, "mpz_init", mpz_t)
#define mpz_set GMP_FUNC(void, "mpz_set", mpz_t, mpz_t)
#define mpz_set_ui GMP_FUNC(void, "mpz_set_ui", mpz_t, int)
#define mpz_set_str GMP_FUNC(int, "mpz_set_str", mpz_t, char *, int)
#define mpz_init_set_si GMP_FUNC(void, "mpz_init_set_si", mpz_t, int)
#define mpz_get_str GMP_FUNC(char *, "mpz_get_str", char *, int, mpz_t)
#define mpz_clear GMP_FUNC(void, "mpz_clear", mpz_t)
#define mpz_get_si GMP_FUNC(int, "mpz_get_si", mpz_t)
#define mpz_add_ui GMP_FUNC(void, "mpz_sub_ui", mpz_t, mpz_t, int)
#define mpz_sub_ui GMP_FUNC(void, "mpz_add_ui", mpz_t, mpz_t, int)
#define mpz_pow_ui GMP_FUNC(void, "mpz_pow_ui", mpz_t, mpz_t, int)
#define mpz_neg GMP_FUNC(void, "mpz_neg", mpz_t, mpz_t)

#define _GMP_OP(fname, tt) GMP_FUNC(void, fname, tt, tt, tt)
#define mpz_add _GMP_OP("mpz_add", mpz_t)
#define mpz_sub _GMP_OP("mpz_sub", mpz_t)
#define mpz_mul _GMP_OP("mpz_mul", mpz_t)
#define mpz_div _GMP_OP("mpz_div", mpz_t)

#define mpf_add _GMP_OP("mpf_add", mpf_t)
#define mpf_sub _GMP_OP("mpf_sub", mpf_t)
#define mpf_mul _GMP_OP("mpf_mul", mpf_t)
#define mpf_div _GMP_OP("mpf_div", mpf_t)


#define mpf_init GMP_FUNC(void, "mpf_init", mpf_t)
#define mpf_set_str GMP_FUNC(int, "mpf_set_str", mpf_t, char *, int)
#define mpf_get_str GMP_FUNC(char *, "mpf_get_str", char *, mp_exp_t *, int, int, mpf_t)
#define mpf_init_set_d GMP_FUNC(void, "mpf_init_set_d", mpf_t, double)
#define mpf_init2 GMP_FUNC(void, "mpf_init2", mpf_t, int)
#define mpf_neg GMP_FUNC(void, "mpf_neg", mpf_t, mpf_t)
#define mpf_clear GMP_FUNC(void, "mpf_clear", mpf_t)
#define mpf_set_d GMP_FUNC(void, "mpf_set_d", mpf_t, double)
#define mpf_set_z GMP_FUNC(void, "mpf_set_z", mpf_t, mpz_t)
#define mpf_set GMP_FUNC(void, "mpf_set", mpf_t, mpf_t)
#define mpf_get_prec GMP_FUNC(int, "mpf_get_prec", mpf_t)
#define mpf_set_default_prec GMP_FUNC(void, "mpf_set_default_prec", int)
#define mpf_get_si GMP_FUNC(int, "mpf_get_si", mpf_t)
#define mpf_get_d GMP_FUNC(double, "mpf_get_d", mpf_t)
#define mpf_sub_ui GMP_FUNC(void, "mpf_sub_ui", mpf_t, mpf_t, int)
#define mpf_add_ui GMP_FUNC(void, "mpf_add_ui", mpf_t, mpf_t, int)


#define MPFR_FUNC(rt, fname, ...) LIB_FUNC(libmpfr, rt, fname, __VA_ARGS__)

#define mpfr_get_f MPFR_FUNC(void, "mpfr_get_f", mpf_t, mpfr_t, int)
#define mpfr_set_f MPFR_FUNC(void, "mpfr_set_f", mpfr_t, mpf_t, int)
#define mpfr_add_d MPFR_FUNC(void, "mpfr_add_d", mpfr_t, mpfr_t, double, int)
#define mpfr_add_si MPFR_FUNC(void, "mpfr_add_si", mpfr_t, mpfr_t, int, int)
#define mpfr_add_z MPFR_FUNC(void, "mpfr_add_z", mpfr_t, mpfr_t, mpz_t, int)
#define mpfr_get_prec MPFR_FUNC(int, "mpfr_get_prec", mpfr_t)
#define mpfr_init_set_str MPFR_FUNC(void, "mpfr_init_set_str", mpfr_t, char *, int, int)
#define mpfr_init_set_d MPFR_FUNC(void, "mpfr_init_set_d", mpfr_t, float, int)
#define mpfr_init_set_f MPFR_FUNC(void, "mpfr_init_set_f", mpfr_t, mpf_t, int)
#define mpfr_get_str MPFR_FUNC(char *, "mpfr_get_str", char *, mp_exp_t *, int, int, mpfr_t, int)
#define mpfr_clear MPFR_FUNC(void, "mpfr_clear", mpfr_t)
#define mpfr_init2 MPFR_FUNC(void, "mpfr_init2", mpfr_t, int)
#define mpfr_set MPFR_FUNC(void, "mpfr_set", mpfr_t, mpfr_t, int)
#define mpfr_set_default_prec MPFR_FUNC(void, "mpfr_set_default_prec", int)
#define mpfr_set_default_rounding_mode MPFR_FUNC(void, "mpfr_set_default_rounding_mode", int)
#define mpfr_get_si MPFR_FUNC(int, "mpfr_get_si", mpfr_t, int)
#define mpfr_get_d MPFR_FUNC(double, "mpfr_get_d", mpfr_t, int)

#define _MPFR_OP(nm) MPFR_FUNC(void, nm, mpfr_t, mpfr_t, mpfr_t, int)
#define mpfr_add _MPFR_OP("mpfr_add")
#define mpfr_sub _MPFR_OP("mpfr_sub")
#define mpfr_mul _MPFR_OP("mpfr_mul")
#define mpfr_div _MPFR_OP("mpfr_div")
#define mpfr_pow _MPFR_OP("mpfr_pow")


#endif
