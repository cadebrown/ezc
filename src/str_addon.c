
#include "ezc.h"


EZC_OBJ str_literal(EZC_STR x) {
	CREATE_OBJ(ret);
	SET_OBJ(ret, TYPE_STR, x);
	return ret;
}

