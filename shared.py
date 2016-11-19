functions = {

}

operators = {

}

var_set = set()

prec = "prec_literal(EZC_PREC);"

start = """
#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>

int EZC_PREC = 128;
#define _bprec (int)(_prec * .301029)

int _prec, _loop, _argc;
char **_argv;
mpfr_t *args;

"""


main="""

int main(int argc, char *argv[]) {
	_argc = argc; _argv = argv;
	mpfr_set_default_rounding_mode(GMP_RNDN);

	args = (mpfr_t *)malloc(sizeof(mpfr_t) * sizeof(argc - 1));
	for (_loop = 1; _loop < argc; ++_loop) {
		mpfr_init(args[_loop]);
		mpfr_set_str(args[_loop], argv[_loop], 10, GMP_RNDN);
	}
"""

end="""
}
"""
