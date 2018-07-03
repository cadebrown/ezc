
#include "exec.h"
#include "routines.h"
#include "module_loader.h"
#include "ezclang.h"
#include "ezcmacros.h"

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

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
    if (__runnable_stack_isinit) return;
    __runnable_stack_isinit = true;

    runnable_stack.len = 0;
    runnable_stack.__max_len = 0;

    runnable_stack.stack = NULL;
    runnable_stack.cur_line = NULL;
    runnable_stack.cur_char = NULL;
}

void __runnable_stack_push(runnable_t * new) {
    ENSURE_RUNNABLE_STACK
    
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
    ENSURE_RUNNABLE_STACK
    
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
                printf("At: %s:%d\n", was_running.from, line_num);
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


bool __is_valid_mpz(char * v) {
    int i;
    for (i = (v[0] == '-'); i < strlen(v); ++v) {
        if (!isdigit(v[i])) return false;
    }
    return true;
}


bool __is_valid_mpf(char * v) {
    int i;
    for (i = 0; i < strlen(v); ++i) {
        if (!(v[i] == '-' || v[i] == '.' || v[i] == 'e' || v[i] == 'E' || isdigit(v[i]))) {
            return false;
        }
    }
    return true;
}



// returns true if it is completely representable
bool __is_valid_int(char * v) {
    char * vastr = malloc(128);
    int vv = strtol(v, NULL, 0);
    sprintf(vastr, "%d", vv);
    bool res = strcmp(vastr, v) == 0;
    free(vastr);
    return res;
}


// is automatically casted to float if it is completely 
bool __is_valid_float(char * v) {
    int i;
    for (i = 0; i < strlen(v); ++i) {
        if (v[i] == '-' || v[i] == '.' || v[i] == 'e' || v[i] == 'E' || isdigit(v[i])) {
            
        } else {
            return false;
        }
    }
    return true;
}

bool __is_valid_bool(char * v) {
    return (strcmp(v, "true") == 0) || (strcmp(v, "false") == 0);
}


type_t implicit_type(char * entered) {
// uncomment this to allow default values to be mpf s

/*
    if (type_exists_name("mpz") && __is_valid_mpz(entered)) {
        return TYPE("mpz");
    }

    if (type_exists_name("mpf") && __is_valid_mpf(entered)) {
        if (type_exists_name("mpfr")) {
            return TYPE("mpfr");
        } else {
            return TYPE("mpf");
        }
    }
*/
    if (__is_valid_int(entered)) {
        return type_from_name("int");
    } else if (__is_valid_float(entered)) {
        return type_from_name("float");
    } else if (__is_valid_bool(entered)) {
        return type_from_name("bool");
    }
    return type_from_name("str");
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
    type_t block_type;

    if (type_exists_name("str")) {
        str_type = type_from_name("str");
    } else {
        printf("no 'str' type found\n");
        return;
    }
    if (type_exists_name("block")) {
        block_type = type_from_name("block");
    } else {
        printf("no 'block' type found\n");
        return;
    }


    // current character
    #define cchar (ezc_source_code[c_off])

    #define csrc (ezc_source_code + c_off)

    // last character
    #define lchar (ezc_source_code[c_off - 1])

    #define TRAVERSE(cond, block) while (c_off < strlen(ezc_source_code) && (cond)) { \
        block; \
        c_off++; \
    }

    function_t addfunc = FUNCTION("add"), subfunc = FUNCTION("sub"), mulfunc = FUNCTION("mul"), divfunc = FUNCTION("div"), powfunc = FUNCTION("pow");
    function_t negfunc = FUNCTION("neg"), floorfunc = FUNCTION("floor");
    function_t copyfunc = FUNCTION("copy"), nullfunc = FUNCTION("pushnull");
    function_t dictgetfunc = FUNCTION("dictget"), dictsetfunc = FUNCTION("dictset");
    function_t stackgetfunc = FUNCTION("stackget"), stacksetfunc = FUNCTION("stackset");
    function_t gtfunc = FUNCTION("gt");
    function_t ltfunc = FUNCTION("lt");



    // preallocated exception buffer
    char to_raise[EXCEPTION_LEN];
    
    while (c_off < strlen(ezc_source_code)) {
        
        set_curchar(c_off);
        // cast using `:TYPE`
        if (ISLIM(csrc, BLOCK_START)) {
            // TODO implment block parsing
            c_off += strlen(BLOCK_START);

            bool is_escaped = false;
            c_obj_off = c_off;
            
            TRAVERSE((!ISLIM(csrc, BLOCK_END)) && !is_escaped,
                if (IS_QUOTE(csrc)) {
                    is_escaped = !is_escaped;
                }
                tmp[c_off - c_obj_off] = cchar;
            )
            tmp[c_off - c_obj_off] = '\0';
            c_off += strlen(BLOCK_END);
            obj_t cur_block = obj_parse(block_type, tmp);

            estack_push(&runtime->stack, cur_block);
        } else if (ISLIM(csrc, CAST)) {
            // tmp will be equal to TYPE
            c_off += strlen(CAST);
            c_obj_off = c_off;
            TRAVERSE(!IS_SPECIAL(csrc), 
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
                obj_t new = obj_construct(to_type, last_on_stack);

                estack_push(&runtime->stack, new);
            }
        } else if (IS_BUILTIN(csrc)) {
            #define CASE_BUILTIN(mac, func) if (ISLIM(csrc, mac)) { \
                c_off += strlen(mac); func.function(runtime); \
            }

            CASE_BUILTIN(BUILTIN_COPY, copyfunc) 
            else CASE_BUILTIN(BUILTIN_NULL, nullfunc)
            else CASE_BUILTIN(BUILTIN_NEG, negfunc)
            else CASE_BUILTIN(BUILTIN_ADD, addfunc)
            else CASE_BUILTIN(BUILTIN_SUB, subfunc)
            else CASE_BUILTIN(BUILTIN_MUL, mulfunc)
            else CASE_BUILTIN(BUILTIN_DIV, divfunc)
            else CASE_BUILTIN(BUILTIN_POW, powfunc)
            else CASE_BUILTIN(BUILTIN_FLOOR, floorfunc)
            else CASE_BUILTIN(BUILTIN_STACKGET, stackgetfunc)
            else CASE_BUILTIN(BUILTIN_STACKSET, stacksetfunc)
            else CASE_BUILTIN(BUILTIN_GT, gtfunc)
            else CASE_BUILTIN(BUILTIN_LT, ltfunc)
            else if (ISLIM(csrc, BUILTIN_DICTSET) && strstr(csrc, BUILTIN_DICTSET) != NULL) {
                c_off += strlen(BUILTIN_DICTSET); dictsetfunc.function(runtime);
            }
            else CASE_BUILTIN(BUILTIN_DICTGET, dictgetfunc)
            else {
                sprintf(to_raise, "unknown builtin '%s'", csrc);
                raise_exception(to_raise, 1);
            }

        } else if (ISLIM(csrc, CALL_FUNCTION)) {
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

            c_off += strlen(CALL_FUNCTION);

        } else if (!IS_SPECIAL(csrc)) {
            // this section is for adding on a (string object) to the stack
            if (IS_QUOTE(csrc)) {
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
                str_obj = obj_parse(str_type, tmp);

                estack_push(&runtime->stack, str_obj);
            } else {
                c_obj_off = c_off;
                TRAVERSE(!IS_SPECIAL(csrc), 
                    tmp[c_off - c_obj_off] = cchar;
                );
                tmp[c_off - c_obj_off] = 0;

                if (!ISLIM(csrc, CAST)) {
                    type_t detected_type = implicit_type(tmp);

                    obj_t ctype_obj = obj_parse(detected_type, tmp);

                    estack_push(&runtime->stack, ctype_obj);
                } else {
                    str_obj = obj_parse(str_type, tmp);

                    estack_push(&runtime->stack, str_obj);
                }

            }

        } else if (ISLIM(csrc, COMMENT)) {
            goto leave;
        } else {
            raise_exception("unexpected character", 1);
        }

        // skip whitespace
        TRAVERSE(cchar == SPACE || cchar == SEPARATOR, )
    }


    leave:;

    free(tmp);
}


