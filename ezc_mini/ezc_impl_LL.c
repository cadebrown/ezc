#include "ezc.h"
#include "ezc_generic.h"

void __int_function(char *func) {
	/**/ if (STR_EQ(func, "sqrt")) {
		__int_sqrt(NEXT(ptr, 0), NEXT(ptr, 0));
	}
}

void __int_op(char *op) {
	if (STR_EQ(op, "_")) {
		gen_push_int(ptr);
	} else if (STR_EQ(op, "$")) {
		EZC_IDX p0 = NEXT(ptr, 0);
		GET(EZC_INT, p0) = GET(EZC_INT, GET(EZC_INT, p0));
	} else if (STR_EQ(op, "<<")) {
		move_ahead(-1);
	} else if (STR_EQ(op, ">>")) {
		move_ahead(1);
	} else {
		EZC_IDX p0 = NEXT(ptr, 1), p1 = NEXT(ptr, 0);
		if (p0 < 0) {
			globalstop = 1;
			return;
		}
		/**/ if (STR_EQ(op, "+")) __int_add(p0, p0, p1);
		else if (STR_EQ(op, "-")) __int_sub(p0, p0, p1);
		else if (STR_EQ(op, "*")) __int_mul(p0, p0, p1);
		else if (STR_EQ(op, "/")) __int_div(p0, p0, p1);
		else if (STR_EQ(op, "%")) __int_mod(p0, p0, p1);
		else if (STR_EQ(op, "^")) __int_pow(p0, p0, p1);
		else if (STR_EQ(op, "<")) __int_lt (p0, p0, p1);
		else if (STR_EQ(op, ">")) __int_gt (p0, p0, p1);
		else if (STR_EQ(op, "==")) __int_et (p0, p0, p1);
		move_ahead(-1);
	}
}


void __int_push(EZC_INT val) {
	move_ahead(1);
	RECENT(EZC_INT, 0) = val;
	RECENT_F(0) = FLAG_DEFAULT;
	RECENT_T(0) = TYPE_INT;
}

void __int_reset(EZC_IDX ret) {
	GET(EZC_INT, ret) = 0;
	GET_F(ret) = FLAG_DEFAULT;
	GET_T(ret) = TYPE_INT;
}

void __int_from_str(EZC_IDX ret, char *val) {
	GET(EZC_INT, ret) = strtoll(val, NULL, 10);
	GET_F(ret) = FLAG_DEFAULT;
	GET_T(ret) = TYPE_INT;
}

void __int_to_str(char *ret, EZC_IDX val) {
	sprintf(ret, "%lld", GET(EZC_INT, val));
}

void __int_print(EZC_IDX pos) {
	printf("%lld", GET(EZC_INT, pos));
}


void __int_gt (EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) > GET(EZC_INT, p1); }
void __int_lt (EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) < GET(EZC_INT, p1); }
void __int_et (EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) == GET(EZC_INT, p1); }

void __int_add(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) + GET(EZC_INT, p1); }
void __int_sub(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) - GET(EZC_INT, p1); }
void __int_mul(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) * GET(EZC_INT, p1); }
void __int_div(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) / GET(EZC_INT, p1); }

void __int_mod(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) { GET(EZC_INT, ret) = GET(EZC_INT, p0) % GET(EZC_INT, p1); }
void __int_pow(EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) {
	EZC_INT _ret = 1, _p0 = GET(EZC_INT, p0), _p1 = GET(EZC_INT, p1);
    while (_p1) {
		if (_p1 & 1) {
			_ret *= _p0;
		}
		_p0 *= _p0;
		_p1 >>= 1;
	}
	GET(EZC_INT, ret) = _ret;
}

void __int_sqrt(EZC_IDX ret, EZC_IDX p0) {
	EZC_INT _a = GET(EZC_INT, ret), _ret = GET(EZC_INT, p0);
	EZC_INT _times = 31;
	_ret = _a;
    while (_times > 0) {
		_ret = (_a / _ret + _ret) / 2;
		_times--;
    }
	GET(EZC_INT, ret) = _ret;
}
