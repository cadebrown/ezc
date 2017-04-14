/* ezc.c -- main file for ec, or EZ calc

  Copyright 2016-2017 ChemicalDevelopment

  This file is part of the EZC project.

  EZC, EC, EZC Documentation, and any other resources in this project are 
free software; you are free to redistribute it and/or modify them under 
the terms of the GNU General Public License; either version 3 of the 
license, or any later version.

  These programs are hopefully useful and reliable, but it is understood 
that these are provided WITHOUT ANY WARRANTY, or MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GPLv3 or email at 
<info@chemicaldevelopment.us> for more info on this.

  Here is a copy of the GPL v3, which this software is licensed under. You 
can also find a copy at http://www.gnu.org/licenses/.

*/

#include "ezc_impl.h"

void show_help() {
    printf(KCYN "Usage: " KGRN "%s" KCYN " [- | -e [EXPR] | -f [FILE]] [OPTIONS ...]" KNRM "\n", arg_parse.exec_name);
    printf("\n");
    printf("  -                                  Run an interactive shell\n");
    printf("  -e, --expr                         Run an expression\n");
    printf("  -f, --file                         Run file contents\n");
    printf("\n");
    printf("  -v, --version                      Print version and then quit\n");
    printf("  -i, --info                         Print info about this binary and then quit\n");
    printf("\n");
    printf("<" PACKAGE_BUGREPORT ">\n");
}

void show_info() {
    printf(PACKAGE_NAME "\n");
    printf("Version: " PACKAGE_VERSION "\n");
    printf("\n");
    printf("Compiled with:\n");
    #ifdef USE_GMP
        printf("    GMP (version %s)\n", gmp_version);
    #endif
    #ifdef USE_MPFR
        printf("    MPFR (version " MPFR_VERSION_STRING ")\n");
    #endif
    #ifdef USE_READLINE
        printf("    Readline\n");
    #endif

    printf("\n");
    printf("<" PACKAGE_BUGREPORT ">\n");
}

void show_version() {
    printf(PACKAGE_NAME "\n");
    printf("Version: " PACKAGE_VERSION "\n");
    printf("\n");
    printf("<" PACKAGE_BUGREPORT ">\n");
}


void parse_args(int argc, char *argv[]) {
    int i = 1;

    int cur_target_ptr = 0;

    arg_parse.exec_name = argv[0];

    arg_parse.show_help = false;
    arg_parse.show_info = false;
    arg_parse.show_version = false;

    dynarray_init(arg_parse.targets, run_t *);

    while (i < argc) {
        if (STR_EQ(argv[i], "-h") || STR_EQ(argv[i], "--help")) {
            arg_parse.show_help = true;
            i++;
        } else if (STR_EQ(argv[i], "-v") || STR_EQ(argv[i], "--version")) {
            arg_parse.show_version = true;
            i++;
        } else if (STR_EQ(argv[i], "-i") || STR_EQ(argv[i], "--info")) {
            arg_parse.show_info = true;
            i++;
        } else if (STR_EQ(argv[i], "-e") || STR_EQ(argv[i], "--expr")) {
            i++;
            run_t *cur_target = malloc(sizeof(run_t));
            (*cur_target).howto = RUN_HOWTO_EXPR;
            (*cur_target).goal = argv[i];
            dynarray_set(arg_parse.targets, run_t *, cur_target_ptr, cur_target);
            cur_target_ptr++;
            i++;
        } else if (STR_EQ(argv[i], "-f") || STR_EQ(argv[i], "--file")) {
            i++;
            run_t *cur_target = malloc(sizeof(run_t));
            (*cur_target).howto = RUN_HOWTO_FILE;
            (*cur_target).goal = argv[i];
            dynarray_set(arg_parse.targets, run_t *, cur_target_ptr, cur_target);
            cur_target_ptr++;
            i++;
        } else if (STR_EQ(argv[i], "-")) {
            i++;
            run_t *cur_target = malloc(sizeof(run_t));
            (*cur_target).howto = RUN_HOWTO_READLINE;
            (*cur_target).goal = "";
            dynarray_set(arg_parse.targets, run_t *, cur_target_ptr, cur_target);
            cur_target_ptr++;
            i++;            
        } else {
            // fail here
            printf(KRED "Unrecognized option[s]: `%s`\n" KNRM, argv[i]);
            arg_parse.show_help = true;
            return;
        }
    }
}


