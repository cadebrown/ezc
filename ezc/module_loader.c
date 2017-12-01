
#include "module_loader.h"

#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char ** search_paths = NULL;
int num_search_paths = 0;
int max_search_path_strlen = 0;



int raised_code = 0;
char *raised_exception = NULL;


// start out large so that system types are enough, and keep this as a block
int id_index = 0x1000;

int num_registered_modules = 0;
module_t * registered_modules = NULL;

int num_registered_types = 0;
type_t * registered_types = NULL;

module_utils_t utils;

void init_module_loader() {

    utils.type_exists_id = &type_exists_id;
    utils.type_exists_name = &type_exists_name;
    utils.type_from_id = &type_from_id;
    utils.type_from_name = &type_from_name;
    utils.type_name_from_id = &type_name_from_id;
    utils.type_id_from_name = &type_id_from_name;
    
    utils.function_exists_id = &function_exists_id;
    utils.function_exists_name = &function_exists_name;
    utils.function_from_id = &function_from_id;
    utils.function_from_name = &function_from_name;
    utils.function_name_from_id = &function_name_from_id;
    utils.function_id_from_name = &function_id_from_name;

    utils.import_module = &import_module;

    utils.raise_exception = &raise_exception;

    utils.num_functions = &num_registered_functions;
    utils.num_types = &num_registered_types;
    utils.num_modules = &num_registered_modules;
    utils.types = &registered_types;
    utils.modules = &registered_modules;
    utils.functions = &registered_functions;

    utils.has_exception = &has_exception;

}

void raise_exception(char * exception, int exitcode) {
    
    raised_exception = exception;

    if (exitcode != 0) {
        raised_code = exitcode;
    }
}

bool has_exception() {
    return raised_code != 0;
}

void register_type(type_t type) {
    num_registered_types++;
    if (registered_types == NULL) {
        registered_types = malloc(sizeof(type_t) * num_registered_types);
    } else {
        registered_types = realloc(registered_types, sizeof(type_t) * num_registered_types);
    }
    registered_types[num_registered_types - 1] = type;
}

void register_module(module_t module) {
    num_registered_modules++;
    if (registered_modules == NULL) {
        registered_modules = malloc(sizeof(module_t) * num_registered_modules);
    } else {
        registered_modules = realloc(registered_modules, sizeof(module_t) * num_registered_modules);
    }
    registered_modules[num_registered_modules - 1] = module;
}

void register_function(function_t function) {
    num_registered_functions++;
    if (registered_functions == NULL) {
        registered_functions = malloc(sizeof(function_t) * num_registered_functions);
    } else {
        registered_functions = realloc(registered_functions, sizeof(function_t) * num_registered_functions);
    }
    registered_functions[num_registered_functions - 1] = function;
}


bool get_module_name(module_t * res, char * name) {
    int i;
    for (i = 0; i < num_registered_modules; ++i) {
        if (strcmp(registered_modules[i].name, name) == 0) {
            *res = registered_modules[i];
            return true;
        }
    }
    return false;
}



//// START module_utils methods


#define SEARCH_TYPES(cmp, rfound, rnfound) { \
    int i; \
    for (i = 0; i < num_registered_types; ++i) { \
        if (cmp) { \
            return rfound; \
        } \
    } \
    return rnfound; \
}

bool type_exists_id(int id) SEARCH_TYPES(registered_types[i].id == id, true, false)

bool type_exists_name(char * name) SEARCH_TYPES(strcmp(registered_types[i].name, name) == 0, true, false)

type_t type_from_id(int id) SEARCH_TYPES(registered_types[i].id == id, registered_types[i], NULL_TYPE)

type_t type_from_name(char * name) SEARCH_TYPES(strcmp(registered_types[i].name, name) == 0, registered_types[i], NULL_TYPE)

int type_id_from_name(char * name) SEARCH_TYPES(strcmp(registered_types[i].name, name) == 0, registered_types[i].id, NULL_TYPE.id)

char * type_name_from_id(int id) SEARCH_TYPES(registered_types[i].id == id, registered_types[i].name, NULL_TYPE.name)


#define SEARCH_FUNCTIONS(cmp, rfound, rnfound) { \
    int i; \
    for (i = 0; i < num_registered_functions; ++i) { \
        if (cmp) { \
            return rfound; \
        } \
    } \
    return rnfound; \
}


bool function_exists_id(int id) SEARCH_FUNCTIONS(registered_functions[i].id == id, true, false)

bool function_exists_name(char * name) SEARCH_FUNCTIONS(strcmp(registered_functions[i].name, name) == 0, true, false)

function_t function_from_id(int id) SEARCH_FUNCTIONS(registered_functions[i].id == id, registered_functions[i], NULL_FUNCTION)

function_t function_from_name(char * name) SEARCH_FUNCTIONS(strcmp(registered_functions[i].name, name) == 0, registered_functions[i], NULL_FUNCTION)

