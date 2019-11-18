
#include "ezc-impl.h"

void ezc_str_init(ezc_str* str) {
    *str = EZC_STR_NULL;
}

void ezc_str_from(ezc_str* str, char* charp, int len) {
    if (charp == NULL || (len <= 0 && (len = strlen(charp)) <= 0)) {
        *str = EZC_STR_NULL;
    } else {
        str->len = len;
        str->_ = ezc_realloc(str->_, len + 1);
        
        strncpy(str->_, charp, len);
        str->_[len] = '\0';
    }
}

void ezc_str_copy(ezc_str* str, ezc_str from) {
    ezc_str_from(str, from._, from.len);
}

void ezc_str_concat(ezc_str* str, ezc_str A, ezc_str B) {
    str->len = A.len + B.len;
    str->_ = ezc_realloc(str->_, str->len+1);
    strncpy(str->_, A._, A.len);
    strncpy(str->_+A.len, B._, B.len);
    str->_[str->len] = '\0';
}

void ezc_str_concatc(ezc_str* str, char c) {
    str->len++;
    str->_ = ezc_realloc(str->_, str->len + 1);
    str->_[str->len-1] = c;
    str->_[str->len] = '\0';
}

int ezc_str_cmp(ezc_str A, ezc_str B) {
    return (A.len == B.len) ? strncmp(A._, B._, A.len) : A.len - B.len;
}

void ezc_str_free(ezc_str* str) {
    ezc_free(str->_);

    // reset it
    *str = EZC_STR_NULL;
}

