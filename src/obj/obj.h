#ifndef __EZC_OBJ_H__
#define __EZC_OBJ_H__

#define DEFAULT_ARRAY_LEN 20

#define EZC_OBJ ezc_obj_t *

typedef struct ezc_obj_t {
	EZC_TYPE type;

	void *val;
} ezc_obj_t;



#endif
