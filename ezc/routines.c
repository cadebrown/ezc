
#include "routines.h"
#include <string.h>

obj_t obj_copy(obj_t input) {
    obj_t ret = input;
    ret.data = malloc(input.data_len);
    strncpy(ret.data, input.data, input.data_len);
    return ret;
}

void init_runtime(runtime_t * runtime) {
    estack_init(&runtime->stack);
    dict_init(&runtime->globals);
}

void run_code(runtime_t * runtime, char * ezc_source_code) {
    printf("running code: '%s'\n", ezc_source_code);
    
}

