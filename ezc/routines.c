
#include "routines.h"

#include <string.h>
#include <stdio.h>

#include "estack.h"
#include "dict.h"

#include "ezclang.h"

#ifndef __EZCMODULE_H__
#include "module_loader.h"
#include "exec.h"
#endif


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
