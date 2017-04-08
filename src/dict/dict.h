#ifndef __EZC_DICT_H__
#define __EZC_DICT_H__

#define EZC_DICT ezc_dict_t *


typedef struct ezc_dict_t {
	EZC_IDX ptr;
	EZC_IDX size;

	char **keys;
	EZC_OBJ *vals;
} ezc_dict_t;


void dict_init(EZC_DICT stk, EZC_INT len);
void dict_resize(EZC_DICT stk, EZC_INT len);

void dict_set(EZC_DICT dict, EZC_STR key, EZC_OBJ val);

EZC_OBJ dict_get(EZC_DICT dict, EZC_STR key);

#endif
