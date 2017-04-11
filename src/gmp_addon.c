/* gmp_addon.c -- file that (optionally) implements functions for
               -- multiprecision objects

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

#include "ezc.h"

#ifdef USE_GMP

#define __TYPE_OP_MAP(type, type_enum, r, a, b, OP)                                                                    \
  type *__tmp_set = (type *)malloc(sizeof(type));                                                                      \
  (*__tmp_set)=((*(type *)(*a).val) OP (*(type *)(*b).val));                                                           \
  SET_OBJ(r, type_enum, (__tmp_set) );

#define __MP_FUNC_MAP(r, a, b, func) func((*r).val, (*a).val, (*b).val)
#define __MPZ_CLEAR(r) mpz_clear((*r).val);
#define __MPF_CLEAR(r) mpf_clear((*r).val);
#define __MPFR_CLEAR(r) mpfr_clear((*r).val);

#define __MP_R_FUNC_MAP(r, a, b, func) func((*r).val, (*a).val, (*b).val, EZC_RND)


void eval_func__mpz(EZC_STR op) {
	if (STR_EQ(op, "+")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		//mpz_set_ui((*a).val, 1);
		__MP_FUNC_MAP(a, a, b, mpz_add);
		__MPZ_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "-")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpz_sub);
		__MPZ_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "*")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpz_mul);
		__MPZ_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "/")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpz_div);
		__MPZ_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else {
		ERR_STR(out);
		sprintf(out, "Operator %s undefined for mpz", op);
		ezc_fail(out);
	}
}

void eval_func__mpf(EZC_STR op) {
	if (STR_EQ(op, "+")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		//mpz_set_ui((*a).val, 1);
		__MP_FUNC_MAP(a, a, b, mpf_add);
		__MPF_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "-")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpf_sub);
		__MPF_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "*")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpf_mul);
		__MPF_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "/")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__MP_FUNC_MAP(a, a, b, mpf_div);
		__MPF_CLEAR(b);
		stk_push(LAST_STACK, a);
	} else {
		ERR_STR(out);
		sprintf(out, "Operator %s undefined for mpf", op);
		ezc_fail(out);
	}
}


#ifdef USE_MPFR
	void eval_func__mpfr(EZC_STR op) {
		if (STR_EQ(op, "+")) {
			EZC_OBJ b = stk_pop(LAST_STACK);
			EZC_OBJ a = stk_pop(LAST_STACK);
			//mpz_set_ui((*a).val, 1);
			__MP_R_FUNC_MAP(a, a, b, mpfr_add);
			__MPFR_CLEAR(b);
			stk_push(LAST_STACK, a);
		} else if (STR_EQ(op, "-")) {
			EZC_OBJ b = stk_pop(LAST_STACK);
			EZC_OBJ a = stk_pop(LAST_STACK);
			__MP_R_FUNC_MAP(a, a, b, mpfr_sub);
			__MPFR_CLEAR(b);
			stk_push(LAST_STACK, a);
		} else if (STR_EQ(op, "*")) {
			EZC_OBJ b = stk_pop(LAST_STACK);
			EZC_OBJ a = stk_pop(LAST_STACK);
			__MP_R_FUNC_MAP(a, a, b, mpfr_mul);
			__MPFR_CLEAR(b);
			stk_push(LAST_STACK, a);
		} else if (STR_EQ(op, "/")) {
			EZC_OBJ b = stk_pop(LAST_STACK);
			EZC_OBJ a = stk_pop(LAST_STACK);
			__MP_R_FUNC_MAP(a, a, b, mpfr_div);
			__MPFR_CLEAR(b);
			stk_push(LAST_STACK, a);
		} else if (STR_EQ(op, "^")) {
			EZC_OBJ b = stk_pop(LAST_STACK);
			EZC_OBJ a = stk_pop(LAST_STACK);
			__MP_R_FUNC_MAP(a, a, b, mpfr_pow);
			__MPFR_CLEAR(b);
			stk_push(LAST_STACK, a);
		} else {
			ERR_STR(out);
			sprintf(out, "Function %s undefined for mpfr", op);
			ezc_fail(out);
		}
	}

#endif


#endif
