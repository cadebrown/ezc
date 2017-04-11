/* ezc.c -- main file for EZC

  Copyright 2016-2017 ChemicalDevelopment

  This file is part of the EZC project.

  EZC, EC, EZC Documentation, and any other resources in this project are 
free software; you are free to redistribute it and/or modify them under 
the terms of the GNU General Public License; either version 3 of the 
license, or any later version.

  These programs are hopefully useful and reliable, but it is understood 
that these are provided WITHOUT ANY WARRANTY, or MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GPLv3 or email at 
<info@chemicaldevelopment.us> for more info on this.

  Here is a copy of the GPL v3, which this software is licensed under. You 
can also find a copy at http://www.gnu.org/licenses/.

*/

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
	SET_OBJ(codes, TYPE_STK, malloc(sizeof(ezc_stk_t)));
	stk_init((*codes).val, 10);

	MALLOC_OBJ(poss);
	SET_OBJ(poss, TYPE_STK, malloc(sizeof(ezc_stk_t)));
	stk_init((*poss).val, 10);

	MALLOC_OBJ(stacks);
	SET_OBJ(stacks, TYPE_STK, malloc(sizeof(ezc_stk_t)));
	stk_init((*stacks).val, 10);

	CREATE_OBJ(this_obj);
	SET_OBJ(this_obj, TYPE_STK, malloc(sizeof(ezc_stk_t)));
	stk_init((*this_obj).val, 10);
	stk_push((*stacks).val, this_obj);

	MALLOC_OBJ(dicts);
	SET_OBJ(dicts, TYPE_DICT, malloc(sizeof(ezc_dict_t)));
	dict_init((*dicts).val, 10);
	
	MALLOC_OBJ(args);
	SET_OBJ(args, TYPE_DICT, malloc(sizeof(ezc_dict_t)));
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
	EZC_INT si = 0, i = 0;


	CREATE_OBJ(this_pos);
	SET_OBJ(this_pos, TYPE_INT, malloc(sizeof(EZC_INT)));


	stk_push((*poss).val, this_pos);
	stk_push((*codes).val, str_literal(code));

	while (i < strlen(code)) {
		SKIP_WHITESPACE(code, i);
		(*this_pos).val = (void *)(i);
		if (i >= strlen(code)) {
			stk_pop((*codes).val);
			stk_pop((*poss).val);
			return;
		}
		if ((!IS_ALPHA(code[i]) && !IS_OP(code, i)) || str_has_char_before_space(code, i, ':')) {
			si = i;
			while (code[i] != ' ' && i < strlen(code) && !IS_OP(code, i)) {
				i++;
			}
			char *cur_obj = (char *)malloc(i-si+3);
			long long j = si;
			while (j < i) {
				cur_obj[j - si] = code[j];
				j++;
			}
			cur_obj[j - si] = 0;
			stk_push(LAST_STACK, obj_from_str(cur_obj));

			free(cur_obj);
		} else {
			si = i;
			while (code[i] != ' ' && i < strlen(code)) {
				i++;
			}
			char *func = (char *)malloc(i-si+1);
			long long j = si;
			while (j < i) {
				func[j - si] = code[j];
				j++;
			}
			func[j - si] = 0;

			eval_func(func);

			free(func);
		}

	}
	stk_pop((*codes).val);
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
	dict_init((*args).val, 10);

	get_args((*args).val, argv, 1, argc);

//	char *trun = str_from_obj(dict_get((*args).val, "-t"));

	//printf("%s\n", trun);

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

