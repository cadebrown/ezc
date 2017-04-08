
#include "ezc.h"


bool obj_eq(EZC_OBJ a, EZC_OBJ b) {
	return obj_cmp(a, b) == 0;
}

EZC_INT obj_cmp(EZC_OBJ a, EZC_OBJ b) {
	if ((*a).type != (*b).type) {
		return false;
	}
	EZC_TYPE type = (*a).type;
	if (type == TYPE_INT) {
		return ((EZC_INT)((*a).val) < (EZC_INT)((*b).val)) ? -1 : (EZC_INT)((*a).val) > (EZC_INT)((*b).val);
	} else if (type == TYPE_BOOL) {
		return ((bool)((*a).val) < (bool)((*b).val)) ? -1 : (bool)((*a).val) > (bool)((*b).val);
	} else if (type == TYPE_STR) {
		return strcmp((EZC_STR)((*a).val), (EZC_STR)((*b).val));
	}
}

void obj_dump(EZC_OBJ obj) {
	if ((*obj).type == TYPE_INT) {
		printf("%lld", (long long)(*obj).val);
	} else if ((*obj).type == TYPE_BOOL) {
		if ((long long)(*obj).val == 0) {
			printf("false");
		} else {
			printf("true");
		}
	} else if ((*obj).type == TYPE_STR) {
		printf("'%s'", (char *)(*obj).val);
	}
}
