
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
typedef int module_init_t(int);

typedef struct module_t {

    // this is the name in string
    char * name;

    // where it was loaded from
    char * path;

    // this is returned by dlopen()
    void * lib_data;

    module_init_t * init;

    int num_types;
    type_t * types;

} module_t;



char ** search_paths;
int num_search_paths;
int max_search_path_strlen;

// start out large so that system types are enough, and keep this as a block
int type_id;

int num_registered_modules;
module_t * registered_modules;

int num_registered_types;
type_t * registered_types;



void register_type(type_t type);
void register_module(module_t module);

bool get_type_id(type_t * res, int id);
bool get_type_name(type_t * res, char * name);

bool get_module_name(module_t * res, char * name);

bool add_search_path(char * path);

void print_search_paths();

void print_modules();

void print_module(module_t module);


bool load_module(module_t * module, char * name);

void free_module(module_t * module);


