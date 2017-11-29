
#include "module_loader.h"

#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char ** search_paths = NULL;
int num_search_paths = 0;
int max_search_path_strlen = 0;


// start out large so that system types are enough, and keep this as a block
int type_id = 0x1000;


int num_registered_modules = 0;
module_t * registered_modules = NULL;

int num_registered_types = 0;
type_t * registered_types = NULL;


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

// returns true if it was successful
bool get_type_id(type_t * res, int id) {
    int i;
    for (i = 0; i < num_registered_types; ++i) {
        if (registered_types[i].id == id) {
            *res = registered_types[i];
            return true;
        }
    }
    return false;
}

bool get_type_name(type_t * res, char * name) {
    int i;
    for (i = 0; i < num_registered_types; ++i) {
        if (strcmp(registered_types[i].name, name) == 0) {
            *res = registered_types[i];
            return true;
        }
    }
    return false;
}


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
    printf("  types[%d]:\n", module.num_types);
    int i;
    for (i = 0; i < module.num_types; ++i) {
        printf("    %s [%d]\n", module.types[i].name, module.types[i].id);
    }
}

bool load_module(module_t * module, char * name) {
    char * cur_search = (char *)malloc(max_search_path_strlen + strlen(LIBRARY_PRE) + 2 * strlen(name) + strlen(LIBRARY_EXT) + 4);
    int i, j, csoff;
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
        printf("Did not find library by name '%s'\n", name);
        printf("dlerror: %s\n", dlerror());
        return false;
    } else {
        module->lib_data = handle;
        
        module->init = (module_init_t *)dlsym(module->lib_data, "init");

        if (module->init == NULL) {
            printf("Could not find init() method\n");
            printf("dlerror: %s\n", dlerror());
            return false;
        } else {
            module->init(type_id);
            int * num_types_ptr = (int *)dlsym(module->lib_data, "num_types");
            if (num_types_ptr == NULL) {
            } else {
                module->num_types = *num_types_ptr;
                module->types = *(type_t **)dlsym(module->lib_data, "types");
                int j;
                for (j = 0; j < module->num_types; ++j) {
                    if (type_id < module->types[j].id + 1) {
                        type_id = module->types[j].id + 1;
                    }
                    register_type(module->types[j]);
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
