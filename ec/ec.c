

#include "ec.h"

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stddef.h>

int main(int argc, char** argv) {


    ezc_ctx ctx;
    ezc_ctx_init(&ctx);

    F_std_register_module(&ctx);

    int status = 0;
    int n_progs = 0;
    ezcp* progs = NULL;


    // add standard library funcs
    //ezc_ctx_addfunc(&ctx, EZC_STR_CONST("print"), ES_FNAME(print));
    //ezc_ctx_addfunc(&ctx, EZC_STR_CONST("repr"), ES_FNAME(repr));
    //ezc_ctx_addfunc(&ctx, EZC_STR_CONST("exec"), ES_FNAME(exec));

    int c;

    while ((c = getopt (argc, argv, "e:f:h")) != -1) {
        switch (c) {

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
    }

    if (optind < argc) {
        ezc_error("Unhandled arguments!");
    }

    //ezcp* prog = ezc_compile(EZC_STR_CONST("-"), EZC_STR_CONST("\"hello world\" print! test!"));

    //printf("%d\n", prog->inst_n);

    //ezc_exec(&ctx, prog);

    //ezc_exec(&ctx, EZC_STR_CONST("\"Hello World\" \"First\" print!print!"));

    //printf("s:%d\n", sizeof(ezc_obj));
    //ezc_info("test");
    /*
    ezc_obj A = EZC_OBJ_NONE;
    ezc_obj_init_str(&A, EZC_STR_CONST("Testing"));

    CTX_PUSH((&ctx), A);
    CTX_DUP((&ctx));

    es_print(&ctx);
    es_print(&ctx);
*/
    ezc_ctx_free(&ctx);

    return 0;
}




