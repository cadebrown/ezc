/* getargs.c -- handles argument parsing for command line, and handles usage
             -- messages

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

void help_message() {

	printf("Usage: %s [- | -e [EXPR]] [OPTIONS ...]\n\n", EXEC_TITLE);

	CREATE_OBJ(usage);
	SET_OBJ(usage, TYPE_DICT, malloc(sizeof(ezc_dict_t)));

	dict_init((*usage).val, 10);

	dict_set((*usage).val, "-", obj_from_str("\"start an interactive shell\""));
	dict_set((*usage).val, "-h, --help", obj_from_str("\"shows help message and exits\""));
	dict_set((*usage).val, "-e, --expr '[CMD]'", obj_from_str("\"executes [CMD]\""));


	obj_dump_fmt(usage, ARGS_PRINT_OFFSET+25, 0, PRINT_OFFSET, false, false);


	printf("\nFor general discussion, please post and subscribe on the mailing list:\n");
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



