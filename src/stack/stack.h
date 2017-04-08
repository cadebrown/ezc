#ifndef __EZC_STACK_H__
#define __EZC_STACK_H__

#define EZC_STACK ezc_stk_t *

typedef struct ezc_stk_t {
	EZC_IDX ptr;
	EZC_IDX size;

	EZC_OBJ *vals;
} ezc_stk_t;


void stk_init(EZC_STACK stk, EZC_INT len);
void stk_resize(EZC_STACK stk, EZC_INT len);

EZC_OBJ stk_pop(EZC_STACK stk);
EZC_OBJ stk_get(EZC_STACK stk, EZC_INT pos);

void stk_push(EZC_STACK stk, EZC_OBJ val);
void stk_set(EZC_STACK stk, EZC_INT pos, EZC_OBJ val);

#endif
