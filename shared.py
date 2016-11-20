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
#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>


int _size_consts = 5, _consts_id = 0, _consts_ov = 0;
mpfr_t *_consts;

mpfr_ptr _next_const(char _set_val[]) {
	_consts_id = _consts_id + 1;
	if (_consts_id >= _size_consts) {
		_consts_ov = 1;
		_consts_id %= _size_consts;
	}
	if (_consts_ov > 0) {
		//mpfr_clear(_consts[_consts_id]);
		mpfr_init(_consts[_consts_id]);
	}
	mpfr_set_str(_consts[_consts_id], _set_val, 10, GMP_RNDN);
	//_consts_id++;
	return (mpfr_ptr) _consts[_consts_id];
}


// default precision (and minimum)
int EZC_PREC = 128;
// macro to get bits instead of digits
#define _bprec (int)(_prec * .301029)

// variables to use in our program that are hidden to others
int _prec, _loop, _argc;
// a pointer so we can read from all methods
char **_argv;
// a lits of commandline arguments that are mpfr s
mpfr_t *args;

// constants (look for initializations in lib_linker)
mpfr_t __start, __stop, __step;

"""


main="""
// main method
int main(int argc, char *argv[]) {
	// set global vars so other functions can use them
	_argc = argc; _argv = argv;
	// set the default rounding mode
	mpfr_set_default_rounding_mode(GMP_RNDN);

	// start the commandline args list
	args = (mpfr_t *)malloc(sizeof(mpfr_t) * sizeof(argc - 1));
	_consts = (mpfr_t *)malloc(sizeof(mpfr_t) * sizeof(_size_consts));
	prec_literal(EZC_PREC);
"""

var_inits="""
	// loop through, and parse each one for a mpfr
	for (_loop = 1; _loop < argc; ++_loop) {
		mpfr_init(args[_loop]);
		mpfr_set_str(args[_loop], argv[_loop], 10, GMP_RNDN);
	}
	for (_loop = 0; _loop < _size_consts; ++_loop) {
		mpfr_init(_consts[_loop]);
	}
	mpfr_init(__start);
	mpfr_init(__start);
	mpfr_init(__stop);

"""

end="""
}
"""
