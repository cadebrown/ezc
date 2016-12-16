# -*- coding: utf-8 -*-
from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
/*

	libMath

*/
void frand_1(mpfr_t r) { 
	mpfr_urandomb(r, __rand_state); 
}
void frand(mpfr_t r, mpfr_t a) { 
	mpfr_urandomb(r, __rand_state);
	mpfr_mul(r, r, a, EZC_RND);
}
void frandgauss(mpfr_t r) { 
	mpfr_grandom(r, NULL, __rand_state, EZC_RND); 
}

void fsqrt(mpfr_t r, mpfr_t a) { 
	mpfr_sqrt(r, a, EZC_RND); 
}
void fcbrt(mpfr_t r, mpfr_t a) { 
	mpfr_cbrt(r, a, EZC_RND); 
}
void fhypot(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_hypot(r, a, b, EZC_RND); 
}

void fexp(mpfr_t r, mpfr_t a) { 
	mpfr_exp(r, a, EZC_RND); 
}

void flog(mpfr_t r, mpfr_t a) { 
	mpfr_log(r, a, EZC_RND); 
}
void flogb(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __flogb_tmp; mpfr_init(__flogb_tmp);
	flog(__flogb_tmp, b);
	flog(r, a);
	fdiv(r, r, __flogb_tmp);
	mpfr_clear(__flogb_tmp);
}

void fagm(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_agm(r, a, b, EZC_RND);
}


void fgamma(mpfr_t r, mpfr_t a) { 
	mpfr_gamma(r, a, EZC_RND); 
}
void ffactorial(mpfr_t r, mpfr_t a) { 
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_add(__tmp, a, _next_const(\"1.0\"), EZC_RND);
	fgamma(r, __tmp);
	mpfr_clear(__tmp);
}

void fzeta(mpfr_t r, mpfr_t a) {
	mpfr_zeta(r, a, EZC_RND);
}
"""

class Rand(LibraryFunction):
	def __str__(self):
		if len(self.args) == 1:
			return "frand_1(%s);" % (self.args[0])
		else:
			return "frand(%s, %s);" % (self.args[0], self.args[1])
class RandGuass(LibraryFunction):
	def __str__(self):
		return "frandgauss(%s);" % (self.args[0])

class Sqrt(LibraryFunction):
	def __str__(self):
		return "fsqrt(%s);" % (", ".join(map(str, self.args)))
class Cbrt(LibraryFunction):
	def __str__(self):
		return "fcbrt(%s);" % (", ".join(map(str, self.args)))
class Hypot(LibraryFunction):
	def __str__(self):
		return "fhypot(%s);" % (", ".join(map(str, self.args)))

class Exp(LibraryFunction):
	def __str__(self):
		return "fexp(%s);" % (", ".join(map(str, self.args)))
class Log(LibraryFunction):
	def __str__(self):
		return "flog(%s);" % (", ".join(map(str, self.args)))
class Logb(LibraryFunction):
	def __str__(self):
		return "flogb(%s);" % (", ".join(map(str, self.args)))

class Agm(LibraryFunction):
	def __str__(self):
		return "fagm(%s);" % (", ".join(map(str, self.args)))


class Gamma(LibraryFunction):
	def __str__(self):
		return "fgamma(%s);" % (", ".join(map(str, self.args)))
class Factorial(LibraryFunction):
	def __str__(self):
		return "ffactorial(%s);" % (", ".join(map(str, self.args)))

class Zeta(LibraryFunction):
	def __str__(self):
		return "fzeta(%s);" % (", ".join(map(str, self.args)))

lib = Library(this_lib, "0.0.2", {
	"rand": Rand,

	"sqrt": Sqrt, 
	"cbrt": Cbrt, 

	"hypot": Hypot, 

	"exp": Exp, 

	"log": Log, 
	"logb": Logb, 

	"agm": Agm,
	
	"gamma": Gamma, 
	"factorial": Factorial, 

	"zeta": Zeta, 
	
}, {
	"?": Rand,
	"??": RandGuass,

	"√": Sqrt,
	"√√": Cbrt,

	"!": Factorial,

	"ζ": Zeta,
	
})
