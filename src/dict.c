/* dict.c -- a key,value dictionary implementation

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

void dict_init(EZC_DICT dict, EZC_INT len) {
	(*dict).ptr = 0;
	(*dict).size = len;
	(*dict).keys = (EZC_STR*)malloc(sizeof(EZC_STR) * len);
	(*dict).vals = (EZC_OBJ*)malloc(sizeof(EZC_STR) * len);
}


void dict_resize(EZC_DICT dict, EZC_INT len) {
	(*dict).size = len;
	(*dict).keys = (EZC_STR *)realloc((*dict).keys, sizeof(EZC_STR) * len);
	(*dict).vals = (EZC_OBJ *)realloc((*dict).vals, sizeof(EZC_OBJ *) * len);
}



bool dict_contains_key(EZC_DICT dict, EZC_STR key) {
	return dict_key_idx(dict, key) >= 0;
}

void dict_add_key(EZC_DICT dict, EZC_STR key) {
	if (!dict_contains_key(dict, key)) {
		dict_set(dict, key, EZC_NIL);
	}
}

EZC_IDX dict_key_idx(EZC_DICT dict, EZC_STR key) {
	EZC_INT i = 0;
	while (i < (*dict).ptr) {
		if (strcmp((*dict).keys[i], key) == 0) {
			return i;
		}
		i++;
	}
	return -1;
}

void dict_set(EZC_DICT dict, EZC_STR key, EZC_OBJ val) {
	EZC_IDX i = dict_key_idx(dict, key);
	if (i < 0) {
		(*dict).keys[(*dict).ptr] = key;
		(*dict).vals[(*dict).ptr] = val;
		(*dict).ptr++;
	} else {
		(*dict).vals[i] = val;
	}
}


bool dict_contains_val(EZC_DICT dict, EZC_OBJ val) {
	return dict_val_idx(dict, val) >= 0;
}

EZC_IDX dict_val_idx(EZC_DICT dict, EZC_OBJ val) {
	EZC_INT i = 0;
	while (i < (*dict).ptr) {
		if (obj_eq((*dict).vals[i], val)) {
			return i;
		}
		i++;
	}
	return -1;
}

EZC_OBJ dict_get(EZC_DICT dict, EZC_STR key) {
	EZC_INT i = dict_key_idx(dict, key);
	if (i < 0) {
		return NULL;
	} else {
		return (*dict).vals[i];
	}
}

