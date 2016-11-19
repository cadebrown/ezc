from EZC_types import LibraryFunction, Library

import re, shared, parser
import lib_linker

this_lib = """

void fsqrt(mpfr_t r, mpfr_t a) { mpfr_sqrt(r, a, GMP_RNDN); }

void fsin(mpfr_t r, mpfr_t a) { mpfr_sin(r, a, GMP_RNDN); }
void fcos(mpfr_t r, mpfr_t a) { mpfr_cos(r, a, GMP_RNDN); }
void ftan(mpfr_t r, mpfr_t a) { mpfr_tan(r, a, GMP_RNDN); }

void fasin(mpfr_t r, mpfr_t a) { mpfr_asin(r, a, GMP_RNDN); }
void facos(mpfr_t r, mpfr_t a) { mpfr_acos(r, a, GMP_RNDN); }
void fatan(mpfr_t r, mpfr_t a) { mpfr_atan(r, a, GMP_RNDN); }

void fexp(mpfr_t r, mpfr_t a) { mpfr_exp(r, a, GMP_RNDN); }
void flog(mpfr_t r, mpfr_t a) { mpfr_log(r, a, GMP_RNDN); }
void flogb(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __flogb_tmp; mpfr_init(__flogb_tmp);
	flog(__flogb_tmp, b);
	flog(r, a);
	fdiv(r, r, __flogb_tmp);
	mpfr_clear(__flogb_tmp);
}

"""

class Sqrt(LibraryFunction):
	def __str__(self):
		return "fsqrt(%s);" % (", ".join(map(str, self.args)))

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


libMath = Library(this_lib, "0.0.2", {
	"sqrt": Sqrt, 
	"sin": Sin, 
	"asin": Asin, 
	"cos": Cos, 
	"acos": Acos, 
	"tan": Tan, 
	"atan": Atan, 
	"exp": Exp, 
	"log": Log, 
	"logb": Logb, 
}, {
	
})
