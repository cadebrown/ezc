#ifndef __MODULE_LOADER_H__
#define __MODULE_LOADER_H__

#include "ezc.h"

#include <stdlib.h>
#include <stdio.h>

#define LIBRARY_PRE "lib"

#ifdef _WIN32
#define LIBRARY_EXT ".dll"
#elif __APPLE__
#define LIBRARY_EXT ".dylib"
#else
#define LIBRARY_EXT ".so"
#endif


// returns status code, takes type_id offset (so that they are unique)
typedef int module_init_t(int, module_utils_t utils);

typedef struct module_t {

    // this is the name in string
    char * name;

    module_export_t exported;

    // where it was loaded from
    char * path;

    // this is returned by dlopen()
    void * lib_data;

    module_init_t * init;

} module_t;



char ** search_paths;
int num_search_paths;
int max_search_path_strlen;

// start out large so that system types are enough, and keep this as a block
int id_index;

int num_registered_modules;
module_t * registered_modules;

int num_registered_types;
type_t * registered_types;

int num_registered_functions;
function_t * registered_functions;

module_utils_t utils;

void init_module_loader();

bool import_module(char * name);

void register_type(type_t type);
void register_module(module_t module);

//// START module_utils methods

bool type_exists_id(int);

bool type_exists_name(char *);

type_t type_from_id(int);

type_t type_from_name(char *);

int type_id_from_name(char *);

char * type_name_from_id(int);


bool function_exists_id(int);

bool function_exists_name(char *);

function_t function_from_id(int);

function_t function_from_name(char *);

int function_id_from_name(char *);

char * function_name_from_id(int);


//// END module_utils methods



bool get_module_name(module_t * res, char * name);

bool add_search_path(char * path);

void print_search_paths();

void print_modules();

void print_module(module_t module);


bool load_module(module_t * module, char * name);

void free_module(module_t * module);



#endif