char * _getline()
{
    int sz = BUFSIZ;
    char * total_buf = NULL;
    char * tmp = malloc(sz);
    int c = 0, c_off = 0;
    while (fgets(tmp, sz, stdin) != NULL) {
        c++;
        total_buf = realloc(total_buf, c * sz);
        strcpy(total_buf + c_off, tmp);
        c_off += strlen(tmp);
        if (tmp[strlen(tmp) - 1] == '\n') break;
    }
    free(tmp);
    if (c == 0) {
        return NULL; 
    } else {
        return total_buf;
    }
}

void run_interactive_fallback(runtime_t * runtime) {
    runnable_t cur_runnable;
    char * cur_line;

    if (isatty(STDIN_FILENO)) printf("%s", INTERACTIVE_PROMPT);

    int lines = 0;

    while ((cur_line = _getline()) != NULL) {
        runnable_init_str(&cur_runnable, cur_line);

        run_runnable(runtime, &cur_runnable);
        if (isatty(STDIN_FILENO)) printf("%s", INTERACTIVE_PROMPT);

        free(cur_line);
        lines++;
    }
}


#ifndef HAVE_READLINE


void run_interactive(runtime_t * runtime) {
    run_interactive_fallback(runtime);
}


#else

#include <readline/readline.h>
#include <readline/history.h>


