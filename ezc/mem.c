
#include "ezc-impl.h"

void* ezc_malloc(size_t sz) {
    void* ptr = malloc(sz);
    ezc_trace("ezc_malloc(%lu) -> %p", sz, ptr);
    // debug on allocations larger than
    if (sz > 1024 * 1024) {
        ezc_debug("[LARGE ] allocating %lu%s", ezc_bytesize_dig(sz), ezc_bytesize_name(sz));
    }
    if (ptr == NULL) {
        ezc_warn("[FAILED] ezc_malloc(%lu)", sz);
    }
    return ptr;
}

void ezc_free(void* ptr) {
    ezc_trace("ezc_free(%p)", ptr);
    if (ptr != NULL) free(ptr);
}

void* ezc_realloc(void* ptr, size_t sz) {
    if (ptr == NULL) return ezc_malloc(sz);
    void* new_ptr = realloc(ptr, sz);
    ezc_trace("ezc_realloc(%p, %lu) -> %p", ptr, sz, new_ptr);
    if (sz > 1e6) {
        ezc_debug("[LARGE ] realloc'ing %lu%s", ezc_bytesize_dig(sz), ezc_bytesize_name(sz));
    }
    if (new_ptr == NULL) {
        ezc_warn("[FAILED] ezc_realloc(%p, %lu)", ptr, sz);
    }

    return new_ptr;
}

void ezc_memcpy(void* dst, void* src, size_t sz) {
    memcpy(dst, src, sz);
}

