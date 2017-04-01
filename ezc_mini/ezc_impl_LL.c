#include "ezc.h"
#include "ezc_generic.h"

void __int_op_1(char *op, EZC_IDX ret, EZC_IDX p0) {
	/**/ if (STR_EQ(op, "$")) GET(EZC_INT, ret) = GET(EZC_INT, GET(EZC_INT, p0));
}
void __int_op_2(char *op, EZC_IDX ret, EZC_IDX p0, EZC_IDX p1) {
	/**/ if (STR_EQ(op, "+")) __int_add(ret, p0, p1);
	else if (STR_EQ(op, "-")) __int_sub(ret, p0, p1);
	else if (STR_EQ(op, "*")) __int_mul(ret, p0, p1);
	else if (STR_EQ(op, "/")) __int_div(ret, p0, p1);
	else if (STR_EQ(op, "%")) __int_mod(ret, p0, p1);
	else if (STR_EQ(op, "^")) __int_pow(ret, p0, p1);
	else if (STR_EQ(op, ">")) __int_lt (ret, p0, p1);
	else if (STR_EQ(op, "<")) __int_gt (ret, p0, p1);
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
	EZC_INT _a = GET(EZC_INT, p0), _ret = GET(EZC_INT, p0), __ret = -1;
    while ((_ret = _ret/2 + _a/(2 * _ret)) != __ret && _ret != __ret + 1) {
        __ret = _ret;
    }
	GET(EZC_INT, ret) = (_ret * _ret > GET(EZC_INT, p0)) ? __ret : _ret;
}
