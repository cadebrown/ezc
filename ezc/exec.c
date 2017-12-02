
#include "exec.h"
#include "routines.h"
#include "module_loader.h"
#include "ezclang.h"

#include <string.h>

#include <setjmp.h>

struct {

    int len;

    int __max_len;

    runnable_t ** stack;

    int * cur_line;

    int * cur_char;

} runnable_stack;

bool __runnable_stack_isinit = false;

#define ENSURE_RUNNABLE_STACK if (!__runnable_stack_isinit) { __runnable_stack_init(); }


void set_curline(int cline) {
    runnable_stack.cur_line[runnable_stack.len - 1] = cline;
}

void set_curchar(int cchar) {
    runnable_stack.cur_char[runnable_stack.len - 1] = cchar;
}

void __runnable_stack_init() {
    __runnable_stack_isinit = true;

    runnable_stack.len = 0;
    runnable_stack.__max_len = 0;

    runnable_stack.stack = NULL;
    runnable_stack.cur_line = NULL;
    runnable_stack.cur_char = NULL;
}

void __runnable_stack_push(runnable_t * new) {
    runnable_stack.len++;
    if (runnable_stack.stack == NULL) {
        runnable_stack.stack = malloc(sizeof(runnable_t *) * runnable_stack.len);
        runnable_stack.cur_line = malloc(sizeof(int) * runnable_stack.len);
        runnable_stack.cur_char = malloc(sizeof(int) * runnable_stack.len);
        runnable_stack.__max_len = runnable_stack.len;
    } else if (runnable_stack.len > runnable_stack.__max_len) {
        runnable_stack.stack = realloc(runnable_stack.stack, sizeof(runnable_t *) * runnable_stack.len);
        runnable_stack.cur_line = realloc(runnable_stack.cur_line, sizeof(int) * runnable_stack.len);
        runnable_stack.cur_char = realloc(runnable_stack.cur_char, sizeof(int) * runnable_stack.len);
        runnable_stack.__max_len = runnable_stack.len;
    }

    runnable_stack.stack[runnable_stack.len - 1] = new;
    runnable_stack.cur_line[runnable_stack.len - 1] = -1;
    runnable_stack.cur_char[runnable_stack.len - 1] = -1;
}

void __runnable_stack_pop() {
    if (runnable_stack.stack == NULL || runnable_stack.len <= 0) {
        printf("internal error in a __runable_stack method\n");
        exit(1);
    }
    runnable_stack.len--;
}


void raise_exception(char * exception, int exitcode) {
    if (exitcode != 0) {
        printf("Exception was raised: %s\n", exception);
        if (runnable_stack.stack != NULL) {

            runnable_t was_running = (*runnable_stack.stack[runnable_stack.len - 1]);

            int line_num = runnable_stack.cur_line[runnable_stack.len - 1];
            int char_num = runnable_stack.cur_char[runnable_stack.len - 1];

            if (line_num >= 0) {
                printf("At:\n");
                printf("%s\n", was_running.lines[line_num]);

                if (char_num >= 0) {
                    int j;
                    for (j = 0; j < char_num; ++j) {
                        printf(" ");
                    }
                    printf("^\n");
                }
            }
        }
        exit(exitcode);
    }
}

void run_runnable(runtime_t * runtime, runnable_t * runnable) {
    ENSURE_RUNNABLE_STACK

    __runnable_stack_push(runnable);


    int i;
    for (i = 0; i < runnable->num_lines; ++i) {
        set_curline(i);
        run_str(runtime, runnable->lines[i]);
    }

    __runnable_stack_pop();
}

void run_str(runtime_t * runtime, char * ezc_source_code) {
    ENSURE_RUNNABLE_STACK

    char * tmp = malloc(strlen(ezc_source_code) + 1);
    
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

    // current character
    #define cchar (ezc_source_code[c_off])
    // last character
    #define lchar (ezc_source_code[c_off - 1])

    #define TRAVERSE(cond, block) while (c_off < strlen(ezc_source_code) && (cond)) { \
        block; \
        c_off++; \
    }


    // preallocated exception buffer
    char to_raise[EXCEPTION_LEN];
    
    while (c_off < strlen(ezc_source_code)) {
        set_curchar(c_off);
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
                raise_exception(to_raise, 1);
            }


            if (runtime->stack.len <= 0) {
                sprintf(to_raise, "no objects on stack to cast");
                raise_exception(to_raise, 1);
            } else {
                obj_t last_on_stack = estack_pop(&runtime->stack);
                obj_t new;
                
                obj_construct(to_type, &new, last_on_stack);

                estack_push(&runtime->stack, new);
            }
        
        } else if (cchar == CALL_FUNCTION) {
            if (runtime->stack.len <= 0) {
                sprintf(to_raise, "no function on the stack");
                raise_exception(to_raise, 1);
            }
            obj_t last_on_stack = estack_pop(&runtime->stack);
            if (last_on_stack.type_id != str_type.id) {
                function_t df = function_from_name("dump");
                df.function(runtime);
                sprintf(to_raise, "the function you are trying to call is not a string (it is of type '%s')", type_name_from_id(last_on_stack.type_id));
                raise_exception(to_raise, 1);
            }

            char * func_name = last_on_stack.data;
            if (!function_exists_name(func_name)) {
                sprintf(to_raise, "no function named '%s'", func_name);
                raise_exception(to_raise, 1);
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
                    raise_exception("no terminating string", 1);
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

        // skip whitespace
        TRAVERSE(cchar == SPACE || cchar == SEPARATOR, )
    }

    free(tmp);
}


