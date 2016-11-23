"""

Shared functions, and data for lib_linker and EZcompiler

"""

class Library():
	def __init__(self, text, version, funcs, opers):
		self.ver = version
		self.text = text
		self.funcs = funcs
		self.opers = opers

	def __str__(self):
		return self.text

class LibraryFunction():
	def __init__(self, *args):
		self.args = args

	def __str__(self):
		return self.__class__.__name__ + "(%s);" % (", ".join(self.args))


class Base(LibraryFunction):
	def __str__(self):
		return "%s(%s);" % (self.args[0], ", ".join(self.args[1:]))

functions = {
	"__base": Base
}

operators = {

}

var_set = set()

prec = "prec_literal(EZC_PREC);"

start = """
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>


gmp_randstate_t __rand_state;
int _size_consts = 5, _consts_id = 0, _consts_ov = 0;
mpfr_t *_consts;

mpfr_ptr _next_const(char _set_val[]) {
	if (_consts_id >= _size_consts) {
		_consts_ov = 1;
		_consts_id %= _size_consts;
	}
	if (_consts_ov == 0) {
		mpfr_init(_consts[_consts_id]);
	}
	mpfr_set_str(_consts[_consts_id], _set_val, 10, MPFR_RNDD);
	return (mpfr_ptr) _consts[_consts_id++];
}


int EZC_PREC = 60;
#define _bprec (_prec * 3.32192809)

int _prec, _loop, _argc;
char **_argv;
//mpfr_t *args;

mpfr_t __start, __stop, __step;

mpfr_ptr _get_arg(int val) {
	if (val >= _argc) {
		return _next_const("0.0");
	} else {
		return _next_const(_argv[val]);
	}
}

"""


main="""
int main(int argc, char *argv[]) {
	_argc = argc; _argv = argv;
	mpfr_set_default_rounding_mode(MPFR_RNDD);

	srand(time(NULL));

	gmp_randinit_default(__rand_state);
	gmp_randseed_ui(__rand_state, rand());

	//args = (mpfr_t *)malloc(sizeof(mpfr_t) * (argc - 1));
	_consts = (mpfr_t *)malloc(sizeof(mpfr_t) * _size_consts);
"""

var_inits="""

"""

end="""
	return 0;
}
"""
