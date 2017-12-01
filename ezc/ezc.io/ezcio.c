
#define MODULE_NAME "ezc.io"

#define MODULE_DESCRIPTION "defines ezc standard input/output"

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef struct ezc_file_t {

    char * name;

    FILE * fp;

} ezc_file_t;

void file_parser(obj_t * ret, char * value) {
    ret->data_len = sizeof(ezc_file_t);
    ret->data = malloc(ret->data_len);

    ezc_file_t efile;
    
    efile.name = malloc(strlen(value) + 1);
    strcpy(efile.name, value);

    efile.fp = NULL;

    *((ezc_file_t * )ret->data) = efile;

}

void file_constructor(obj_t * ret, obj_t value) {
    type_t str_type = type_from_name("str"), input_type = type_from_id(value.type_id);
    bool has_str = type_exists_name("str");
    
    // if passed in a string, parse it
    if (has_str && str_type.id == input_type.id) {
        file_parser(ret, (char *)value.data);
    } else {
        UNKNOWN_CONVERSION(type_name_from_id(ret->type_id), type_name_from_id(value.type_id));
    }
}

void file_representation(obj_t * obj, char ** ret) {
    ezc_file_t v = *((ezc_file_t *)obj->data);
    *ret = malloc(strlen(v.name) + 1 + 50);
    sprintf(*ret, "%s@%p", v.name, (void *)v.fp);
}

void file_destroyer(obj_t * ret) {
    ezc_file_t * v = (ezc_file_t *)ret->data;
    free(v->name);
    if (v->fp != NULL) {
        fclose(v->fp);
    }
    if (ret->data != NULL) {
        free(ret->data);
    }
    *ret = NULL_OBJ;
}


bool _open_file(ezc_file_t * v) {
    v->fp = fopen(v->name, "w");
    
    if (v->fp == NULL) {
        sprintf(to_raise, "opening file '%s' failed, reason: %s", v->name, strerror(errno));
        raise_exception(to_raise, 1);
        return false;
    }

    return true;
}

void open_file(runtime_t * runtime) {
    ASSURE_NONEMPTY_STACK;

    obj_t last_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file"), str_type = type_from_name("str");

    if (last_obj.type_id == file_type.id) {
        ezc_file_t * v = (ezc_file_t *)last_obj.data;
        if (!_open_file(v)) return;
        estack_push(&runtime->stack, last_obj);
    } else if (last_obj.type_id == str_type.id) {
        obj_t new_file_obj;
        obj_construct(file_type, &new_file_obj, last_obj);

        estack_push(&runtime->stack, new_file_obj);

        open_file(runtime);
    } else {
        UNKNOWN_TYPE(type_name_from_id(last_obj.type_id));
    }
    
}

void write_file(runtime_t * runtime) {
    if (runtime->stack.len <= 1) {
        raise_exception("not enough items on stack (need val, file)", 1);
    }

    obj_t file_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file");
    type_t str_type = type_from_name("str");


    if (file_obj.type_id == file_type.id) {
        ezc_file_t * v = (ezc_file_t *)file_obj.data;

        obj_t to_write = estack_pop(&runtime->stack);


        if (to_write.data_len == 0 || to_write.data == NULL) {
            raise_exception("data to be written has nothing/is null", 1);
        }

        if (fwrite(to_write.data, 1, to_write.data_len, v->fp) != to_write.data_len) {
            printf("warning: writing did not complete, errors may have happened\n");
        }


    } else if (file_obj.type_id == str_type.id) {
        obj_t new_file_obj;

        obj_construct(file_type, &new_file_obj, file_obj);

        estack_push(&runtime->stack, new_file_obj);

        open_file(runtime);
        write_file(runtime);

    } else {
        
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}

void close_file(runtime_t * runtime) {
    ASSURE_NONEMPTY_STACK;

    obj_t file_obj = estack_pop(&runtime->stack);


    type_t file_type = type_from_name("file");

    if (file_obj.type_id == file_type.id) {
        ezc_file_t v = *((ezc_file_t *)file_obj.data);
        if (v.fp != NULL) fclose(v.fp);
    } else {
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}


int init (int type_id, module_utils_t utils) {

    init_exported(type_id, utils);

    add_type("file", "file pointer, with name, for reading and writing", file_constructor, file_parser, file_representation, file_destroyer);

    add_function("open", "opens a file or string file name", open_file);
    add_function("write", "write to a file, assumes stack has (val, file) as last two values", write_file);
    add_function("close", "closes a file object", close_file);

    return 0;
}



