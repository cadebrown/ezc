/* double_obj.c -- an double object

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

void double_obj_init(obj_t *r, double val) {
    double *_r_val = malloc(sizeof(double));
    (*_r_val) = val;
    (*r).val = _r_val;
    (*r).type = TYPE_DOUBLE;
}

void double_obj_to_double(double *r, obj_t val) {
    (*r) = *(double *)val.val;
}

