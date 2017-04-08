
#include "ezc.h"

void stk_dump(EZC_STACK stk) {
	EZC_INT i = 0;
	while (i < (*stk).ptr) {
		printf("%lld, ", (long long)(*stk_get(stk, i++)).val);
	}
	printf("\n");
}

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
	EZC_OBJ res = stk_get(stk, (*stk).ptr);
	(*stk).ptr--;
	return res;
}

EZC_OBJ stk_get(EZC_STACK stk, EZC_INT pos) {
	return (*stk).vals[pos];
}


