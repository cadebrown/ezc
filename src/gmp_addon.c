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
