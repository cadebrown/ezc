#ifndef __EZCTYPES_H__
#define __EZCTYPES_H__

#include <stdbool.h>
#include <stdlib.h>

typedef struct obj_t {

    int type_id;

    int data_len;

    void * data;

} obj_t;



typedef void constructor_t(obj_t *, obj_t);

typedef void parser_t(obj_t *, char *);

typedef void representation_t(obj_t *, char **);

typedef void destroyer_t(obj_t *);


typedef struct type_t {

    int id;

    char * name;

    char * description;

    constructor_t * constructor;

    parser_t * parser;

    representation_t * representation;

    destroyer_t * destroyer;

} type_t;

typedef struct estack_t {

    int len;

    // this is used so it doesn't have to realloc if unneccessary.
    int __max_len_so_far;

    obj_t * vals;

} estack_t;


// dictionary
typedef struct dict_t {

    int len;

    int __max_len_so_far;

    char ** keys;

    obj_t * vals;

} dict_t;


typedef struct runtime_t {
    
    dict_t globals;

    estack_t stack;

} runtime_t;




typedef void raw_function_t(runtime_t *);

typedef struct function_t {

    int id;

    char * name;

    char * description;

    raw_function_t * function;

} function_t;

typedef struct module_export_t {

    char * description;

    int num_types;
    type_t * types;

    int num_functions;
    function_t * functions;

} module_export_t;


        
typedef bool get_type_id_t(type_t *, int);

typedef bool get_type_name_t(type_t *, char*);

#define RUNNABLE_LIVE 0x0001
#define RUNNABLE_TEXT 0x0002
#define RUNNABLE_FILE 0x0003
#define RUNNABLE_EVAL 0x0004

typedef enum runnable_flag_t { LIVE, TEXT, MODULE, SCRIPT, EVAL } runnable_flag_t;

typedef struct runnable_t {

    // should be runnable_flag
    runnable_flag_t flag;

    char * from;

    int num_lines;
    char ** lines;

} runnable_t;

typedef struct module_t {

    // this is the name in string
    char * name;

    module_export_t exported;

    // where it was loaded from
    char * path;

    // this is returned by dlopen()
    void * lib_data;

} module_t;


typedef struct module_utils_t {

    bool   (* type_exists_id     )(int);
    bool   (* type_exists_name   )(char *);
    type_t (* type_from_id       )(int);
    type_t (* type_from_name     )(char *);
    char * (* type_name_from_id  )(int);
    int    (* type_id_from_name  )(char *);

    bool       (* function_exists_id     )(int);
    bool       (* function_exists_name   )(char *);
    function_t (* function_from_id       )(int);
    function_t (* function_from_name     )(char *);
    char *     (* function_name_from_id  )(int);
    int        (* function_id_from_name  )(char *);

    int * num_types;
    type_t ** types;

    int * num_functions;
    function_t ** functions;

    int * num_modules;
    module_t ** modules;

    bool       (* import_module)(char *);

    void       (* raise_exception)(char *, int);

} module_utils_t;


// returns status code, takes type_id offset (so that they are unique)
typedef int module_init_t(int, module_utils_t utils);


// MAKE SURE TO INCLUDE 'ezcsymboldefs.h' IN SOME FILE

obj_t NULL_OBJ;
type_t NULL_TYPE;
function_t NULL_FUNCTION;

#endif
