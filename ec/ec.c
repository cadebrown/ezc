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
#include <unistd.h>

// force include the standard module (which should be linked wit -lezc)
#define EZC_MODULE_NAME std
#include "ezc-module.h"

// the prompt before each line you enter
#define EC_PROMPT " %> "


/* static variables */

static ezc_vm vm;

// if there is no readline support, run a non-interactive version
#ifndef EZC_HAVE_READLINE

void ec_run_repl(ezc_vm* vm) {
    ezc_str curline = EZC_STR_EMPTY;

    // read from stdin
    bool do_prompt = isatty(STDIN_FILENO);

    ezcp* progs = NULL;
    int n_progs = 0;

    if (do_prompt) printf("%s", EC_PROMPT);

    int lines = 0;
    for (;;) {

        char c = fgetc(stdin);
        if (c == EOF || c == '\n') {
            // execute
            int pidx = n_progs++;
            progs = realloc(progs, sizeof(ezcp) * n_progs);
            progs[pidx] = EZCP_EMPTY;
            ezcp_init(&progs[pidx], EZC_STR_CONST("-"), curline);

            int status = ezc_vm_exec(vm, progs[pidx]);

            ezc_str_copy(&curline, EZC_STR_CONST(""));

        } else {
            ezc_str_append_c(&curline, c);
        }

        if (c == '\n' && do_prompt) printf("%s", EC_PROMPT);
        if (c == EOF) break;

    }

    free(progs);

    //while (getline(&line, &size, stdin) != -1) {
    //    if (do_prompt) printf("%s", EC_PROMPT)0;
    //    lines++;
    //}
}


#else
// use readline autocomplete

// readline headers
#include <readline/readline.h>
#include <readline/history.h>

// readline match generator
char* ec_rl_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    // if state is zero, we are just beginning
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    // return every match that occurs
    while ((name = vm.funcs.keys[list_index++]._)) {
        if (strncmp(name, text, len) == 0) {
            char* new_name = malloc(strlen(name) + 2);
            sprintf(new_name, "%s!", name);
            return new_name;
        }
    }
    // TODO: Also autocomplete with variables in the global dictionary

    // if nothing is found this iteration, return NULL
    return NULL;
}

// just call our generator
char** ec_rl_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, ec_rl_generator);
}

// begins a repl, using readline for easier usage
void ec_run_repl_rl(ezc_vm* vm) {

    ezcp* progs = NULL;
    int n_progs = 0;

    char * cur_line = NULL;

    //rl_parse_and_bind("TAB: menu-complete");

    rl_attempted_completion_function = ec_rl_completion;
    rl_basic_word_break_characters = "\t\n\"\\'`+-*/%^<>0123456789!@$=;|{(# ";

    bool do_prompt = isatty(STDIN_FILENO);

    char * interact = NULL;

    if (do_prompt) {
        interact = EC_PROMPT;
    }

    while ((cur_line = readline(interact)) != NULL) {
        //runnable_free(&cur_runnable);
        int pidx = n_progs++;
        progs = realloc(progs, sizeof(ezcp) * n_progs);
        progs[pidx] = EZCP_EMPTY;
        ezcp_init(&progs[pidx], EZC_STR_CONST("-"), EZC_STR_VIEW(cur_line, strlen(cur_line)));

        int status = ezc_vm_exec(vm, progs[pidx]);

        // this will allow readline to display prior command when the user hits the up arrow
        if (cur_line && *cur_line) add_history(cur_line);
        free(cur_line);
    }

    free(progs);
}

#endif

int main(int argc, char** argv) {

    ezc_init();

    vm = EZC_VM_EMPTY;

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
        {"INTERACTIVE_PROMPT", no_argument, NULL, 'i'},
        {"all", no_argument, NULL, 'A'},
        {"v", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},

        {NULL, 0, NULL, 0}
    };

    int c;

    while ((c = getopt_long (argc, argv, "e:f:Aivh", long_options, NULL)) != -1)
    switch (c){
        case 'A':
            fA = true;
            break;
        case 'i':
            #ifdef EZC_HAVE_READLINE
            ec_run_repl_rl(&vm);
            #else
            ec_run_repl(&vm);
            #endif
            break;
        case 'e':
            progs = ezc_realloc(progs, sizeof(ezcp) * ++n_progs);
            progs[n_progs - 1] = EZCP_EMPTY;
            ezcp_init(&progs[n_progs - 1], EZC_STR_CONST("-e"), EZC_STR_CONST(optarg));
            ezc_debug("Running `-e`: '%s' (compiled to %d instructions)", progs[n_progs - 1].src._, progs[n_progs - 1].body._block.n);
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
                ezc_debug("Running `-f`: %s (compiled to %d instructions)", progs[n_progs - 1].src_name._, progs[n_progs - 1].body._block.n);
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
            printf("Usage: %s [-e expr|-f file] [-A,-h] [files...]\n", argv[0]);
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

    while (optind < argc) {
        // treat the input as a file
        optarg = argv[optind];
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
            ezc_debug("Running `-f`: %s (compiled to %d instructions)", progs[n_progs - 1].src_name._, progs[n_progs - 1].body._block.n);
            ezcp_init(&progs[n_progs - 1], EZC_STR_CONST(optarg), EZC_STR_VIEW(src, size));
            ezc_free(src);
            ezc_vm_exec(&vm, progs[n_progs - 1]);
        }

        optind++;
    }
    // shouldn't happen
    if (optind < argc) {
        ezc_error("Unhandled arguments!");
    }

    // print the entire stack
    if (fA) {
        ezcp printall_p = EZCP_EMPTY;
        ezcp_init(&printall_p, EZC_STR_CONST("__printall"), EZC_STR_CONST("dump!"));
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




