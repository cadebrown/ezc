
#include "ezc-impl.h"

void* ezc_malloc(size_t sz) {
    return malloc(sz);
}

void ezc_free(void* ptr) {
    if (ptr != NULL) free(ptr);
}

void* ezc_realloc(void* ptr, size_t sz) {
    return realloc(ptr, sz);
}

void ezc_memcpy(void* dst, void* src, size_t sz) {
    memcpy(dst, src, sz);
}

