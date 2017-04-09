
#include "ezc.h"

#ifdef USE_GMP

#define __MP_FUNC_MAP(r, a, b, func) func((*r).val, (*a).val, (*b).val)
#define __MPZ_CLEAR(r) mpz_clear((*r).val);

void eval_op__mpz(EZC_STR op) {
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


#endif
