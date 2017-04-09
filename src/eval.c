/* eval.c -- evaluating functions, operators, etc and matching obj types

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

#define __INT_OP_MAP(r, a, b, OP) SET_OBJ(r, TYPE_INT, \
	((EZC_INT)(*a).val OP (EZC_INT)(*b).val) \
);

#define __INT_FUNC_MAP(r, a, b, func) SET_OBJ(r, TYPE_INT, \
	func((EZC_INT)(*a).val, (EZC_INT)(*b).val) \
);


void eval_func(EZC_STR func) {

	if (STR_EQ(func, "q") || STR_EQ(func, "quit")) {
		ezc_end();
		exit (0);
		return;
	} else if (STR_EQ(func, "e") || STR_EQ(func, "eval")) {
		EZC_OBJ code_to_run = stk_pop(LAST_STACK);
		exec((char *)(*code_to_run).val);
	} else if (STR_EQ(func, "p") || STR_EQ(func, "print")) {
		obj_dump_fmt(stk_get_recent(LAST_STACK, 0), 10, 0, 2, true, true);
		printf("\n");
	} else if (STR_EQ(func, "s") || STR_EQ(func, "stk")) {
		obj_dump_fmt(stk_get_recent((*stacks).val, 0), 10, 0, 2, true, true);
		printf("\n");
	} else if (STR_EQ(func, "d") || STR_EQ(func, "dict")) {
		obj_dump((*dicts).val);
		printf("\n");
	} else {
		ERR_STR(out);
		sprintf(out, "Function not defined: '%s'", func);
		ezc_fail(out);
	}
}

void eval_op(EZC_STR op) {
	if (IS_1OP(op, 0)) {
		EZC_TYPE type = (*stk_get_recent(LAST_STACK, 0)).type;
		if (type == TYPE_INT) {
			eval_op__int(op);
		}
	} else if (IS_2OP(op, 0)) {
		EZC_TYPE type0 = (*stk_get_recent(LAST_STACK, 0)).type;
		EZC_TYPE type1 = (*stk_get_recent(LAST_STACK, 1)).type;
		if (type0 == type1) {
			if (type0 == TYPE_INT) {
				eval_op__int(op);
			} else if (type0 == TYPE_STR) {
				eval_op__str(op);
			}
			#ifdef USE_GMP
				else if (type0 == TYPE_MPZ) {
					eval_op__mpz(op);
				}
			#endif
			else {
				ERR_STR(out);
				sprintf(out, "Operator %s not defined for type: %s", op, get_type_name(type0));
				ezc_fail(out);
			}
		} else {
			ERR_STR(out);
			sprintf(out, "Operator %s being applied to two different types: %s and %s", op, get_type_name(type0), get_type_name(type1));
			ezc_fail(out);
		}
	} else {
		ERR_STR(out);
		sprintf(out, "Unknown operator: %s", op);
		ezc_fail(out);
	}
	
}


void eval_op__str(EZC_STR op) {
	if (STR_EQ(op, "+")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_STR, malloc(strlen((*a).val) + strlen((*b).val)));
		strcpy((*ret).val, (*a).val);
		strcat((*ret).val, (*b).val);
		free((*a).val);
		free((*b).val);
		stk_push(LAST_STACK, ret);
	} else {
		ERR_STR(out);
		sprintf(out, "Operator %s undefined for str", op);
		ezc_fail(out);
	}
}

void eval_op__int(EZC_STR op) {
	if (STR_EQ(op, "+")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__INT_OP_MAP(a, a, b, +);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "-")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__INT_OP_MAP(a, a, b, -);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "*")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__INT_OP_MAP(a, a, b, *);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "/")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__INT_OP_MAP(a, a, b, /);
		stk_push(LAST_STACK, a);
	} else if (STR_EQ(op, "^")) {
		EZC_OBJ b = stk_pop(LAST_STACK);
		EZC_OBJ a = stk_pop(LAST_STACK);
		__INT_FUNC_MAP(a, a, b, __int_pow);
		stk_push(LAST_STACK, a);
	} else {
		ERR_STR(out);
		sprintf(out, "Operator %s undefined for int", op);
		ezc_fail(out);
	}
}



EZC_INT __int_pow(EZC_INT a, EZC_INT b) {
	EZC_INT r = 1;
    while (b) {
		if (b & 1) {
			r *= a;
		}
		a *= a;
		b >>= 1;
	}
	return r;
}

