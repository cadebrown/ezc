
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
    }
    out[i] = 0;
    (*start) += strlen(out);
}

void ret_str(char *out, char *code, long long *start) {
    long long i = (*start);
    if (code[i] != '"') {
        printf("Error, called 'ret_str' with something that didn't start with \"");
        return;
    } else {
        i++;
    }
    while (IS_ALPHA(code[i])) {
        out[i-(*start)] = code[i];
        i++;
    }
    out[i-(*start)] = 0;

    if (code[i] != '"') {
        printf("Error, called 'ret_str' with something that didn't end with \"");
        return;
    } else {
        i++;
    }
    (*start) = i;
}