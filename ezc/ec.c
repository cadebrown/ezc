
#include "ec.h"
#include "ezc.h"
#include "exec.h"

#include <ezcconfig.h>


#include <stdio.h>
#include <getopt.h>
#include <unistd.h>

#include <string.h>


int main(int argc, char ** argv) {
    char c;
    int long_index;

    int num_to_import = 0;
    char ** to_import = malloc(sizeof(char *));

    char * expression = NULL;
    char * rfile = NULL;

    static int noreadline = false;

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"include", required_argument, NULL, 'I'},
        {"import", required_argument, NULL, 'i'},
        {"expression", required_argument, NULL, 'e'},
        {"file", required_argument, NULL, 'f'},
        {"verbosity", required_argument, NULL, 'v'},
        {"noreadline", no_argument, &noreadline, 1},
        {NULL, 0, NULL, 0}
    };
    

    while ((c = getopt_long (argc, argv, "hI:i:e:f:v:", long_options, &long_index)) != (char)-1)
    switch (c) {
        case 'h':
            printf("Usage: %s [-f file | -e expr]\n\n", argv[0]);
            printf("    -f, --file            Evalue a file\n");
            printf("    -e, --expression      Evaluate an expression\n");
            printf("\n");
            #ifdef HAVE_READLINE
            printf("    --noreadline          Force using the non-readline interpreter\n");
            printf("\n");
            #endif
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
        case 'e':
            if (expression != NULL) {
                printf("error, already specified '-e'\n");
                return 1;
            } else if (rfile != NULL) {
                printf("error, cannot use '-e' and '-f'\n");
                return 1;
            } else {
                expression = optarg;
            }
            break;
        case 'f':
            if (rfile != NULL) {
                printf("error, already specified '-f'\n");
                return 1;
            } else if (expression != NULL) {
                printf("error, cannot use '-e' and '-f'\n");
                return 1;
            } else {
                rfile = optarg;
            }
            break;
        case 'v':
            log_level = atoi(optarg);
            break;
        case '?':
            printf("Unknown option `%c`\n", optopt);
            printf("Run `%s --help` for usage\n", argv[0]);
            return 1;
            break;
        case 0:
            break;
        default:
            printf("Error parsing arguments\n");
            printf("Run `%s --help` for usage\n", argv[0]);
            return 1;
            break;
    }

    ezc_init(argv[0]);

    // import all defaults here

    import_module("ezc.types");
    import_module("ezc.functions");
    import_module("ezc.io");

    int i;
    for (i = 0; i < num_to_import; ++i) {
        import_module(to_import[i]);
    }

    runtime_t runtime;
    init_runtime(&runtime);

    if (expression == NULL && rfile != NULL) {
        runnable_t file_run;
        file_run.from = malloc(strlen(rfile) + 1);
        strcpy(file_run.from, rfile);

        FILE * fp;
        fp = fopen(rfile, "r");
        if (fp == NULL) {
            printf("error opening file '%s'\n", rfile);
            return 1;
        }

        int numbytes;
        fseek(fp, 0, SEEK_END);
        numbytes = ftell(fp);
        fseek(fp, 0, SEEK_SET);  //same as rewind(f);

        char * file_source = malloc(numbytes + 1);

        if (file_source == NULL) {
            log_fatal("could not malloc the source buffer");
            exit(1);
        }

        if (numbytes != fread(file_source, numbytes, 1, fp)) {
            log_fatal("could not read file source");
            exit(1);
        }

        fclose(fp);

        file_source[numbytes] = 0;

        runnable_init_str(&file_run, file_source);
        run_runnable(&runtime, &file_run);

    } else if (expression != NULL) {
        runnable_t expr_run;
        expr_run.from = "-e";

        runnable_init_str(&expr_run, expression);
        run_runnable(&runtime, &expr_run);

    } else {
        if (noreadline == 1) {
            run_interactive_fallback(&runtime);
        } else {
            run_interactive(&runtime);
        }
    }
    
    if (log_level >= LOG_DEBUG && function_exists_name("dump")) {
        log_debug("\nfinal results");
        log_debug("-------------\n");
        function_t dump_func = function_from_name("dump");
        dump_func.function(&runtime);
    }


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