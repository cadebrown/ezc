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
		printf("%lld", *(EZC_INT *)(*obj).val);
	} else if ((*obj).type == TYPE_FLOAT) {
		printf("%f", *(EZC_FLOAT *)(*obj).val);
	} else if ((*obj).type == TYPE_BOOL) {
		if ((long long)(*obj).val == 0) {
			printf("false");
		} else {
			printf("true");
		}
	} else if ((*obj).type == TYPE_MPZ) {
		#ifdef USE_GMP
			if (raw) {
				gmp_printf("%Zd", (*obj).val);

			} else {
				gmp_printf("%Zd [Z]", (*obj).val);
			}
		#else
			ERR_STR(out);
			sprintf(out, "Trying to print an mpz, but wasnt compiled with GMP");
			ezc_fail(out);
		#endif
	} else if ((*obj).type == TYPE_MPF) {
		#ifdef USE_GMP
			if (raw) {
				gmp_printf("%.Ff", (*obj).val);

			} else {
				gmp_printf("%.Ff [F]", (*obj).val);
			}
		#else
			ERR_STR(out);
			sprintf(out, "Trying to print an mpf, but wasnt compiled with GMP");
			ezc_fail(out);
		#endif
	} else if ((*obj).type == TYPE_MPFR) {
		#ifdef USE_GMP
			#ifdef USE_MPFR
				if (raw) {
					mpfr_printf("%.Rf", (*obj).val);
				} else {
					mpfr_printf("%.*Rf [FR]", EZC_STRSIZE(mpfr_get_prec(*(EZC_MPFR *)(*obj).val), 10), (*obj).val);
				}
			#else
				ERR_STR(out);
				sprintf(out, "Trying to print an mpfr, but wasnt compiled with MPFR (does have GMP, though)");
				ezc_fail(out);
			#endif
		#else
			ERR_STR(out);
			sprintf(out, "Trying to print an mpfr, but wasnt compiled with GMP or MPFR");
			ezc_fail(out);
		#endif
	} 
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
	} else if ((*v).type == TYPE_FLOAT) {
		char *ret = (char *)malloc(30);
		sprintf(ret, "%f", *(EZC_FLOAT *)((*v).val));
		return ret;
	} else if ((*v).type == TYPE_STR) {
		return (char *)(*v).val;
	}
	#ifdef USE_GMP
		else if ((*v).type == TYPE_MPZ) {
			return mpz_get_str(NULL, 10, (*v).val);
		} else if ((*v).type == TYPE_MPF) {
			#ifdef USE_MPFR
				char *ret = (char *)malloc(EZC_STRSIZE(mpfr_get_prec(*(EZC_MPFR *)(*v).val), 10));
				mpfr_sprintf(ret, "%Rf", (*v).val);
				return ret;
			#else
				return mpf_get_str(NULL, NULL, 10, 0, (*v).val);
			#endif
		}
	#endif
	else {
		return "nil";
	}
}

EZC_STR remove_suffix(EZC_STR val) {
	long long i = strlen(val)-1;
	long long start = 0;
	while (i > 0 && val[i] != ':') i--;

	if (val[i] != ':') {
		char *ret = (char *)malloc(strlen(val));
		strcpy(ret, val);
		return ret;
	} else {
		EZC_STR ret = (char *)malloc(i);
		while (start < i) {
			ret[start] = val[start];
			start++;
		}
		return ret;
	}
}

EZC_STR parse_suffix(EZC_STR val) {
	long long i = strlen(val)-1;
	while (i > 0 && val[i] != ':') i--;
	if (val[i] != ':') {
		return "unknown";
	} else {
		long long start = i+1;
		EZC_STR ret = (char *)malloc(strlen(val)-start+1);
		while (start < strlen(val)) {
			ret[start-i-1] = val[start];
			start++;
		}
		ret[start] = 0;
		return ret;
	}
}

EZC_TYPE type_from_str_suffix(EZC_STR suffix) {
	if (STR_EQ(suffix, "unknown")) {
		return TYPE_UNKNOWN;
	} if (STR_EQ(suffix, "i") || STR_EQ(suffix, "int")) {
		return TYPE_INT;
	} else if (STR_EQ(suffix, "f") || STR_EQ(suffix, "float")) {
		return TYPE_FLOAT;
	} else if (STR_EQ(suffix, "str")) {
		return TYPE_STR;
	} else if (STR_EQ(suffix, "bool")) {
		return TYPE_BOOL;
	} else if (STR_EQ(suffix, "nil")) {
		return TYPE_NIL;
	} else if (STR_EQ(suffix, "Z") || STR_EQ(suffix, "mpz")) {
		return TYPE_MPZ;
	}  else if (STR_EQ(suffix, "mpf")) {
		return TYPE_MPF;
	}  else if (STR_EQ(suffix, "mpfr")) {
		return TYPE_MPFR;
	}
	// if MPFR is enabled, F defaults to mpfr_t
	#ifdef USE_MPFR
		else if (STR_EQ(suffix, "F")) {
			return TYPE_MPFR;
		}
	#else
		else if (STR_EQ(suffix, "F")) {
			return TYPE_MPF;
		}
	#endif
	else {
		return TYPE_UNKNOWN;
	}
}

