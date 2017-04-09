
#include "ezc.h"


EZC_OBJ EZC_NIL;

EZC_OBJ codes;
EZC_OBJ poss;

EZC_OBJ stacks;
EZC_OBJ dicts;

EZC_OBJ args;

bool is_live = false;

char *EXEC_TITLE;


void ezc_end() {
	if ((*(EZC_DICT)((*dicts).val)).ptr > 0) {
		printf("\nFinal results:\n Dict:\n");
		obj_dump(dicts);
	}

	if ((*(EZC_STACK)((*stacks).val)).ptr == 1) {
		if (dict_contains_key((*args).val, "--raw")) {
			obj_dump_raw((*(EZC_STACK)((*stacks).val)).vals[0]);
		} else {
			printf("\nStacks:\n");
			obj_dump((*(EZC_STACK)((*stacks).val)).vals[0]);
		}
	} else {
		printf("\nStacks:\n");
		obj_dump(stacks);
	}

}

void init() {
	MALLOC_OBJ(EZC_NIL);
	SET_OBJ(EZC_NIL, TYPE_NIL, 0);


	MALLOC_OBJ(codes);
	SET_OBJ(codes, TYPE_STK, malloc(sizeof(EZC_STACK)));
	stk_init((*codes).val, 10);

	MALLOC_OBJ(poss);
	SET_OBJ(poss, TYPE_STK, malloc(sizeof(EZC_STACK)));
	stk_init((*poss).val, 10);

	MALLOC_OBJ(stacks);
	SET_OBJ(stacks, TYPE_STK, malloc(sizeof(EZC_STACK)));
	stk_init((*stacks).val, 10);

	CREATE_OBJ(this_obj);
	SET_OBJ(this_obj, TYPE_STK, malloc(sizeof(EZC_STACK)));
	stk_init((*this_obj).val, 10);
	stk_push((*stacks).val, this_obj);

	MALLOC_OBJ(dicts);
	SET_OBJ(dicts, TYPE_DICT, malloc(sizeof(EZC_STACK)));
	dict_init((*dicts).val, 10);
	
	MALLOC_OBJ(args);
	SET_OBJ(args, TYPE_DICT, malloc(sizeof(EZC_STACK)));
	dict_init((*args).val, 10);
}

void ezc_fail(EZC_STR reason) {
	printf("%sError: %s%s\n", KRED, KMAG, reason);
	printf("%sWhile executing:%s\n", KBLU, KNRM);

	long long i = 0;
	while (i < (*(EZC_STACK)((*codes).val)).ptr) {
		obj_dump_raw(stk_get((EZC_STACK)(*codes).val, i));
		printf("\n");
		_SPACE((long long)(*stk_get((EZC_STACK)(*poss).val, i)).val);
		printf("^\n");
		i++;
	}

	if (is_live) {
		return;
	}

	printf("Dictionary: \n");
	obj_dump(dicts);

	printf("Stacks: \n");
	obj_dump(stacks);

	printf("\n\nTHE PROGRAM FAILED\n\n");
	exit (3);
}


void exec(EZC_STR code) {
	EZC_INT i = 0;


	CREATE_OBJ(this_pos);
	SET_OBJ(this_pos, TYPE_INT, malloc(sizeof(EZC_INT)));


	stk_push((*poss).val, this_pos);
	stk_push((*codes).val, str_literal(code));

	while (i < strlen(code)) {
		SKIP_WHITESPACE(code, i);
		(*this_pos).val = (void *)(i);

		if (IS_DIGIT(code[i]) || (code[i] == '-' && IS_DIGIT(code[i+1]))) {
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


			stk_push(LAST_STACK, ret);

			free(res);
		} else if (IS_OP(code, i)) {
			char *res = (char *)malloc(MAX_OP_LENGTH);
			ret_operator(res, code, &i);
			eval_op(res);
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
			stk_push(LAST_STACK, obj_from_str(res));
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
			eval_func(res);
			free(res);
		} else if (i < strlen(code)) {
			ezc_fail("Unexpected character");
			stk_pop((*poss).val);
			return;
		}
	}
	stk_pop((*poss).val);
}

void interperet() {
	char line[MAX_INTERPERET_LEN];
	printf(" > ");
	while (fgets(line, sizeof(line), stdin)) {
		if (line[strlen(line)-1] == '\n') {
			line[strlen(line)-1] = 0;
		}
		exec(line);
		printf(" > ");
	}
}


int main(int argc, char *argv[]) {
	EXEC_TITLE = argv[0];
	
	init();

	get_args((*args).val, argv, 1, argc);

	if (CONT_ALIAS((*args).val, "-h", "--help")) {
		help_message();
		return 0;
	}

	if (CONT_ALIAS((*args).val, "-e", "--expr")) {
		char * torun;
		if (dict_contains_key((*args).val, "-e")) {
			torun = str_from_obj(dict_get((*args).val, "-e"));
		} else {
			torun = str_from_obj(dict_get((*args).val, "--expr"));
		}

		exec(torun);

		ezc_end();

	} else {
		is_live = true;
		interperet();

		ezc_end();
	}



}

