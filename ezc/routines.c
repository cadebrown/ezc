
#include "routines.h"

#include <string.h>
#include <stdio.h>

#include "estack.h"
#include "dict.h"

#ifndef __EZCMODULE_H__
#include "module_loader.h"
#endif

#include "ezclang.h"


obj_t obj_copy(obj_t input) {
    obj_t ret;
    ret.type_id = input.type_id;
    type_t input_type = type_from_id(input.type_id);
    input_type.copier(&ret, &input);
    return ret;
}

void obj_free(obj_t * obj) {
    type_t obj_type = type_from_id(obj->type_id);
    obj_type.destroyer(obj);
}

void obj_construct(type_t type, obj_t * to, obj_t from) {
    to->type_id = type.id;
    type.constructor(to, from);    
}


void obj_parse(type_t type, obj_t * to, char * from) {
    to->type_id = type.id;
    type.parser(to, from);    
}

void obj_representation(type_t type, obj_t * from, char ** to) {
    type.representation(from, to);    
}

void init_runtime(runtime_t * runtime) {
    estack_init(&runtime->stack);
    dict_init(&runtime->globals);
}

bool str_obj_force(char ** out, obj_t in) {
    if (!type_exists_name("str")) {
        raise_exception("no str type", 1);
        return false;
    }

    type_t str_type = type_from_name("str");
    if (in.type_id == str_type.id) {
        *out = malloc(in.data_len);
        strcpy(*out, (char *)in.data);
        return true;
    } else {
        char to_raise[EXCEPTION_LEN];
        sprintf(to_raise, "object is not a string, but is '%s'", type_name_from_id(in.type_id));

        raise_exception(to_raise, 1);
        return false;
    }

}

void runnable_init_str(runnable_t * runnable, char * _src) {
    int lines = 0;
    char * src = malloc(strlen(_src) + 1);
    strcpy(src, _src);
    char * delim = NEWLINE_STR;
    char * cur_line = strtok(src, delim);

    while (cur_line != NULL) {
        cur_line = strtok(NULL, delim);
        lines++;
    }

    runnable->num_lines = lines;
    runnable->lines = malloc(sizeof(char *) * lines);
    
    strcpy(src, _src);
    
    cur_line = strtok(src, delim);

    int i = 0;

    while (cur_line != NULL) {
        runnable->lines[i] = malloc(strlen(cur_line) + 1);
        strcpy(runnable->lines[i], cur_line);
        
        cur_line = strtok(NULL, delim);
        i++;
    }
}


void run_runnable(runtime_t * runtime, runnable_t * runnable) {
    int i;
    for (i = 0; i < runnable->num_lines; ++i) {
        run_str(runtime, runnable->lines[i], runnable, i);
    }
}

void run_str(runtime_t * runtime, char * ezc_source_code, runnable_t * runnable, int linenum) {
    char * tmp = malloc(strlen(ezc_source_code) + 1);
    
    int c_off = 0;
    int c_obj_off;

    int loop_start_off;

    obj_t str_obj;
    type_t str_type;

    if (type_exists_name("str")) {
        str_type = type_from_name("str");
    } else {
        printf("no 'str' type found\n");
        return;
    }

    // current character
    #define cchar (ezc_source_code[c_off])
    // last character
    #define lchar (ezc_source_code[c_off - 1])

    #define TRAVERSE(cond, block) while (c_off < strlen(ezc_source_code) && (cond)) { \
        block; \
        c_off++; \
    }

    // raise error while compiling
    #define craise(reason, code) raise_exception(reason, code); goto handle;

    // preallocated exception buffer
    char to_raise[EXCEPTION_LEN];


    while (c_off < strlen(ezc_source_code)) {
        loop_start_off = c_off;

        // cast using `:TYPE`
        if (cchar == CAST) {
            // tmp will be equal to TYPE
            c_off++;
            c_obj_off = c_off;
            TRAVERSE(!IS_SPECIAL(cchar), 
                tmp[c_off - c_obj_off] = cchar;
            );
            tmp[c_off - c_obj_off] = CHAR0;

            type_t to_type;
            if (type_exists_name(tmp)) {
                to_type = type_from_name(tmp);
            } else {
                sprintf(to_raise, "could not find type '%s'", tmp);
                craise(to_raise, 1);
            }


            if (runtime->stack.len <= 0) {
                sprintf(to_raise, "no objects on stack to cast");
                craise(to_raise, 1);
            } else {
                obj_t last_on_stack = estack_pop(&runtime->stack);
                obj_t new;
                
                obj_construct(to_type, &new, last_on_stack);

                estack_push(&runtime->stack, new);
            }
        
        } else if (cchar == CALL_FUNCTION) {
            if (runtime->stack.len <= 0) {
                sprintf(to_raise, "no function on the stack");
                craise(to_raise, 1);
            }
            obj_t last_on_stack = estack_pop(&runtime->stack);
            if (last_on_stack.type_id != str_type.id) {
                function_t df = function_from_name("dump");
                df.function(runtime);
                sprintf(to_raise, "the function you are trying to call is not a string (it is of type '%s')", type_name_from_id(last_on_stack.type_id));
                craise(to_raise, 1);
                return;
            }

            char * func_name = last_on_stack.data;
            if (!function_exists_name(func_name)) {
                sprintf(to_raise, "no function named '%s'", func_name);
                craise(to_raise, 1);
                return;
            }

            function_t cur_func = function_from_name(func_name);

            cur_func.function(runtime);

            c_off++;

        } else {
            // this section is for adding on a (string object) to the stack
            if (IS_QUOTE(cchar)) {
                char which_quote_used = cchar;
                c_off++;
                c_obj_off = c_off;
                int num_escapes = 0;
                TRAVERSE(cchar != which_quote_used,
                    if (cchar == ESC) {
                        c_off++;
                        num_escapes++;
                        char escaped = 0;
                        switch (cchar) {
                            case 'n':
                                escaped = '\n';
                                break;
                            case '"':
                                escaped = '"';
                                break;
                            case '\'':
                                escaped = '\'';
                                break;
                            default:
                                sprintf(to_raise, "unknown escape sequence '\\%c' in string literal", cchar);
                                raise_exception(to_raise, 1);
                                break;
                        }
                        tmp[c_off - c_obj_off - num_escapes] = escaped;
                    } else {
                        tmp[c_off - c_obj_off - num_escapes] = cchar;
                    }
                );
                tmp[c_off - c_obj_off - num_escapes] = CHAR0;
                if (cchar != which_quote_used) {
                    //fail(runtime, 1);
                    //exit(1);
                    craise("no terminating string", 1);
                } else {
                    c_off++;
                }
            } else {
                c_obj_off = c_off;
                TRAVERSE(!IS_SPECIAL(cchar), 
                    tmp[c_off - c_obj_off] = cchar;
                );
                tmp[c_off - c_obj_off] = 0;
            }

            obj_parse(str_type, &str_obj, tmp);

            estack_push(&runtime->stack, str_obj);
        }

        handle:
        #ifndef __EZCMODULE_H__
        if (raised_code != 0) {
            printf("Exception was raised: %s\n", raised_exception);
            if (runnable != NULL) {

                printf("At:\n");
                printf("%s\n", ezc_source_code);
                int j;
                for (j = 0; j < loop_start_off; ++j) {
                    printf(" ");
                }
                printf("^\n");
            }
            exit(raised_code);
        }
        #endif
        
        // skip whitespace
        TRAVERSE(cchar == SPACE || cchar == SEPARATOR, )
    }
}

