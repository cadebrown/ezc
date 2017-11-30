
#include "ec.h"
#include "ezc.h"
#include <ezcconfig.h>


#include <stdio.h>
#include <getopt.h>
#include <unistd.h>

int main(int argc, char ** argv) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"include", required_argument, NULL, 'I'},
        {"import", required_argument, NULL, 'i'},
        {NULL, 0, NULL, 0}
    };
    
    char c;
    int long_index;

    int num_to_import = 0;
    char ** to_import = malloc(sizeof(char *));

    while ((c = getopt_long (argc, argv, "hI:i:", long_options, &long_index)) != (char)-1)
    switch (c) {
        case 'h':
            printf("Usage: %s [file0 ...] [-e expr]\n\n", argv[0]);
            printf("    -I, --include         Add a directory to search for modules in\n");
            printf("    -i, --import          Import module\n");
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
        case 'i':
            num_to_import++;
            to_import = realloc(to_import, sizeof(char *) * num_to_import);
            to_import[num_to_import - 1] = optarg;           
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

    ezc_init(argv[0]);

    // import all defaults here

    int i;
    for (i = 0; i < num_to_import; ++i) {
        import_module(to_import[i]);
    }

    runtime_t runtime;
    init_runtime(&runtime);

    run_code(&runtime, "hello world");


    /*
    type_t str;
    if (!type_exists_name("str")) {
        printf("No 'str' found!\n");
    } else {
        str = type_from_name("str");
    }

    obj_t A_str;
    str.parser(&A_str, "Hello world!");

    printf("asdfasdf\n");    

    estack_t estack;
    estack_init(&estack);

    estack_push(&estack, A_str);
    estack_push(&estack, A_str);
    estack_push(&estack, A_str);
    estack_push(&estack, A_str);

    ((char *)A_str.data)[0] = 'B';

    function_t printstack = function_from_name("printstack");

    printstack.function(&estack);

    */
}