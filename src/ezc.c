
#include "ezc.h"


EZC_OBJ EZC_NIL;
char *EXEC_TITLE;

void init() {
	MALLOC_OBJ(EZC_NIL);
	SET_OBJ(EZC_NIL, TYPE_NIL, 0);
}

void fail(EZC_STR reason, EZC_STR code, EZC_DICT dict, EZC_STACK stk, long long pos) {
	printf("Error: %s\n", reason);
	printf("While executing:\n %s", code);
	if (code[strlen(code)-1] != '\n') {
		printf("\n");
	}

	long long i = 0;
	while (i <= pos) {
		printf(" ");
		i++;
	}
	printf("^\n");

	printf("Final dict:\n");
	dict_dump(dict);

	printf("\nFinal stack:\n");
	stk_dump(stk);

	exit (3);
}


void exec(EZC_STR code, EZC_DICT dict, EZC_STACK stk) {
	EZC_INT i = 0;
	while (i < strlen(code)) {
		SKIP_WHITESPACE(code, i);
		if (IS_DIGIT(code[i]) || (code[i] == '-' && IS_DIGIT(code[i++]))) {
			EZC_INT sind = i, eind = i;
			if (code[eind] == '-') {
				eind++;
			}

			while (IS_DIGIT(code[eind])) {
				eind++;
			}
			if (code[eind] == 'Z') {
				eind++;
			}
			char *res = (char *)malloc(eind-i+1);
			while (i < eind) {
				res[i - sind] = code[i];
				i++;
			}
			res[i] = 0;
			EZC_OBJ ret = obj_from_str(res);

			stk_push(stk, ret);

			free(res);
		} else if (IS_OP(code, i)) {
			char *res = (char *)malloc(MAX_OP_LENGTH);
			ret_operator(res, code, &i);
			eval_op(dict, stk, res);
			free(res);
		} else if (code[i] == '"') {
			EZC_INT sind = i, eind = i;
			eind++;
			while (code[eind] != '"' && code[eind-1] != '\\') {
				eind++;
			}
			eind++;
			char *res = (char *)malloc(eind-i+1);
			while (i < eind) {
				res[i - sind] = code[i];
				i++;
			}
			res[i-sind] = 0;
			stk_push(stk, obj_from_str(res));
			free(res);
		} else if (IS_ALPHA(code[i])) {
			EZC_INT sind = i, eind = i;
			while (IS_ALPHA(code[eind]) && eind < strlen(code)) {
				eind++;
			}
			char *res = (char *)malloc(eind-i+1);
			while (i < eind &&  i < strlen(code)) {
				res[i - sind] = code[i];
				i++;
			}
			res[i-sind] = 0;
			eval_func(dict, stk, res);
			free(res);
		} else if (i < strlen(code)) {
			fail("Unexpected character", code, dict, stk, i);
		}
	}
}

void interperet(EZC_DICT dict, EZC_STACK stk) {
	char line[MAX_INTERPERET_LEN];
	printf(" > ");
	while (fgets(line, sizeof(line), stdin)) {
		if (strlen(line) == 2 && line[0] == 'q') {
			return;
		}
		exec(line, dict, stk);
		printf(" > ");
	}
}


int main(int argc, char *argv[]) {
	EXEC_TITLE = argv[0];
	init();

	ezc_dict_t args;
	init_args(&args);

	get_args(&args, argv, 1, argc);

	if (CONT_ALIAS(&args, "-h", "--help")) {
		help_message();
		return 0;
	}

	if (CONT_ALIAS(&args, "-e", "--expr")) {
		ezc_stk_t stk;
		ezc_dict_t dict;
		stk_init(&stk, DEFAULT_STACK_LEN);
		dict_init(&dict, DEFAULT_DICT_LEN);
		char * torun;
		if (dict_contains_key(&args, "-e")) {
			torun = str_from_obj(dict_get(&args, "-e"));
		} else {
			torun = str_from_obj(dict_get(&args, "--expr"));
		}

		exec(torun, &dict, &stk);

		//printf("\nFinal results:\n");

		//dict_dump(&dict);
		//stk_dump(&stk);
	} else {
		ezc_stk_t stk;
		ezc_dict_t dict;
		stk_init(&stk, DEFAULT_STACK_LEN);
		dict_init(&dict, DEFAULT_DICT_LEN);

		interperet(&dict, &stk);

		printf("Final results:\n");

		dict_dump(&dict);
		stk_dump(&stk);
	}



}

