

#include "EZC.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>



int main(int argc, char *argv[]) {
	ezc_init(argc, argv);
	

	mpfr_t __tmp_z; mpfr_init (__tmp_z);
	ezc_set(__tmp_z,ezc_next_const("2"));
	ezc_var(__tmp_z);
	mpfr_clear (__tmp_z);
	return 0;
}
