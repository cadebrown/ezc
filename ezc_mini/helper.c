#include "helper.h"
#include "ezc.h"

#include "ezc_generic.h"

long long str_startswith(char *str, char *val, long long offset) {
    long long i = 0;
    while (i < strlen(val)) {
		if (str[i + offset] != val[i]) return 0;
		i++;
    }
    return 1;
}

long long next_valid(EZC_STACK stk, EZC_IDX val, EZC_INT num) {
	while (num > 0) {
    	while (MEETS_FLAG(GET_F(stk, val), FLAG_POINT)) {
			val--;
    	}
		num--;
		val--;
	}
    return val;
}