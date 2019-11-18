
#include "ezc-impl.h"

void* ezc_malloc(size_t b) {
    return malloc(b);
}

void ezc_free(void* p) {
    free(p);
}

void* ezc_realloc(void* p, size_t b) {
    return realloc(p, b);
}


