#include "ezc.h"

#define __INT_OP_MAP(r, a, b, OP) SET_OBJ(r, TYPE_INT, \
	((EZC_INT)(*a).val OP (EZC_INT)(*b).val) \
);

#define __INT_FUNC_MAP(r, a, b, func) SET_OBJ(r, TYPE_INT, \
	func((EZC_INT)(*a).val, (EZC_INT)(*b).val) \
);


void eval_func(EZC_DICT dict, EZC_STACK stk, EZC_STR func) {
	if (STR_EQ(func, "p") || STR_EQ(func, "print")) {
		obj_dump_fmt(stk_get_recent(stk, 0), true);
		printf("\n");
	} else if (STR_EQ(func, "s") || STR_EQ(func, "stk")) {
		stk_dump(stk);
	} else if (STR_EQ(func, "d") || STR_EQ(func, "dict")) {
		dict_dump(dict);
	} else {
		printf("Error in function: don't know the name of function: :%s:.\n", func);
	}
}

void eval_op(EZC_DICT dict, EZC_STACK stk, EZC_STR op) {
	if (IS_1OP(op, 0)) {
		EZC_TYPE type = (*stk_get_recent(stk, 0)).type;
		if (type == TYPE_INT) {
			eval_op__int(dict, stk, op);
		}
	} else if (IS_2OP(op, 0)) {
		EZC_TYPE type0 = (*stk_get_recent(stk, 0)).type;
		EZC_TYPE type1 = (*stk_get_recent(stk, 1)).type;
		if (type0 == type1) {
			if (type0 == TYPE_INT) {
				eval_op__int(dict, stk, op);
			} else if (type0 == TYPE_STR) {
				eval_op__str(dict, stk, op);
			}
			#ifdef USE_GMP
				else if (type0 == TYPE_MPZ) {
					eval_op__mpz(dict, stk, op);
				}
			#endif
			else {
				printf("Error in operator: not defined for this type.\n");
			}
		} else {
			printf("Error in operator: different types.\n");
		}
	} else {
		printf("Error in operator: don't know the type of operator.\n");
	}
	
}


void eval_op__str(EZC_DICT dict, EZC_STACK stk, EZC_STR op) {
	if (STR_STARTS(op, "+", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_STR, malloc(strlen((*a).val) + strlen((*b).val)));
		strcpy((*ret).val, (*a).val);
		strcat((*ret).val, (*b).val);
		free((*a).val);
		free((*b).val);
		stk_push(stk, ret);
	} else {
		printf("Error: undefined str operation: %s\n", op);
	}
}

void eval_op__int(EZC_DICT dict, EZC_STACK stk, EZC_STR op) {
	if (STR_STARTS(op, "+", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__INT_OP_MAP(a, a, b, +);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "-", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__INT_OP_MAP(a, a, b, -);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "*", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__INT_OP_MAP(a, a, b, *);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "/", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__INT_OP_MAP(a, a, b, /);
		stk_push(stk, a);
	} else if (STR_STARTS(op, "^", 0)) {
		EZC_OBJ b = stk_pop(stk);
		EZC_OBJ a = stk_pop(stk);
		__INT_FUNC_MAP(a, a, b, __int_pow);
		stk_push(stk, a);
	} else {
		printf("Error: undefined int operation: %s\n", op);
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

