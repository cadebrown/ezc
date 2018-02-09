
#include "routines.h"

#include <string.h>
#include <stdio.h>

#include "estack.h"
#include "dict.h"

#include "ezcmacros.h"

#include "ezclang.h"

#ifndef __EZCMODULE_H__
#include "module_loader.h"
#include "exec.h"
#endif


bool valid_module_name(char * fn) {
    int i;
    for (i = 0; i < strlen(fn); ++i) {
        if (!((fn[i] >= 'a' && fn[i] <='z') || (fn[i] >='A' && fn[i] <= 'Z') || fn[i] == '.')) {
            return false;
        }
    }
    return true;
}

bool valid_type_name(char * fn) {
    int i;
    for (i = 0; i < strlen(fn); ++i) {
        if (!((fn[i] >= 'a' && fn[i] <='z') || (fn[i] >='A' && fn[i] <= 'Z'))) {
            return false;
        }
    }
    return true;
}



obj_t repr_obj(obj_t obj) {
    type_t str_type = TYPE("str");

    obj_t res;

    if (obj.type_id == NULL_TYPE.id) {
        res = obj_parse(str_type, "null");
    } else {
        char * repr_str;

        type_t obj_type = OBJ_TYPE(obj);

        repr_str = obj_representation(obj_type, &obj);

        res = obj_parse(str_type, repr_str);
    
        free(repr_str);
    }

    return res;
}


void dump_runtime(runtime_t * runtime) {
    int i;

    printf("globals[%d]:\n", runtime->globals.len);
    obj_t c_repr;
    type_t c_type;
    for (i = 0; i < runtime->globals.len; ++i) {
        c_repr = repr_obj(runtime->globals.vals[i]);
        c_type = type_from_id(runtime->globals.vals[i].type_id);
        printf ("  '%s': '%s':%s\n", runtime->globals.keys[i], (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }

    printf("stack[%d]:\n", runtime->stack.len);

    for (i = 0; i < runtime->stack.len; ++i) {
        c_repr = repr_obj(runtime->stack.vals[i]);
        c_type = type_from_id(runtime->stack.vals[i].type_id);
        printf ("  %d: '%s':%s\n", i, (char *)c_repr.data, c_type.name);
        obj_free(&c_repr);
    }

    printf("\n");

}

bool valid_function_name(char * fn) {
    if (strstr(fn, CALL_FUNCTION) || strstr(fn, CAST) || IS_BUILTIN(fn)) {
        return false;
    }
    int i;
    for (i = 0; i < strlen(fn); ++i) {
        if (fn[i] == ' ') {
            return false;
        }
    }
    return true;
}


obj_t obj_copy(obj_t input) {
    obj_t ret;
    ret.type_id = input.type_id;
    type_t input_type = type_from_id(input.type_id);
    input_type.copier(&ret, &input);
    return ret;
}

void obj_free(obj_t * obj) {
    if (!ISTYPE(*obj, NULL_TYPE)) {
        type_t obj_type = type_from_id(obj->type_id);
        obj_type.destroyer(obj);
        *obj = NULL_OBJ;
    }
}

obj_t obj_construct(type_t type, obj_t from) {
    obj_t ret;
    ret.type_id = type.id;
    type.constructor(&ret, from);
    return ret;
    
}


obj_t obj_parse(type_t type, char * from) {
    obj_t ret;
    ret.type_id = type.id;
    type.parser(&ret, from);    
    return ret;
}

char * obj_representation(type_t type, obj_t * from) {
    char * reprstr = NULL;
    type.representation(from, &reprstr);
    return reprstr;
}

void init_runtime(runtime_t * runtime) {
    estack_init(&runtime->stack);
    dict_init(&runtime->globals);
}

// allocates memory
void str_obj_force(char ** out, obj_t in) {
    if (!type_exists_name("str")) {
        raise_exception("no str type", 1);
    }

    type_t str_type = type_from_name("str");

    if (in.type_id == str_type.id) {
        *out = malloc(in.data_len);
        strcpy(*out, (char *)in.data);
    } else {
        char to_raise[EXCEPTION_LEN];
        sprintf(to_raise, "object is not a string, but is '%s'", type_name_from_id(in.type_id));
        raise_exception(to_raise, 1);
    }

}


void runnable_add_str(runnable_t * runnable, char * _src) {
    runnable->num_lines++;
    runnable->lines = realloc(runnable->lines, sizeof(char *) * runnable->num_lines);

    runnable->lines[runnable->num_lines - 1] = malloc(strlen(_src) + 1);
    strcpy(runnable->lines[runnable->num_lines - 1], _src);
}

void runnable_init_str(runnable_t * runnable, char * src) {
    int lines = 0;
    int lc = 0, cc = 0;

    #define NEXTLINE while (cc < strlen(src) && src[cc] != '\n') { cc++; }

    char * cur_line = NULL;

    runnable->lines = NULL;


    while (cc < strlen(src)) {
        lc = cc;
        NEXTLINE
        cur_line = malloc(cc - lc + 1);
        strncpy(cur_line, src + lc, cc - lc);
        cur_line[cc - lc] = '\0';
        lines++;
        runnable->lines = realloc(runnable->lines, sizeof(char *) * lines);
        runnable->lines[lines - 1] = cur_line;
        cc++;
    }

    runnable->num_lines = lines;
}

void runnable_free(runnable_t * runnable) {
    if (runnable->from != NULL) {
        free(runnable->from);
    }
    runnable->from = NULL;
    int i;
    for (i = 0; i < runnable->num_lines; ++i) {
        free(runnable->lines[i]);
    }
    free(runnable->lines);
    runnable->lines = NULL;
    runnable->num_lines = 0;
}

