
#include "ezc.h"


void exec(EZC_STR code, EZC_DICT dict, EZC_STACK stk) {
	EZC_INT i = 0, l;
	while (i < strlen(code)) {
		if (IS_DIGIT(code[i])) {
			char *tmp = (char *)malloc(1000);
			l=0;
			while (IS_DIGIT(code[i])) {
				printf("%s\n", tmp);
				tmp[l++]=code[i++];
			}
			tmp[l] = 0;
			EZC_OBJ llobj = (EZC_OBJ)malloc(sizeof(EZC_OBJ));
			(*llobj).val = (void *)strtoll(tmp, NULL, 10);
			stk_push(stk, llobj);
			i++;
		}
	}
}


int main(int argc, char *argv[]) {
    ezc_stk_t stk;
	ezc_dict_t dict;
	stk_init(&stk, DEFAULT_ARRAY_LEN);
	dict_init(&dict, DEFAULT_ARRAY_LEN);

	exec(argv[1], &dict, &stk);
	stk_dump(&stk);

}

