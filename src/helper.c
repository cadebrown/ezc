/* helper.c -- utilities that are useful for the main program and parser

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

#include "ezc.h"

long long str_startswith(char *str, char *val, long long offset) {
    long long i = 0;
    while (i < strlen(val)) {
		  if (str[i + offset] != val[i]) return 0;
      i++;
    }
    return 1;
}

void ret_operator(char *out, char *val, long long *start) {
    long long i = 0;
    if (STR_STARTS(val, "==", (*start))) {
        out[i++] = '=';
        out[i++] = '=';
    } else if (STR_STARTS(val, "<<", (*start))) {
        out[i++] = '<';
        out[i++] = '<';
    } else if (STR_STARTS(val, ">>", (*start))) {
        out[i++] = '>';
        out[i++] = '>';
    } else if (IS_OP(val, (*start))) {
        out[i++] = val[(*start)];
    } else {
        ERR_STR(out);
		sprintf(out, "Unknown operator: %c", val[*start]);
		ezc_fail(out);
    }
    out[i] = 0;
    (*start) += strlen(out);
}

