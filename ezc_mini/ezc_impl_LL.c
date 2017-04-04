#include "ezc.h"
#include "helper.h"
#include "ezc_generic.h"

void __int_function(EZC_STACK stk, char *func) {
	/**/ if (STR_EQ(func, "sqrt")) {
		__int_sqrt(stk, NEXT(stk, (*stk).ptr, 0), NEXT(stk, (*stk).ptr, 0));
	}
}

void __int_op(EZC_STACK stk, char *op) {
	if (STR_EQ(op, "_")) {
		__int_push(stk, (*stk).ptr);
	} else if (STR_EQ(op, "$")) {
		EZC_IDX p0 = NEXT(stk, (*stk).ptr, 0);
		GET(stk, EZC_INT, p0) = GET(stk, EZC_INT, GET(stk, EZC_INT, p0));
	} else if (STR_EQ(op, "<<")) {
		move_ahead(stk, -1);
	} else if (STR_EQ(op, ">>")) {
		move_ahead(stk, 1);
	} else {
		EZC_IDX p0 = NEXT(stk, (*stk).ptr, 1), p1 = NEXT(stk, (*stk).ptr, 0);
		if (p0 < 0 || GET_F(stk, p0) == FLAG_STOP || GET_F(stk, p1) == FLAG_STOP) {
			globalstop = 1;
			return;
		}
		/**/ if (STR_EQ(op, "+")) __int_add(stk, p0, p0, p1);
		else if (STR_EQ(op, "-")) __int_sub(stk, p0, p0, p1);
		else if (STR_EQ(op, "*")) __int_mul(stk, p0, p0, p1);
		else if (STR_EQ(op, "/")) __int_div(stk, p0, p0, p1);
		else if (STR_EQ(op, "%")) __int_mod(stk, p0, p0, p1);
		else if (STR_EQ(op, "^")) __int_pow(stk, p0, p0, p1);
		else if (STR_EQ(op, "<")) __int_lt (stk, p0, p0, p1);
		else if (STR_EQ(op, ">")) __int_gt (stk, p0, p0, p1);
		else if (STR_EQ(op, "==")) __int_et(stk, p0, p0, p1);
		move_ahead(stk, -1);
	}
}


void __int_push(EZC_STACK stk, EZC_INT val) {
	move_ahead(stk, 1);
	RECENT(stk, EZC_INT, 0) = val;
	RECENT_F(stk, 0) = FLAG_DEFAULT;
	RECENT_T(stk, 0) = TYPE_INT;
}

void __int_reset(EZC_STACK stk, EZC_IDX ret) {
	GET(stk, EZC_INT, ret) = 0;
	GET_F(stk, ret) = FLAG_DEFAULT;
	GET_T(stk, ret) = TYPE_INT;
}

void __int_from_str(EZC_STACK stk, EZC_IDX ret, char *val) {
	GET(stk, EZC_INT, ret) = strtoll(val, NULL, 10);
	GET_F(stk, ret) = FLAG_DEFAULT;
	GET_T(stk, ret) = TYPE_INT;
}

void __int_to_str(EZC_STACK stk, char *ret, EZC_IDX val) {
	sprintf(ret, "%lld", GET(stk, EZC_INT, val));
}

void __int_print(EZC_STACK stk, EZC_IDX pos) {
	printf("%lld", GET(stk, EZC_INT, pos));
}

#define OPST(stk, op, ret, p0, p1) GET(stk, EZC_INT, ret) = GET(stk, EZC_INT, p0) op GET(stk, EZC_INT, p1); 

void __int_gt (EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, >, ret, p0, p1);
}
void __int_lt (EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, <, ret, p0, p1);
}
void __int_et (EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, ==, ret, p0, p1);
}

void __int_add(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, +, ret, p0, p1);
}
void __int_sub(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) {
	OPST(stk, -, ret, p0, p1);	 
}
void __int_mul(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) {
	OPST(stk, *, ret, p0, p1);
}
void __int_div(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, /, ret, p0, p1);	 
}
void __int_mod(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { 
	OPST(stk, %, ret, p0, p1);	 
}
void __int_pow(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) {
	EZC_INT _ret = 1, _p0 = GET(stk, EZC_INT, p0), _p1 = GET(stk, EZC_INT, p1);
    while (_p1) {
		if (_p1 & 1) {
			_ret *= _p0;
		}
		_p0 *= _p0;
		_p1 >>= 1;
	}
	GET(stk, EZC_INT, ret) = _ret;
}

void __int_sqrt(EZC_STACK stk, EZC_IDX ret, EZC_IDX p0) {
	EZC_INT _a = GET(stk, EZC_INT, ret), _ret = GET(stk, EZC_INT, p0);
	EZC_INT _times = 31;
	_ret = _a;
    while (_times > 0) {
		_ret = (_a / _ret + _ret) / 2;
		_times--;
    }
	GET(stk, EZC_INT, ret) = _ret;
}
