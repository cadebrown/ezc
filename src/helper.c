
#include "ezc.h"


long long str_startswith(char *str, char *val, long long offset) {
    long long i = 0;
    while (i < strlen(val)) {
		  if (str[i + offset] != val[i]) return 0;
      i++;
    }
    return 1;
}

void ret_operator(char *out, char *val, long long *start) {
    long long i = 0;
    if (STR_STARTS(val, "==", (*start))) {
        out[i++] = '=';
        out[i++] = '=';
    } else if (STR_STARTS(val, "<<", (*start))) {
        out[i++] = '<';
        out[i++] = '<';
    } else if (STR_STARTS(val, ">>", (*start))) {
        out[i++] = '>';
        out[i++] = '>';
    } else if (IS_OP(val, (*start))) {
        out[i++] = val[(*start)];
    } else {
        ERR_STR(out);
		sprintf(out, "Unknown operator: %c", val[*start]);
		ezc_fail(out);
    }
    out[i] = 0;
    (*start) += strlen(out);
}
