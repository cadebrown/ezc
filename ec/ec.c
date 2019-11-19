

#include "ec.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <getopt.h>

// force include the standard module
#define EZC_MODULE_NAME std
#include "ezc-module.h"

int main(int argc, char** argv) {

    // initialize the global context
    ezc_ctx ctx;
    ezc_ctx_init(&ctx);

    // this should be defined by `ezc-module.h`, for module standard 
    F_std_register_module(&ctx);

    // here are the programs to be executed
    int status = 0;
    int n_progs = 0;
    ezcp* progs = NULL;

    // storing flags
    bool fA = false;

    // long options
    static struct option long_options[] = {
        {"expr", required_argument, NULL, 'e'},
        {"file", required_argument, NULL, 'f'},
        {"all", no_argument, NULL, 'A'},
        {"help", no_argument, NULL, 'h'},

        {NULL, 0, NULL, 0}
    };

    int c;

    while ((c = getopt_long (argc, argv, "e:f:Ah", long_options, NULL)) != -1)
    switch (c){
        case 'A':
            fA = true;
            break;
        case 'e':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_NONE;
            status = ezc_compile(&progs[n_progs - 1], EZC_STR_CONST("-e"), EZC_STR_CONST(optarg));

            if (status == 0) {
                ezc_exec(&ctx, &progs[n_progs - 1]);
            } else {
                return 1;
            }
            break;
        case 'f':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_NONE;
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
                status = ezc_compile(&progs[n_progs - 1], EZC_STR_CONST(optarg), EZC_STR_VIEW(src, size));
                ezc_free(src);
                if (status == 0) {
                    ezc_exec(&ctx, &progs[n_progs - 1]);
                } else {
                    return 1;
                }
            }
            break;
        case 'h':
            printf("Usage: %s [-e expr | -f file] [-A,-h]\n", argv[0]);
            printf("  -h,--help              Prints this help message\n");
            printf("  -e,--expr [EXPR]       Executes [EXPR], which is a valid EZC expression\n");
            printf("  -f,--file [FILE]       Reads \n");
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
/*
    while ((c = getopt (argc, argv, "e:f:h")) != -1) switch (c) {
        case 'e':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_NONE;
            status = ezc_compile(&progs[n_progs - 1], EZC_STR_CONST("-e"), EZC_STR_CONST(optarg));
            if (status == 0) {
                ezc_exec(&ctx, &progs[n_progs - 1]);
            } else {
                return 1;
            }
            break;
        case 'f':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_NONE;
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
                status = ezc_compile(&progs[n_progs - 1], EZC_STR_CONST(optarg), EZC_STR_VIEW(src, size));
                ezc_free(src);
                if (status == 0) {
                    ezc_exec(&ctx, &progs[n_progs - 1]);
                } else {
                    return 1;
                }
            }
            break;
        case 'h':
            printf("Usage: %s [-e expr | -f file] [-h]\n", argv[0]);
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

    */
    if (optind < argc) {
        ezc_error("Unhandled arguments!");
    }

    // print the entire stack
    if (fA) {
        ezcp printall_p = EZCP_NONE;
        ezc_compile(&printall_p, EZC_STR_CONST("__printall"), EZC_STR_CONST("printall!"));
        ezc_exec(&ctx, &printall_p);
    } else {
        // just print top
        if (ctx.stk.n > 0) {

            ezcp print_p = EZCP_NONE;
            ezc_compile(&print_p, EZC_STR_CONST("__print"), EZC_STR_CONST("print!"));
            ezc_exec(&ctx, &print_p);
        } else {
            // just blank it
        }
    }

    ezc_ctx_free(&ctx);

    return 0;
}




