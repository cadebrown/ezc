
#include "ezc.h"



void obj_dump(EZC_OBJ obj) {
	obj_dump_fmt(obj, false);
}


void obj_dump_fmt(EZC_OBJ obj, bool raw) {
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
		if (raw) {
			printf("%s", (char *)(*obj).val);
		} else {
			printf("'%s'", (char *)(*obj).val);
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
				printf("ERROR: Specified using `Z` but GMP support is not enabled")
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

