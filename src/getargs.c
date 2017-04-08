
#include "ezc.h"

void init_args(EZC_DICT dict) {
	dict_init(dict, DEFAULT_ARRAY_LEN);
}

void help_message() {
	printf("%s (%s)\n", PACKAGE, VERSION);


	printf("");
}


// doesn't include end
void get_args(EZC_DICT dict, EZC_STR *args, EZC_IDX start, EZC_IDX end) {
	EZC_IDX i = start;
	while (i < end) {
		if (STR_STARTS(args[i], "-", 0)) {
			if (i == end - 1 || STR_STARTS(args[i+1], "-", 0)) {
				CREATE_OBJ(ret);
				(*ret).val = (void *)1;
				(*ret).type = TYPE_BOOL;
				dict_set(dict, args[i], ret);
			} else {
				CREATE_OBJ(ret);
				(*ret).val = (void *)args[i+1];
				(*ret).type = TYPE_STR;
				dict_set(dict, args[i], ret);
			}
		}
		i++;
	}
	
}



