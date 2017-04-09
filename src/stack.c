/* stack.c -- stack implementation for pushing and popping values,
           -- reallocating needed memory, etc

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

void stk_init(EZC_STACK stk, EZC_INT len) {
	(*stk).ptr = 0;
	(*stk).size = len;
	(*stk).vals = (EZC_OBJ *)malloc(sizeof(EZC_OBJ) * len);
}

void stk_resize(EZC_STACK stk, EZC_INT len) {
	if ((*stk).size != len) {
		(*stk).size = len;
		(*stk).vals = (EZC_OBJ *)realloc((*stk).vals, sizeof(EZC_OBJ) * len);
	}
}


void stk_push(EZC_STACK stk, EZC_OBJ val) {
	stk_set(stk, (*stk).ptr, val);
	(*stk).ptr++;
}
void stk_set(EZC_STACK stk, EZC_INT pos, EZC_OBJ val) {
	(*stk).vals[pos] = val;
}


EZC_OBJ stk_pop(EZC_STACK stk) {
	(*stk).ptr--;
	return stk_get(stk, (*stk).ptr);
}

EZC_OBJ stk_get(EZC_STACK stk, EZC_INT pos) {
	return (*stk).vals[pos];
}

EZC_OBJ stk_get_recent(EZC_STACK stk, EZC_INT rpos) {
	return stk_get(stk, (*stk).ptr - 1 - rpos);
}
