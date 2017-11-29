
/*

Taken from: https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

Licensed under MIT

Copyright 2015 by Mark Whitis.  License=MIT style


*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>


int findexecdir(char *argv0, char *result, size_t size_of_result);

