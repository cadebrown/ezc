#include "helper.h"
#include "ezc_generic.h"

long long str_startswith(char *str, char *val, long long offset) {
    long long i = 0;
    while (i < strlen(val)) {
		if (str[i + offset] != val[i]) return 0;
		i++;
    }
    return 1;
}

long long next_valid(long long val, long long num) {
	while (num > 0) {
    	while (MEETS_FLAG(GET_F(val), FLAG_POINT)) {
			val -= 1;
    	}
		num--;
		val--;
	}
    return val;
}