#include "ezc.h"
#include "ezc_generic.h"

void reset_val(EZC_STACK_TYPE *ret) {
	(*ret) = 0;
}

void from_str(EZC_STACK_TYPE *ret, EZC_FLAG_TYPE *flags, char *val) {
	(*ret) = strtoll(val, NULL, 10);
	(*flags) = EZC_CONST_FLAGS;
}
void to_str(char *ret, EZC_STACK_TYPE val, EZC_FLAG_TYPE flags) {
	if (MEETS_FLAG(flags, EZC_SPECIAL_STOP_FLAGS)) {
		sprintf(ret, "|");
	} else if (MEETS_FLAG(flags, EZC_SPECIAL_POINT_FLAGS)) {
		sprintf(ret, "#");

	} else {
		sprintf(ret, "%lld", val);
	}
	
}

void print_single(FILE *stream, EZC_STACK_TYPE val, EZC_FLAG_TYPE flags) {
	if (MEETS_FLAG(flags, EZC_SPECIAL_STOP_FLAGS)) {
		fprintf(stream, "|");
	} else if (MEETS_FLAG(flags, EZC_SPECIAL_POINT_FLAGS)) {
		fprintf(stream, "#");
	} else {
		fprintf(stream, "%lld", val);
	}
}

void __gt(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 > op1; }
void __lt(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 < op1; }

void __swap(EZC_STACK_TYPE *op0, EZC_STACK_TYPE *op1) {
	EZC_STACK_TYPE __tmp_var = (*op0);
	(*op0) = (*op1);
	(*op1) = __tmp_var;
}

void __add(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 + op1; }
void __sub(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 - op1; }
void __mul(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 * op1; }
void __div(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 / op1; }
void __mod(EZC_STACK_TYPE *ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) { (*ret) = op0 % op1; }

void __pow(EZC_STACK_TYPE *_ret, EZC_STACK_TYPE op0, EZC_STACK_TYPE op1) {
	EZC_STACK_TYPE ret = 1;
    while (op1) {
		if (op1 & 1) {
			ret *= op0;
		}
		op0 *= op0;
		op1 >>= 1;
	}
	(*_ret) = ret;
}

void __sqrt(EZC_STACK_TYPE *_ret, EZC_STACK_TYPE op0) {
	EZC_STACK_TYPE ret = op0>>1, lret = -1;
    while ((ret = ret/2 + op0/(2 * ret)) != lret && ret != lret + 1) {
        lret = ret;
    }
	(*_ret) = (ret * ret > op0) ? lret : ret;
}
