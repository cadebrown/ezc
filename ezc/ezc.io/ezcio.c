
#define MODULE_NAME "ezc.io"

#define MODULE_DESCRIPTION "defines ezc standard input/output"

#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>


typedef struct ezc_file_t {

    char * name;

    char * mode;

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

void file_copier(obj_t * to, obj_t * from) {    
    to->data_len = from->data_len;
    to->data = malloc(to->data_len);

    ezc_file_t * fromv = (ezc_file_t * )from->data;
    ezc_file_t * tov = (ezc_file_t * )to->data;

    tov->name = malloc(strlen(fromv->name) + 1);
    strcpy(tov->name, fromv->name);

    tov->mode = malloc(strlen(fromv->mode) + 1);
    strcpy(tov->mode, fromv->mode);

    tov->fp = fromv->fp;
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
    if (v->name != NULL) free(v->name);
    if (v->mode != NULL) free(v->mode);
    if (v->fp != NULL) {
        fclose(v->fp);
    }
    if (ret->data != NULL) {
        free(ret->data);
    }
    *ret = NULL_OBJ;
}


bool _open_file(ezc_file_t * v, char * _mode) {
    if (v->mode != NULL) free(v->mode);
    v->mode = malloc(strlen(_mode) + 1);
    strcpy(v->mode, _mode);

    if (strcmp(v->name, "stdout") == 0) {
        v->fp = stdout;
    } else if (strcmp(v->name, "stderr") == 0) {
        v->fp = stderr;
    } else if (strcmp(v->name, "stdin") == 0) {
        v->fp = stdin;
    } else {
        v->fp = fopen(v->name, _mode);
    }
    
    if (v->fp == NULL) {
        sprintf(to_raise, "opening file '%s' failed: %s", v->name, strerror(errno));
        raise_exception(to_raise, 1);
        return false;
    }

    return true;
}

bool _close_file(ezc_file_t * v) {
    int res = fclose(v->fp);
    v->fp = NULL;
    if (v->mode != NULL) {
        free(v->mode);
        v->mode = NULL;
    }
    if (res == 0) {
        return true;
    } else {
        return false;
    }
}

void open_file(runtime_t * runtime) {
    
    ASSURE_NONEMPTY_STACK;

    obj_t last_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file"), str_type = type_from_name("str");


    if (last_obj.type_id == file_type.id) {
        ezc_file_t * v = (ezc_file_t *)last_obj.data;
    
        if (!_open_file(v, "w+")) {
            raise_exception("opening file failed", 1);
            return;
        }
    
        estack_push(&runtime->stack, last_obj);

    } else if (last_obj.type_id == str_type.id) {
        obj_t new_file_obj;

        obj_construct(file_type, &new_file_obj, last_obj);

        obj_free(&last_obj);

        estack_push(&runtime->stack, new_file_obj);

        open_file(runtime);

    } else {

        UNKNOWN_TYPE(type_name_from_id(last_obj.type_id));
    }
    
}

void write_file(runtime_t * runtime) {
    if (runtime->stack.len <= 1) {
        raise_exception("not enough items on stack (need val, file)", 1);
        return;       
    }

    obj_t to_write = estack_pop(&runtime->stack);

    obj_t file_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file");
    type_t str_type = type_from_name("str");


    if (file_obj.type_id == file_type.id) {
        
        ezc_file_t * v = (ezc_file_t *)file_obj.data;

        if (v->fp == NULL && !_open_file(v, "w")) {
            raise_exception("file object is null pointer and could not be opened", 1);
            return;
        }

        if (to_write.data_len <= 1 || to_write.data == NULL) {
            raise_exception("data to be written has nothing/is null", 1);
            return;
        }

        if (fwrite(to_write.data, 1, to_write.data_len - 1, v->fp) != to_write.data_len - 1) {
            printf("warning: writing did not complete, errors may have happened\n");
        }


        estack_push(&runtime->stack, file_obj);

        obj_free(&to_write);

    } else if (file_obj.type_id == str_type.id) {
        obj_t new_file_obj;

        obj_construct(file_type, &new_file_obj, file_obj);

        obj_free(&file_obj);

        estack_push(&runtime->stack, new_file_obj);

        open_file(runtime);

        estack_push(&runtime->stack, to_write);

        write_file(runtime);

    } else {
        
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}

void read_file(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        raise_exception("not enough items on stack (need file)", 1);
        return;
    }

    obj_t file_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file");
    type_t str_type = type_from_name("str");


    if (file_obj.type_id == file_type.id) {
        ezc_file_t * v = (ezc_file_t *)file_obj.data;

        if (v->fp == NULL && !_open_file(v, "r+")) {
            raise_exception("file object is null pointer", 1);
            return;
        }
        
        obj_t read_obj;

        char * read_buffer = NULL;

        if (v->fp == stdin) {
            read_buffer = NULL;
            char * tmp_buf = malloc(BUFSIZ);
            size_t tmp_len, total_len = 0;
            //printf("asdfasdf\n");
            while (fgets(tmp_buf, BUFSIZ, stdin) != NULL) {
                tmp_len = strlen(tmp_buf);
                
                read_buffer = realloc(read_buffer, total_len + tmp_len + 1);

                strcpy(read_buffer + total_len, tmp_buf);

                total_len += tmp_len;
            }

            free(tmp_buf);

        } else {
            // normal file
            fseek(v->fp, 0, SEEK_END);
            int numbytes = ftell(v->fp);
            fseek(v->fp, 0, SEEK_SET);

            read_buffer = malloc(numbytes + 1);

            if (read_buffer == NULL) {
                raise_exception("mallocing buffer failed", 1);
            }

            int recv_bytes;

            if ((recv_bytes = fread(read_buffer, 1, numbytes, v->fp)) != numbytes) {
                printf("warning: reading did not complete, errors may have happened, numbytes=%d, recv_bytes=%d\n", numbytes, recv_bytes);
            }

            read_buffer[numbytes] = 0;

        }
        if (read_buffer == NULL) {
            raise_exception("internal error with read buffer", 1);
        }

        obj_parse(str_type, &read_obj, read_buffer);

            
        free(read_buffer);

        estack_push(&runtime->stack, file_obj);
        estack_push(&runtime->stack, read_obj);

    } else if (file_obj.type_id == str_type.id) {
        obj_t new_file_obj;

        obj_construct(file_type, &new_file_obj, file_obj);

        ezc_file_t * v = (ezc_file_t *)new_file_obj.data;

        if (!_open_file(v, "r")) {
            sprintf(to_raise, "could not open file '%s'", v->name);
            raise_exception(to_raise, 1);
            return;
        }

        obj_free(&file_obj);

        estack_push(&runtime->stack, new_file_obj);

        read_file(runtime);

    } else {
        
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}



void clear_file(runtime_t * runtime) {
    if (runtime->stack.len <= 0) {
        raise_exception("not enough items on stack (need file)", 1);
        return;       
    }

    obj_t file_obj = estack_pop(&runtime->stack);

    type_t file_type = type_from_name("file");
    type_t str_type = type_from_name("str");


    if (file_obj.type_id == file_type.id) {
        
        ezc_file_t * v = (ezc_file_t *)file_obj.data;

        if (v->fp == NULL) {
            if (_open_file(v, "w") && _close_file(v)) {
            } else {
                raise_exception("file failed to clear", 1);
                return;
            }
        } else {
            char * l_mode = malloc(strlen(v->mode) + 1);
            strcpy(l_mode, v->mode);
            if (!(_close_file(v) && _open_file(v, "w") && _close_file(v) && _open_file(v, l_mode))) {
                raise_exception("file failed to clear", 1);
            } else {
                free(l_mode);
            }
        }

        estack_push(&runtime->stack, file_obj);

    } else if (file_obj.type_id == str_type.id) {
        obj_t new_file_obj;

        obj_construct(file_type, &new_file_obj, file_obj);

        obj_free(&file_obj);

        estack_push(&runtime->stack, new_file_obj);

        open_file(runtime);
        clear_file(runtime);

    } else {
        
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}


void close_file(runtime_t * runtime) {
    ASSURE_NONEMPTY_STACK;

    obj_t file_obj = estack_pop(&runtime->stack);


    type_t file_type = type_from_name("file");

    if (file_obj.type_id == file_type.id) {
        ezc_file_t * v = (ezc_file_t *)file_obj.data;
        if (v->fp != NULL) {
            fclose(v->fp);
            
        }
    } else {
        UNKNOWN_TYPE(type_name_from_id(file_obj.type_id));
    }
}


int init (int type_id, module_utils_t utils) {

    init_exported(type_id, utils);

    add_type("file", "file pointer, with name, for reading and writing", file_constructor, file_copier, file_parser, file_representation, file_destroyer);

    add_function("open", "opens a file or string file name", open_file);
    add_function("read", "read from a file, assumes stack has (val, file) as last two values", read_file);
    add_function("write", "write to a file, assumes stack has (val, file) as last two values", write_file);
    add_function("clear", "clear file contents", clear_file);
    add_function("close", "closes a file object", close_file);

    return 0;
}



