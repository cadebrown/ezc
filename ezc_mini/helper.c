#include "helper.h"

long long str_startswith(char *str, char *val, long long offset) {
	long long i = 0;
	while (i < strlen(val)) {
		if (str[i+offset] != val[i]) return 0;
		i++;
	}
	return 1;
}