EZC_OBJ obj_from_str(EZC_STR s) {
	EZC_TYPE s_type = type_from_str_suffix(parse_suffix(s));
	EZC_STR s_val = remove_suffix(s);
	if (s_type == TYPE_UNKNOWN) {
		if (STR_EQ(s, "nil")) {
			s_type = TYPE_NIL;
		} else if (STR_EQ(s, "true") || STR_EQ(s, "false")) {
			s_type = TYPE_BOOL;
		} else if (s[0] == '"') {
			s_type = TYPE_STR;
		} else if (IS_CONST(s[0]) || (s[0] == '-' && IS_CONST(s[1]))) {
			long long i = 0;
			bool has_had_point = false;
			while (i < strlen(s_val)) {
				if (s_val[i] == '.') {
					has_had_point = true;
				}
				i++;
			}
			if (has_had_point) {
				if (strlen(s_val) >= 20) {
					#ifdef USE_GMP
						#ifdef USE_MPFR
							s_type = TYPE_MPFR;
						#else
							s_type = TYPE_MPF;
						#endif
					#else
						s_type = TYPE_FLOAT;
					#endif
				} else {
					s_type = TYPE_FLOAT;
				}
			} else {
				if (strlen(s_val) >= 20) {
					#ifdef USE_GMP
						s_type = TYPE_MPZ;
					#else
						s_type = TYPE_INT;
					#endif
				} else {
					s_type = TYPE_INT;
				}
			}
		} else {
			ERR_STR(out);
			sprintf(out, "Couldn't autodetermine type for value: %s", s);
			ezc_fail(out);
		}
	}

	if (s_type == TYPE_NIL) {
		return EZC_NIL;
	} else if (s_type == TYPE_BOOL) {
		printf(";%s;\n", s_val);
		if (STR_EQ(s_val, "true")) {
			CREATE_OBJ(ret);
			SET_OBJ(ret, TYPE_BOOL, 1);
			return ret;
		} else if (STR_EQ(s_val, "false")) {
			CREATE_OBJ(ret);
			SET_OBJ(ret, TYPE_BOOL, 0);
			return ret;
		}
	// todo: detect whether they entered "" or `` '' or something else
	} else if (s_type == TYPE_STR) {
		char *new = malloc(strlen(s_val)-1);
		memcpy(new, &s_val[1], strlen(s_val)-2);
		CREATE_OBJ(ret);
		SET_OBJ(ret, TYPE_STR, new);
		return ret;
	} else if (s_type == TYPE_INT) {
		CREATE_OBJ(ret);
		EZC_INT *retl = (EZC_INT  *)malloc(sizeof(EZC_INT));
		(*retl) = strtoll(s_val, NULL, 10);
		SET_OBJ(ret, TYPE_INT, retl);
		return ret;
	} else if (s_type == TYPE_FLOAT) {
		CREATE_OBJ(ret);
		EZC_FLOAT *retf = (EZC_FLOAT *)malloc(sizeof(EZC_FLOAT));
		(*retf) = atof(s_val);
		SET_OBJ(ret, TYPE_FLOAT, retf);
		return ret;
	} else if (s_type == TYPE_MPZ) {
		#ifdef USE_GMP
			CREATE_OBJ(ret);
			EZC_MPZ *retz = (EZC_MPZ *)malloc(sizeof(EZC_MPZ));
			mpz_init(*retz);
			mpz_set_str(*retz, s_val, 10);
			SET_OBJ(ret, TYPE_MPZ, retz);
			return ret;
		#else
			ERR_STR(out);
			sprintf(out, "Trying to create mpz, but has not been compiled with GMP");
			ezc_fail(out);
		#endif
	} else if (s_type == TYPE_MPF) {
		#ifdef USE_GMP
			CREATE_OBJ(ret);
			EZC_MPF *retf = (EZC_MPF *)malloc(sizeof(EZC_MPF));
			mpf_init(*retf);
			mpf_set_prec(*retf, EZC_BITSIZE(strlen(s_val), 10));
			mpf_set_str(*retf, s_val, 10);
			SET_OBJ(ret, TYPE_MPF, retf);
			return ret;
		#else
			ERR_STR(out);
			sprintf(out, "Trying to create mpf, but has not been compiled with GMP");
			ezc_fail(out);
		#endif
	} else if (s_type == TYPE_MPFR) {
		#ifdef USE_GMP
			#ifdef USE_MPFR
				CREATE_OBJ(ret);
				EZC_MPFR *retfr = (EZC_MPFR *)malloc(sizeof(EZC_MPFR));
				mpfr_init2(*retfr, EZC_BITSIZE(strlen(s_val), 10));
				mpfr_set_str(*retfr, s_val, 10, EZC_RND);
				SET_OBJ(ret, TYPE_MPFR, retfr);
				return ret;
			#else
				//todo: add fallback to MPF with warning message
				ERR_STR(out);
				sprintf(out, "Trying to create mpfr, but has not been compiled with MPFR (does have GMP, though)");
				ezc_fail(out);
			#endif
		#else
			ERR_STR(out);
			sprintf(out, "Trying to create mpfr, but has not been compiled with GMP or MPFR");
			ezc_fail(out);
		#endif
	}
	ERR_STR(out);
	sprintf(out, "Dont know how to get object from type: %s and val: %s", get_type_name(s_type), s_val);
	ezc_fail(out);
	return EZC_NIL;
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
	} else if (type == TYPE_FLOAT) {
		return "float";
	} else if (type == TYPE_INT) {
		return "int";
	} else if (type == TYPE_STR) {
		return "str";
	} else if (type == TYPE_STK) {
		return "stack";
	} else if (type == TYPE_DICT) {
		return "dict";
	} else if (type == TYPE_MPZ) {
		return "mpz";
	} else if (type == TYPE_MPF) {
		return "mpf";
	} else if (type == TYPE_MPFR) {
		return "mpfr";	
	} else {
		ERR_STR(out);
		sprintf(out, "Unknown type (0x%x)", (unsigned int)type);
		ezc_fail(out);
		return "unknown";
	}
}

