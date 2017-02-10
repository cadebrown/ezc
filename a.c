

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>



int main(int argc, char *argv[]) {
	ezc_init(argc, argv);
	

	mpfr_t __tmp_uw; mpfr_init (__tmp_uw);
	 ezc_set();
	 ezc_var();
	mpfr_clear (__tmp_uw);
	return 0;
}