int function_id_from_name(char * name) SEARCH_FUNCTIONS(strcmp(registered_functions[i].name, name) == 0, registered_functions[i].id, NULL_FUNCTION.id)

char * function_name_from_id(int id) SEARCH_FUNCTIONS(registered_functions[i].id == id, registered_functions[i].name, NULL_FUNCTION.name)




//// END module_utils methods



bool add_search_path(char * path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        //printf("error while adding search/include path: no such file or directory: '%s'\n", path);
        return false;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        //printf("error while adding search/include path: '%s' is not a directory\n", path);
        return false;
    }

    num_search_paths++;
    if (search_paths == NULL) {
        search_paths = (char **)malloc(sizeof(char *));
        max_search_path_strlen = strlen(path);
    } else {
        search_paths = (char **)realloc(search_paths, num_search_paths * sizeof(char *));
    }
    search_paths[num_search_paths - 1] = (char *)malloc(strlen(path) + 2);
    strcpy(search_paths[num_search_paths - 1], path);

    if (strlen(path) > max_search_path_strlen) {
        max_search_path_strlen = strlen(path);
    }
    return true;
}

void print_search_paths() {
    int i;
    for (i = 0; i < num_search_paths; ++i) {
        printf("'%s'", search_paths[i]);
        if (i != num_search_paths - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_modules() {
    int i;
    for (i = 0; i < num_registered_modules; ++i) {
        print_module(registered_modules[i]);
        printf("\n");
    }
}

void print_module(module_t module) {
    printf("%s\n", module.name);
    printf("  location: %s\n", module.path);
    printf("  types[%d]:\n", module.exported.num_types);
    int i;
    for (i = 0; i < module.exported.num_types; ++i) {
        printf("    %s [%d]\n", module.exported.types[i].name, module.exported.types[i].id);
    }
}

bool import_module(char * name) {
    module_t cur_module;
    bool found = load_module(&cur_module, name);
    if (found) {
        register_module(cur_module);
    }
    return found;
}

#define FAIL_LOAD_MODULE(reason) log_fatal("while loading module '%s': %s", name, reason); return false;

bool load_module(module_t * module, char * name) {
    char * cur_search = (char *)malloc(max_search_path_strlen + strlen(LIBRARY_PRE) + 2 * strlen(name) + strlen(LIBRARY_EXT) + 4);
    int i, csoff;
    void * handle = NULL;
    module->name = malloc(strlen(name));
    strcpy(module->name, name);
    
    #define APPEND(v) strcpy(cur_search + csoff, (v)); csoff = strlen(cur_search);
    
    for (i = 0; i < num_search_paths; ++i) {
        csoff = 0;

        APPEND(search_paths[i])
        
        if (search_paths[i][strlen(search_paths[i])] != '/') APPEND("/")
        APPEND(name)
        APPEND("/")
        APPEND(LIBRARY_PRE)
        APPEND(name)
        APPEND(LIBRARY_EXT)

        if ((handle = dlopen(cur_search, RTLD_NOW)) != NULL) break;

        // also do a copy without the 'name' subfolder
        csoff = 0;

        APPEND(search_paths[i])
        
        if (search_paths[i][strlen(search_paths[i])] != '/') APPEND("/")
        APPEND(LIBRARY_PRE)
        APPEND(name)
        APPEND(LIBRARY_EXT)

        
        if ((handle = dlopen(cur_search, RTLD_NOW)) != NULL) break;
    }

    if (handle != NULL) {
        module->path = malloc(strlen(cur_search));
        strcpy(module->path, cur_search);
    }
    free(cur_search);

    if (handle == NULL) {
        FAIL_LOAD_MODULE("module not found");
    } else {
        module->lib_data = handle;

        module_init_t * module_init = (module_init_t *)dlsym(module->lib_data, "module_init");

        if (module_init == NULL) {
            FAIL_LOAD_MODULE("module has no module_init object");
        } else {
            module_init->init(id_index, utils);
            module_export_t * exported = (module_export_t *)dlsym(module->lib_data, "exported");
            if (exported == NULL) {
                FAIL_LOAD_MODULE("module lacks an 'exported' symbol");
            } else {
                module->exported = *exported;
                int j;
                for (j = 0; j < module->exported.num_types; ++j) {
                    if (id_index < module->exported.types[j].id + 1) {
                        id_index = module->exported.types[j].id + 1;
                    }
                    register_type(module->exported.types[j]);
                }
                for (j = 0; j < module->exported.num_functions; ++j) {
                    if (id_index < module->exported.functions[j].id + 1) {
                        id_index = module->exported.functions[j].id + 1;
                    }
                    register_function(module->exported.functions[j]);
                }
            }
            /*
            printf("num types: %d\n", module->num_types);
            printf("type names:\n");
            for (i = 0; i < module->num_types; ++i) {
                printf("  %s\n", module->types[i].name);
            }
            */
            return true;
        }
    }
}

void free_module(module_t * module) {
    free(module->name);
    free(module->lib_data);
    module->name = NULL;
    module->lib_data = NULL;
}
