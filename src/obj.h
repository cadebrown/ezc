/* obj.h -- object header file

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

#ifndef __EZC_OBJ_H__
#define __EZC_OBJ_H__

#define TYPE_INT         0x000001
#define TYPE_DOUBLE      0x000002
#define TYPE_STR         0x000010
#define TYPE_MPF         0x000102

typedef struct obj_t {
    int type;

    void *val;
} obj_t;


void obj_fromstr(obj_t *r, char *val);

void str_obj_init(obj_t *r, char *val);
void str_obj_to_str(char *r, obj_t val);

void int_obj_init(obj_t *r, int val);
void int_obj_to_int(int *r, obj_t val);

void double_obj_init(obj_t *r, double val);
void double_obj_to_double(double *r, obj_t val);


#ifdef USE_GMP

void mpf_obj_init(obj_t *r, mpf_t val);
void mpf_obj_get_mpf(mpf_t *r, obj_t val);

#endif

#endif
