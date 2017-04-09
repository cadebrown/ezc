
#include "ezc.h"

#ifdef USE_GMP

#define __MP_FUNC_MAP(r, a, b, func) func((*r).val, (*a).val, (*b).val)
#define __MPZ_CLEAR(r) mpz_clear((*r).val);

void eval_op__mpz(EZC_DICT dict, EZC_STACK stk, EZC_STR op) {
	if (STR_STARTS(op, "+", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		//mpz_set_ui((*a).val, 1);
		__MP_FUNC_MAP(a, a, b, mpz_add);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "-", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpz_sub);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "*", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpz_mul);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "/", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpz_div);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else {
		printf("Error: undefined mpz operation: %s\n", op);
	}
}



void eval_op__mpf(EZC_DICT dict, EZC_STACK stk, EZC_STR op) {
	if (STR_STARTS(op, "+", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpf_add);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "-", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpf_sub);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "*", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpf_mul);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "/", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__MP_FUNC_MAP(a, a, b, mpf_div);
		__MPZ_CLEAR(b);
		stk_push(stk, a);
	} else {
		printf("Error: undefined mpf operation: %s\n", op);
	}
}

#endif