char ** cmd_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, cmd_generator);
}


#define GEN_TYPES   0x03
#define GEN_FUNCS   0x04

char * cmd_generator(const char * text, int state) {
    static int i, len, gen_thing;


    if (!state) {
        i = 0;
        len = strlen(text);



        bool haslim = false;

        int j;
        for (j = 0; j < strlen(text); ++j) {
            if (ISLIM(text + j, CAST)) {
                haslim = true;
            }
        }

        if (len > 0 && haslim) {
            gen_thing = GEN_TYPES;
        } else {
            gen_thing = GEN_FUNCS;
        }
    }

    if (gen_thing == GEN_TYPES) {
        if (i >= num_registered_types) {
            return NULL;
        }

        while (i++ < num_registered_types) {
            int j;

            char * looking_for = malloc(strlen(registered_types[i - 1].name) + strlen(CAST) + 1);
            sprintf(looking_for, "%s%s", CAST, registered_types[i - 1].name);
            for (j = 0; j < strlen(text); ++j) {
                if (ISLIM(looking_for, text + j)) {
                    char * res = malloc(j + strlen(looking_for) + 1);
                    sprintf(res, "%.*s%s", j, text, looking_for);
                    free(looking_for);
                    return res;
                }
            }
            free(looking_for);

        }

        // search types
    } else if (gen_thing == GEN_FUNCS) {

        if (i >= num_registered_functions) {
            return NULL;
        }
        

        while (i++ < num_registered_functions) {
            if (strncmp(registered_functions[i - 1].name, text, len) == 0) {
                char * to_ret = malloc(strlen(registered_functions[i - 1].name) + strlen(CALL_FUNCTION) + 1);
                sprintf(to_ret, "%s%s", registered_functions[i - 1].name, CALL_FUNCTION);
                return to_ret;
            }
        }
    } else {
        printf("internal error in cmd_generator\n");
        exit(1);
    }

    // now search generically

    return NULL;

}

void run_interactive(runtime_t * runtime) {
    runnable_t cur_runnable;

    char * cur_line = NULL;

    //rl_parse_and_bind("TAB: menu-complete");

    rl_attempted_completion_function = cmd_completion;
    rl_basic_word_break_characters = "\t\n\"\\'`@$=;|{(# ";

    char * interact;

    if (!isatty(STDIN_FILENO)) {
        interact = "";
    } else {
        interact = INTERACTIVE_PROMPT;
    }

    while ((cur_line = readline( interact )) != NULL) {
        runnable_init_str(&cur_runnable, cur_line);

        run_runnable(runtime, &cur_runnable);

        //runnable_free(&cur_runnable);
        if (cur_line && *cur_line) add_history(cur_line);
        free(cur_line);
    }
}


#endif




