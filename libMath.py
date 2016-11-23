# -*- coding: utf-8 -*-
from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
/*

	libMath

*/
//finds the maximum of a and b
void frand_1(mpfr_t r) { 
	mpfr_urandomb(r, __rand_state); 
}
//random number between [0, a)
void frand(mpfr_t r, mpfr_t a) { 
	mpfr_urandomb(r, __rand_state);
	mpfr_mul(r, r, a, MPFR_RNDD);
}
//finds the maximum of a and b
void frandgauss(mpfr_t r) { 
	mpfr_grandom(r, NULL, __rand_state, MPFR_RNDD); 
}

// square root, i.e. r * r ~ a
void fsqrt(mpfr_t r, mpfr_t a) { 
	mpfr_sqrt(r, a, GMP_RNDN); 
}
// cube root, i.e. r * r * r ~ a
void fcbrt(mpfr_t r, mpfr_t a) { 
	mpfr_cbrt(r, a, GMP_RNDN); 
}

// e ^ x, where e is euler's number (roughly 2.718281828)
void fexp(mpfr_t r, mpfr_t a) { 
	mpfr_exp(r, a, GMP_RNDN); 
}

// ln(a), log base eulers number. where e ^ (ln(a)) ~ a
void flog(mpfr_t r, mpfr_t a) { 
	mpfr_log(r, a, GMP_RNDN); 
}
// log_(b)(a), log base b of a. where b ^ (log_(b)(a)) ~ a
// identity: log_(b)(a) = ln(a)/ln(b)
void flogb(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __flogb_tmp; mpfr_init(__flogb_tmp);
	flog(__flogb_tmp, b);
	flog(r, a);
	fdiv(r, r, __flogb_tmp);
	mpfr_clear(__flogb_tmp);
}

// calculates the gamma function of a, gamma(a)=factorial(a-1)
void fgamma(mpfr_t r, mpfr_t a) { 
	mpfr_gamma(r, a, GMP_RNDN); 
}
// calculates the factorial of a, or gamma(a+1)
void ffactorial(mpfr_t r, mpfr_t a) { 
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_add(__tmp, a, _next_const(\"1.0\"), GMP_RNDN);
	fgamma(r, __tmp);
	mpfr_clear(__tmp);
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

class Exp(LibraryFunction):
	def __str__(self):
		return "fexp(%s);" % (", ".join(map(str, self.args)))
class Log(LibraryFunction):
	def __str__(self):
		return "flog(%s);" % (", ".join(map(str, self.args)))
class Logb(LibraryFunction):
	def __str__(self):
		return "flogb(%s);" % (", ".join(map(str, self.args)))

class Gamma(LibraryFunction):
	def __str__(self):
		return "fgamma(%s);" % (", ".join(map(str, self.args)))
class Factorial(LibraryFunction):
	def __str__(self):
		return "ffactorial(%s);" % (", ".join(map(str, self.args)))

lib = Library(this_lib, "0.0.2", {
	"sqrt": Sqrt, 
	"cbrt": Cbrt, 

	"exp": Exp, 

	"log": Log, 
	"logb": Logb, 
	
	"gamma": Gamma, 
	"factorial": Factorial, 
}, {
	"?": Rand,
	"??": RandGuass,

	"√": Sqrt,
	"√√": Cbrt,

	"!":Factorial,
})
