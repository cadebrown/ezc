/* mpf_obj.c -- a multiprecision object

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

#ifdef USE_GMP

#include "ezc_impl.h"

obj_t mpf_obj_init(mpf_t val) {
    obj_t r;
    mpf_init(r.val);
    mpf_set(r.val, val);
    r.type = TYPE_MPF;
    return r;
}

mpf_t mpf_obj_to_mpf(obj_t val) {
    mpf_t r;
    mpf_init(r);
    mpf_set(r, val.val);
    return r;
}

#endif

