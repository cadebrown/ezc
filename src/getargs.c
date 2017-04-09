
#include "ezc.h"

void init_args(EZC_DICT dict) {
	dict_init(dict, DEFAULT_ARRAY_LEN);
}

void help_message() {

	printf("Usage: %s [- | -e [EXPR]] [OPTIONS ...]\n\n", EXEC_TITLE);

	ezc_dict_t usage;
	dict_init(&usage, 10);

	dict_set(&usage, "-", obj_from_str("\"start an interactive shell\""));
	dict_set(&usage, "-h, --help", obj_from_str("\"shows help message and exits\""));
	dict_set(&usage, "-e, --expr '[CMD]'", obj_from_str("\"executes [CMD]\""));

	dict_dump_fmt(&usage, true, ARG_PRINT_OFF);


	printf("For general discussion, please post and subscribe on the mailing list:\n");
	printf("<%s>\n", PACKAGE_BUGREPORT);
//	SET_OBJ();
	printf("\n");
	
	#ifdef PACKAGE_VERSION
		printf("v%s", PACKAGE_VERSION);
		#ifdef __DATE__
			printf(" compiled on %s", __DATE__);
			#ifdef __TIME__
				printf(" %s", __TIME__);
			#endif	
		#endif
	#endif
	

	printf("\n");
}


// doesn't include end
void get_args(EZC_DICT dict, EZC_STR *args, EZC_IDX start, EZC_IDX end) {
	EZC_IDX i = start;
	while (i < end) {
		if (STR_STARTS(args[i], "-", 0)) {
			if (i == end - 1 || STR_STARTS(args[i+1], "-", 0)) {
				CREATE_OBJ(ret);
				SET_OBJ(ret, TYPE_BOOL, 1);
				dict_set(dict, args[i], ret);
			} else {
				CREATE_OBJ(ret);
				SET_OBJ(ret, TYPE_STR, args[i+1]);
				dict_set(dict, args[i], ret);
			}
		}
		i++;
	}
	
}



