/* ezc_impl.h -- source development file for ezc

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

#ifndef __EZC_IMPL_H__
#define __EZC_IMPL_H__

#include "ezc.h"

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define STRSIZE(bitsize, strbase) (int)((bitsize) * log(2.0) / log(strbase))
#define BITSIZE(strsize, strbase)  (lint)((strsize) * log(strbase) / log(2.0))

#define IS_SPACE(ch) (ch == ' ' || ch == ',' || ch == '\n')
#define IS_CONST(ch) ((ch - '0' >= 0 && ch - '9' <= 0))
#define IS_ALPHA(ch) ((ch - 'a' >= 0 && ch - 'z' <= 0) || (ch - 'A' >= 0 && ch - 'Z' <= 0))

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#endif

