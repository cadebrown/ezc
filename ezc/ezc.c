

#include <ezcconfig.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>


#include "ezc.h"
#include "estack.h"
#include "module_loader.h"
#include "findexecdir.h"

#ifdef WINDOWS
#include <direct.h>
#define getcwd _getcwd
#endif
 

int main(int argc, char ** argv) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"include", required_argument, NULL, 'I'},
        {NULL, 0, NULL, 0}
    };
    
    char c;
    int long_index;

    while ((c = getopt_long (argc, argv, "hI:", long_options, &long_index)) != (char)-1)
    switch (c) {
        case 'h':
            printf("Usage: %s [file0 ...] [-e expr]\n\n", argv[0]);
            printf("    -I, --include         Add a directory to search for modules in\n");
            printf("    -h, --help            Print out this usage dialogue\n");
            printf("\n");
            #ifdef EZC_DEV
            printf("Version: %d.%d [dev version]\n", EZC_VERSION_MAJOR, EZC_VERSION_MINOR);
            #else
            printf("Version: %d.%d [release]\n\n", EZC_VERSION_MAJOR, EZC_VERSION_MINOR);
            #endif
            printf("Questions, comments, concerns can go to: <cade@chemicaldevelopment.us>\n");
            return 0;
            break;
        case 'I':
            if (!add_search_path(optarg)) {
                exit(1);
            }
            break;
        case '?':
            printf("Unknown option `%c`\n", optopt);
            printf("Run `%s --help` for usage\n", argv[0]);
            return 1;
            break;
        default:
            printf("Error parsing arguments\n");
            printf("Run `%s --help` for usage\n", argv[0]);
            return 1;
            break;
    }

    char * cwd = malloc(FILENAME_MAX);
    getcwd(cwd, FILENAME_MAX);
    add_search_path(cwd);

    char * execdir = malloc(FILENAME_MAX);
    findexecdir(argv[0], execdir, FILENAME_MAX);
    add_search_path(execdir);

    char * libdir = malloc(FILENAME_MAX);
    sprintf(libdir, "%s/../lib/ezc", execdir);
    add_search_path(libdir);



    module_t test;

    load_module(&test, "systypes");
    register_module(test);

    obj_t obj;
    type_t type;


    get_type_name(&type, "float");

    type.constructor(&obj, "33.234");

    char * rval;

    type.representation(&rval, obj);

    printf("rval: '%s'\n", rval);
    printf("type name: '%s'\n", type.name);

    print_modules();


    //printf("num items: %d\n", dstack.len);

    return 0;
}

