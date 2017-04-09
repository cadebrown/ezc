/* obj.c -- A generic object class, includes IO, serializing, etc

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


void obj_dump_raw(EZC_OBJ obj) {
	obj_dump_fmt(obj, 0, 0, 0, true, false);
}

void obj_dump(EZC_OBJ obj) {
	obj_dump_fmt(obj, PRINT_COL_OFFSET, 0, PRINT_OFFSET, false, true);
}


void obj_dump_fmt(EZC_OBJ obj, long long coloff, long long offset, long long addeach, bool raw, bool print_quotes) {
	_SPACE(offset);
	if ((*obj).type == TYPE_NIL) {
		printf("nil");
	} else if ((*obj).type == TYPE_INT) {
		printf("%lld", (long long)(*obj).val);
	} else if ((*obj).type == TYPE_BOOL) {
		if ((long long)(*obj).val == 0) {
			printf("false");
		} else {
			printf("true");
		}
	} 
	#ifdef USE_GMP
		else if ((*obj).type == TYPE_MPZ) {
			if (raw) {
				gmp_printf("%Zd", (*obj).val);

			} else {
				gmp_printf("%Zd [Z]", (*obj).val);
			}
		} 
	#endif
	else if ((*obj).type == TYPE_STR) {
		if (raw && !print_quotes) {
			printf("%s", (char *)(*obj).val);
		} else {
			printf("\"%s\"", (char *)(*obj).val);
		}
	} else if ((*obj).type == TYPE_STK) {
		EZC_INT i = 0;
		EZC_STACK stk = (EZC_STACK)(*obj).val;
		if (!raw) {
			printf("\n");
			_SPACE(offset);
		}
		printf("[");
		if (!raw) {
			printf("\n");
		}
		while (i < (*stk).ptr) {
			if (!raw) {
				_SPACE(offset+addeach/2);
				printf("(%lld): ", i);
			}
			obj_dump_fmt((*stk).vals[i], coloff, offset, addeach, raw, print_quotes);
			if (i != (*stk).ptr - 1) {
				printf(",");
			}
			if (!raw) {
				printf("\n");
			}
			i++;
		}
		if (!raw) {
			_SPACE(offset);
		}
		printf("]\n");
	} else if ((*obj).type == TYPE_DICT) {
		EZC_INT i = 0;
		EZC_DICT dict = (EZC_DICT)(*obj).val;
		printf("{");
		if (!raw) {
			printf("\n");
		}
		while (i < (*dict).ptr) {
			if (!raw) _SPACE(offset+addeach/2);
			if (!raw && print_quotes) printf("\"");
			printf("%s", (*dict).keys[i]);
			if (!raw && print_quotes) printf("\"");
			if (!raw) _SPACE(coloff-strlen((*dict).keys[i])-2);
			printf(": ");
			obj_dump_fmt((*dict).vals[i], coloff, offset, addeach, raw, print_quotes);
			printf("\n");

			i++;
		}
		printf("}");
		if (!raw) {
			printf("\n");
		}
	}
}


EZC_STR str_from_obj(EZC_OBJ v) {
	if ((*v).type == TYPE_NIL) {
		return "nil";
	} else if ((*v).type == TYPE_BOOL) {
		if ((*v).val == 0) {
			return "false";
		} else {
			return "true";
		}
	} else if ((*v).type == TYPE_INT) {
		char *ret = (char *)malloc(30);
		sprintf(ret, "%lld", (EZC_INT)((*v).val));
		return ret;
	} else if ((*v).type == TYPE_STR) {
		return (char *)(*v).val;
	}
	#ifdef USE_GMP
		else if ((*v).type == TYPE_MPZ) {
			return mpz_get_str(NULL, 10, (*v).val);
		} 
	#endif
	else {
		return "nil";
	}
}


EZC_OBJ obj_from_str(EZC_STR s) {
	if (STR_EQ(s, "nil")) {
		return EZC_NIL;
	} else if (STR_EQ(s, "true")) {
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_BOOL, 1);
		return ret;
	} else if (STR_EQ(s, "false")) {
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_BOOL, 0);
		return ret;
	} else if (IS_DIGIT(s[0]) || (s[0] == '-' && IS_DIGIT(s[1]))) {
		CREATE_OBJ(ret);
		if (s[strlen(s)-1] == 'Z') {
			#ifdef USE_GMP
				s[strlen(s)-1] = 0;
				EZC_MP retz = (EZC_MP)malloc(sizeof(EZC_MP));
				mpz_init(*retz);
				mpz_set_str(*retz, s, 10);
				SET_OBJ(ret, TYPE_MPZ, retz);
				return ret;
			#else
				ERR_STR(out);
				sprintf(out, "Trying to create multiprecision integer, but has not been compiled with GMP");
				ezc_fail(out);
			#endif
		} else {
			SET_OBJ(ret, TYPE_INT, strtoll(s, NULL, 10));
		}
		return ret;
	} else if (s[0] == '"') {
		char *new = malloc(strlen(s)-1);
		memcpy(new, &s[1], strlen(s)-2);
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_STR, new);
		return ret;
	} else {
		printf("Don't know the type for string: [%s]\n", s);
		return EZC_NIL;
	}
}

bool obj_eq(EZC_OBJ a, EZC_OBJ b) {
	return obj_cmp(a, b) == 0;
}

EZC_INT obj_cmp(EZC_OBJ a, EZC_OBJ b) {
	if ((*a).type != (*b).type) {
		return false;
	}
	EZC_TYPE type = (*a).type;
	if (type == TYPE_NIL) {
		return 0;
	} else if (type == TYPE_INT) {
		return ((EZC_INT)((*a).val) < (EZC_INT)((*b).val)) ? -1 : (EZC_INT)((*a).val) > (EZC_INT)((*b).val);
	} else if (type == TYPE_BOOL) {
		return ((bool)((*a).val) < (bool)((*b).val)) ? -1 : (bool)((*a).val) > (bool)((*b).val);
	} else if (type == TYPE_STR) {
		return strcmp((EZC_STR)((*a).val), (EZC_STR)((*b).val));
	} else {
		return -1;
	}
}

void obj_cpy(EZC_OBJ a, EZC_OBJ b) {
	if ((*a).type != (*b).type) {
		printf("Trying to copy two different types\n");
		return;
	}
	EZC_TYPE type = (*a).type;
	if (type == TYPE_INT) {
		(*b).val = (*a).val;
	} else if (type == TYPE_BOOL) {
		(*b).val = (*a).val;
	} else if (type == TYPE_STR) {
		strcpy((*b).val, (*a).val);
	}
}

char * get_type_name(EZC_TYPE type) {
	if (type == TYPE_NIL) {
		return "nil";
	} else if (type == TYPE_BOOL) {
		return "bool";
	} else if (type == TYPE_FLT) {
		return "float";
	} else if (type == TYPE_INT) {
		return "int";
	} else if (type == TYPE_STR) {
		return "str";
	} else if (type == TYPE_STK) {
		return "stack";
	} else if (type == TYPE_DICT) {
		return "dict";
	}
	#ifdef USE_GMP
		else if (type == TYPE_MPZ) {
			return "mpz";
		}
	#endif 
	else {
		ERR_STR(out);
		sprintf(out, "Unknown type (0x%x)", (unsigned int)type);
		ezc_fail(out);
		return "unknown";
	}
}

