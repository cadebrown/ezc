
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>

#include "ezc.h"

#ifdef WINDOWS
#include <direct.h>
#define getcwd _getcwd
#endif

#include "ezcsymboldefs.h"
 
void ezc_init(char * execname) {
    char * cwd = malloc(FILENAME_MAX);
    getcwd(cwd, FILENAME_MAX);
    add_search_path(cwd);

    if (execname != NULL) {
        int length, dirname_length;
        length = wai_getExecutablePath(NULL, 0, &dirname_length);

        char * execdir = malloc(length + 1);
        wai_getExecutablePath(execdir, length, &dirname_length);
        execdir[dirname_length] = '\0';

        add_search_path(execdir);

        char * libdir = malloc(FILENAME_MAX);
        sprintf(libdir, "%s/../lib/ezc", execdir);
        add_search_path(libdir);
    }

    init_module_loader();
    /*
    import_module("systypes");
    import_module("dummy");
    import_module("printstack");
    */

    /*

    type_t str;
    if (!type_exists_name("str")) {
        printf("No 'str' found!\n");
    } else {
        str = type_from_name("str");
    }

    obj_t A_str;
    str.parser(&A_str, "Hello world!");

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

    /*
    obj_t A_dummy;
    type_t dummy;

    dummy = type_from_name("dummy");

    dummy.constructor(&A_dummy, A_str);

    char * rval;

    dummy.representation(&A_dummy, &rval);

    printf("return value: '%s'\n", rval);
    */
}

