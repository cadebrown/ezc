
#include "routines.h"

#include <string.h>
#include <stdio.h>

#include "estack.h"
#include "dict.h"
#include "module_loader.h"

obj_t obj_copy(obj_t input) {
    obj_t ret = input;
    ret.data = malloc(input.data_len);
    strncpy(ret.data, input.data, input.data_len);
    return ret;
}

void obj_free(obj_t * obj) {
    type_t obj_type = type_from_id(obj->type_id);
    obj_type.destroyer(obj);
}

void init_runtime(runtime_t * runtime) {
    estack_init(&runtime->stack);
    dict_init(&runtime->globals);
}

void run_code(runtime_t * runtime, char * ezc_source_code) {
    char * tmp = malloc(strlen(ezc_source_code));
    
    int c_off = 0;
    int c_obj_off;


    obj_t str_obj;
    type_t str_type;

    if (type_exists_name("str")) {
        str_type = type_from_name("str");
    } else {
        printf("no 'str' type found\n");
        return;
    }

    #define IS_SPECIAL(c) (c == ':' || c == '!')
    #define IS_QUOTE(c) (c == '\'' || c == '"')


    while (c_off < strlen(ezc_source_code)) {

        // cast using `:TYPE`
        if (ezc_source_code[c_off] == ':') {
            // tmp will be equal to TYPE
            c_off++;
            c_obj_off = c_off;
            while (c_off < strlen(ezc_source_code) && ezc_source_code[c_off] != ' ' && !IS_SPECIAL(ezc_source_code[c_off])) {
                tmp[c_off - c_obj_off] = ezc_source_code[c_off];
                c_off++;
            }
            tmp[c_off - c_obj_off] = 0;

            type_t to_type;
            if (type_exists_name(tmp)) {
                to_type = type_from_name(tmp);
            } else {
                printf("error: could not find type '%s'\n", tmp);
                return;
            }

            if (runtime->stack.len <= 0) {
                printf ("error (while casting): no objects on stack\n");
                return;
            } else {
                obj_t last_on_stack = estack_pop(&runtime->stack);
                obj_t new;
                to_type.constructor(&new, last_on_stack);
                new.type_id = to_type.id;
                estack_push(&runtime->stack, new);
            }
        
        } else if (ezc_source_code[c_off] == '!') {
            if (runtime->stack.len <= 0) {
                printf ("no objects on the stack\n");
                return;
            }
            obj_t last_on_stack = estack_pop(&runtime->stack);
            if (last_on_stack.type_id != str_type.id) {
                printf ("error (while finding function to execute): last item was not a str\n");
                return;
            }

            char * func_name = last_on_stack.data;
            if (!function_exists_name(func_name)) {
                printf("error: could not find function: %s\n", func_name);
                return;
            }

            function_t cur_func = function_from_name(func_name);

            cur_func.function(runtime);

            c_off++;

        } else {
            // this section is for adding on a (string object) to the stack
            
            if (IS_QUOTE(ezc_source_code[c_off])) {
                char which_quote_used = ezc_source_code[c_off];
                c_off++;
                // handle escapes
                c_obj_off = c_off;
                while (c_off < strlen(ezc_source_code) && ezc_source_code[c_off] != which_quote_used && ezc_source_code[c_off - 1] !=  '\\') {
                    tmp[c_off - c_obj_off] = ezc_source_code[c_off];
                    c_off++;
                }
                c_off++;
                tmp[c_off - c_obj_off] = 0;
            } else {
                c_obj_off = c_off;
                while (c_off < strlen(ezc_source_code) && ezc_source_code[c_off] != ' ' && !IS_SPECIAL(ezc_source_code[c_off])) {
                    tmp[c_off - c_obj_off] = ezc_source_code[c_off];
                    c_off++;
                }
                tmp[c_off - c_obj_off] = 0;
            }

            str_type.parser(&str_obj, tmp);

            estack_push(&runtime->stack, str_obj);
        }

        // skip whitespace
        while (c_off < strlen(ezc_source_code) && ezc_source_code[c_off] == ' ') {
            c_off++;
        }
    }
}

