
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

