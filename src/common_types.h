/* common_types.h -- defines common objects for EZC, including dynamic stacks
                  -- dictionaries, objects, etc

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

#ifndef __EZC_COMMON_TYPES_H__
#define __EZC_COMMON_TYPES_H__

#define K_DARRAY_SLEN 0

#define dynarray_init(dar, type) dar.size = K_DARRAY_SLEN; dar.vals = malloc((dar.size) * sizeof(type *));
#define dynarray_resize(dar, type, len) dar.vals = realloc(dar.vals, (len) * sizeof(type *)); dar.size = len;

#define dynarray_get(dar, type, ptr) ((type *)(dar.vals))[ptr]
#define dynarray_set(dar, type, ptr, val) if (ptr >= dar.size) { dynarray_resize(dar, type, ptr + 1); } (dar.vals)[ptr] = val;

typedef struct dynarray_t {
    int size;
    int ptr;

    void **vals;
} dynarray_t;

#define RUN_HOWTO_READLINE (2)
#define RUN_HOWTO_EXPR     (3)
#define RUN_HOWTO_FILE     (4)

// for expressions, files, and console input
typedef struct run_t {
  int howto;
  char *goal;
} run_t;


#endif
