/* ezc.h -- main header file for ec, or EZ calc

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

#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>

// TODO: should we make math.h optional?
#include <math.h>


#include "obj.h"
#include "common_types.h"


// Optional includes from configure.ac
#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef USE_GMP
    #include <gmp.h>
#endif

#ifdef USE_MPFR
    #include <mpfr.h>
#endif

// readline for completion/history
#ifdef USE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

typedef struct argp_t {
    bool has_error;

    bool show_help;
    bool show_info;
    bool show_version;

    bool is_interperet;

    char *exec_name;

    dynarray_t targets;
} argp_t;


void end();

void show_help();
void show_info();
void show_version();

void parse_args(int argc, char *argv[]);



argp_t arg_parse;

int main(int argc, char *argv[]);

#endif

