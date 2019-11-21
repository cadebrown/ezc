
#include "ezc-impl.h"



void ezc_str_copy_cp(ezc_str* str, char* charp, int len) {
    if (str->_ == NULL || str->max_len < len) {
        str->max_len = (int)(1.5 * len + 10);
        str->_ = ezc_realloc(str->_, str->max_len + 1);
    }
    str->len = len;
    ezc_memcpy(str->_, charp, len);
    str->_[len] = '\0';
}

void ezc_str_copy(ezc_str* str, ezc_str from) {
    ezc_str_copy_cp(str, from._, from.len);
}


void ezc_str_append(ezc_str* str, ezc_str A) {
    int start_len = str->len;
    int new_len = str->len + A.len;
    if (str->_ == NULL || str->max_len < new_len) {
        str->max_len = (int)(1.5 * new_len + 10);
        str->_ = ezc_realloc(str->_, str->max_len + 1);
    }
    str->len = new_len;
    //ezc_memcpy(str->_, A._, A.len);
    ezc_memcpy(str->_ + start_len, A._, A.len);
    str->_[new_len] = '\0';
}


void ezc_str_concat(ezc_str* str, ezc_str A, ezc_str B) {
    int new_len = A.len + B.len;
    if (str->_ == NULL || str->max_len < new_len) {
        str->max_len = (int)(1.5 * new_len + 10);
        str->_ = ezc_realloc(str->_, str->max_len + 1);
    }
    str->len = new_len;
    ezc_memcpy(str->_, A._, A.len);
    ezc_memcpy(str->_ + A.len, B._, B.len);
    str->_[new_len] = '\0';
}

void ezc_str_append_c(ezc_str* str, char c) {
    str->len++;
    if (str->_ == NULL || str->max_len < str->len) {
        str->max_len = str->len;
        str->_ = ezc_realloc(str->_, str->max_len + 1);
    }
    str->_[str->len-1] = c;
    str->_[str->len] = '\0';
}

int ezc_str_cmp(ezc_str A, ezc_str B) {
    return (A.len == B.len) ? memcmp(A._, B._, A.len) : A.len - B.len;
}

void ezc_str_free(ezc_str* str) {
    ezc_free(str->_);

    // reset it
    *str = EZC_STR_NULL;
}

