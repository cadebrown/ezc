// ezc/ec.c - the commandline interface to run EZC
//
// Just defines a `main` to be ran
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-21
//

#include "ec.h"

// just include standard library for IO/printing/etc
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <getopt.h>

// force include the standard module (which should be linked wit -lezc)
#define EZC_MODULE_NAME std
#include "ezc-module.h"

int main(int argc, char** argv) {

    ezc_init();

    // initialize the global context
    ezc_vm vm = EZC_VM_EMPTY;

    // this should be defined by `ezc-module.h`, for module standard 
    F_std_register_module(&vm);

    // here are the programs to be executed
    int status = 0;
    int n_progs = 0;
    ezcp* progs = NULL;

    // storing flags
    bool fA = false;

    // long options for commandline parsing
    static struct option long_options[] = {
        {"expr", required_argument, NULL, 'e'},
        {"file", required_argument, NULL, 'f'},
        {"all", no_argument, NULL, 'A'},
        {"v", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},

        {NULL, 0, NULL, 0}
    };

    int c;

    while ((c = getopt_long (argc, argv, "e:f:Avh", long_options, NULL)) != -1)
    switch (c){
        case 'A':
            fA = true;
            break;
        case 'e':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_EMPTY;
            ezcp_init(&progs[n_progs - 1], EZC_STR_CONST("-e"), EZC_STR_CONST(optarg));
            ezc_vm_exec(&vm, progs[n_progs - 1]);
            break;
        case 'f':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_EMPTY;
            FILE* fp = fopen(optarg, "r");
            if (fp == NULL) {
                ezc_error("Couldn't open file '%s'", optarg);
                return 1;
            } else {
                fseek(fp, 0, SEEK_END);
                int size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                char* src = ezc_malloc(size+1);
                if (size != fread(src, 1, size, fp)) ezc_warn("File wasn't read correctly... '%s'", optarg);
                src[size] = '\0';
                ezcp_init(&progs[n_progs - 1], EZC_STR_CONST(optarg), EZC_STR_VIEW(src, size));
                ezc_free(src);
                ezc_vm_exec(&vm, progs[n_progs - 1]);
            }
            break;
        case 'v':
            // get more verbose
            ezc_log_set_level(ezc_log_get_level() - 1);
            break;
        case 'h':
            printf("Usage: %s [-e expr | -f file] [-A,-h]\n", argv[0]);
            printf("  -h,--help              Prints this help message\n");
            printf("  -e,--expr [EXPR]       Compiles [EXPR], then executes it\n");
            printf("  -f,--file [FILE]       Reads [FILE], compiles it, then executes it\n");
            printf("  -A,--all               Prints out the entire stack after execution\n");
            return 0;
            break;
        case '?':
            if (strchr("e", optopt) != NULL) {
                ezc_error("Option -%c requires an argument.", optopt);
            } else {
                ezc_error("Unknown option `-%c'.", optopt);
            }
            return 1;
            break;
        default:
            return 1;
            break;
    }

    if (optind < argc) {
        ezc_error("Unhandled arguments!");
    }

    // print the entire stack
    if (fA) {
        ezcp printall_p = EZCP_EMPTY;
        ezcp_init(&printall_p, EZC_STR_CONST("__printall"), EZC_STR_CONST("printall!"));
        ezc_vm_exec(&vm, printall_p);
    } else {
        // just print top
        if (vm.stk.n > 0) {
            ezcp print_p = EZCP_EMPTY;
            ezcp_init(&print_p, EZC_STR_CONST("__print"), EZC_STR_CONST("print!"));
            ezc_vm_exec(&vm, print_p);
        } else {
            // print nothing
        }
    }



    // free and finalize the library
    ezc_vm_free(&vm);
    ezc_finalize();

    return 0;
}




