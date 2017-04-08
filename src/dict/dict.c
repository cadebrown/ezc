
#include "ezc.h"

void dict_init(EZC_DICT dict, EZC_INT len) {
	(*dict).ptr = 0;
	(*dict).size = len;
	(*dict).keys = (EZC_STR*)malloc(sizeof(EZC_STR) * len);
	(*dict).vals = (EZC_OBJ *)malloc(sizeof(EZC_STR) * len);
}

void dict_resize(EZC_DICT dict, EZC_INT len) {
	(*dict).size = len;
	(*dict).keys = (EZC_STR *)realloc((*dict).keys, sizeof(EZC_STR) * len);
	(*dict).vals = (EZC_OBJ *)realloc((*dict).vals, sizeof(EZC_OBJ *) * len);
}


void dict_set(EZC_DICT dict, EZC_STR key, EZC_OBJ val) {
	(*dict).keys[(*dict).ptr] = key;
	(*dict).vals[(*dict).ptr] = val;
	(*dict).ptr++;
}

EZC_OBJ dict_get(EZC_DICT dict, EZC_STR key) {
	EZC_INT i = 0;
	while (i <= (*dict).ptr) {
		if (strcmp((*dict).keys[i], key) == 0) {
			return (*dict).vals[i];
		}
		i++;
	}
	return NULL;
}

