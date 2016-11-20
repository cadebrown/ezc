# -*- coding: utf-8 -*-
from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
// square root, i.e. r * r ~ a
void fsqrt(mpfr_t r, mpfr_t a) { mpfr_sqrt(r, a, GMP_RNDN); }
// cube root, i.e. r * r * r ~ a
void fcbrt(mpfr_t r, mpfr_t a) { mpfr_cbrt(r, a, GMP_RNDN); }

// sin of a in radians
void fsin(mpfr_t r, mpfr_t a) { mpfr_sin(r, a, GMP_RNDN); }
// cos of a in radians
void fcos(mpfr_t r, mpfr_t a) { mpfr_cos(r, a, GMP_RNDN); }
// tan of a in radians (sin(a)/cos(a))
void ftan(mpfr_t r, mpfr_t a) { mpfr_tan(r, a, GMP_RNDN); }

// asin of a, returned in radians. asin(sin(a)) ~ a
void fasin(mpfr_t r, mpfr_t a) { mpfr_asin(r, a, GMP_RNDN); }
// acos of a, returned in radians. acos(cos(a)) ~ a
void facos(mpfr_t r, mpfr_t a) { mpfr_acos(r, a, GMP_RNDN); }
// atan of a, returned in radians. atan(tan(a)) ~ a
// note that this can return odd values of poles in tan (i.e. pi/2+pi*n)
void fatan(mpfr_t r, mpfr_t a) { mpfr_atan(r, a, GMP_RNDN); }

// e ^ x, where e is euler's number (roughly 2.718281828)
void fexp(mpfr_t r, mpfr_t a) { mpfr_exp(r, a, GMP_RNDN); }
// ln(a), log base eulers number. where e ^ (ln(a)) ~ a
void flog(mpfr_t r, mpfr_t a) { mpfr_log(r, a, GMP_RNDN); }
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
void fgamma(mpfr_t r, mpfr_t a) { mpfr_gamma(r, a, GMP_RNDN); }
// calculates the factorial of a, or gamma(a+1)
void ffactorial(mpfr_t r, mpfr_t a) { 
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_add(__tmp, a, _next_const(\"1.0\"), GMP_RNDN);
	fgamma(r, __tmp);
	mpfr_clear(__tmp);
}
"""

class Sqrt(LibraryFunction):
	def __str__(self):
		return "fsqrt(%s);" % (", ".join(map(str, self.args)))
class Cbrt(LibraryFunction):
	def __str__(self):
		return "fcbrt(%s);" % (", ".join(map(str, self.args)))

class Sin(LibraryFunction):
	def __str__(self):
		return "fsin(%s);" % (", ".join(map(str, self.args)))
class Cos(LibraryFunction):
	def __str__(self):
		return "fcos(%s);" % (", ".join(map(str, self.args)))
class Tan(LibraryFunction):
	def __str__(self):
		return "ftan(%s);" % (", ".join(map(str, self.args)))

class Asin(LibraryFunction):
	def __str__(self):
		return "fasin(%s);" % (", ".join(map(str, self.args)))
class Acos(LibraryFunction):
	def __str__(self):
		return "facos(%s);" % (", ".join(map(str, self.args)))
class Atan(LibraryFunction):
	def __str__(self):
		return "fatan(%s);" % (", ".join(map(str, self.args)))

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

libMath = Library(this_lib, "0.0.2", {
	"sqrt": Sqrt, 
	"cbrt": Cbrt, 
	"sin": Sin, 
	"asin": Asin, 
	"cos": Cos, 
	"acos": Acos, 
	"tan": Tan, 
	"atan": Atan, 
	"exp": Exp, 
	"log": Log, 
	"logb": Logb, 
	"gamma": Gamma, 
	"factorial": Factorial, 
}, {

	"√": Sqrt,
	"!":Factorial
	
})
