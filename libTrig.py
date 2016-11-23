# -*- coding: utf-8 -*-
from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
/*

	libTrig

*/

// sin of a in radians
void fsin(mpfr_t r, mpfr_t a) { 
	mpfr_sin(r, a, MPFR_RNDD); 
}
// cosecant of a in radians (1/sin(a))
void fcsc(mpfr_t r, mpfr_t a) { 
	mpfr_csc(r, a, MPFR_RNDD); 
}
// cos of a in radians
void fcos(mpfr_t r, mpfr_t a) { 
	mpfr_cos(r, a, MPFR_RNDD); 
}
// secant of a in radians (1/cos(a))
void fsec(mpfr_t r, mpfr_t a) { 
	mpfr_sec(r, a, MPFR_RNDD); 
}
// tan of a in radians (sin(a)/cos(a))
void ftan(mpfr_t r, mpfr_t a) { 
	mpfr_tan(r, a, MPFR_RNDD); 
}
// cotangent of a, or 1/(tan(a))
void fcot(mpfr_t r, mpfr_t a) { 
	mpfr_tan(r, a, MPFR_RNDD); 
}

// asin of a, returned in radians. asin(sin(a)) ~ a
void fasin(mpfr_t r, mpfr_t a) { 
	mpfr_asin(r, a, MPFR_RNDD); 
}
// acsc of a, returned in radians. acsc(csc(a)) ~ a
void facsc(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, MPFR_RNDD);
	mpfr_asin(r, r, MPFR_RNDD); 
}
// acos of a, returned in radians. acos(cos(a)) ~ a
void facos(mpfr_t r, mpfr_t a) { 
	mpfr_acos(r, a, MPFR_RNDD); 
}
// asec  of a, returned in radians. asec(sec(a)) ~ a
void fasec(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, MPFR_RNDD);
	mpfr_acos(r, r, MPFR_RNDD); 
}
// atan of a, returned in radians. atan(tan(a)) ~ a
// note that this can return odd values (such as +Infinity or NaN) at poles (i.e. pi/2+pi*n)
void fatan(mpfr_t r, mpfr_t a) { 
	mpfr_atan(r, a, MPFR_RNDD); 
}
// acot of a, returned in radians. acot(cot(a)) ~ a
void facot(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, MPFR_RNDD);
	mpfr_atan(r, r, MPFR_RNDD); 
}


"""

class Sin(LibraryFunction):
	def __str__(self):
		return "fsin(%s);" % (", ".join(map(str, self.args)))
class Csc(LibraryFunction):
	def __str__(self):
		return "fcsc(%s);" % (", ".join(map(str, self.args)))
class Cos(LibraryFunction):
	def __str__(self):
		return "fcos(%s);" % (", ".join(map(str, self.args)))
class Sec(LibraryFunction):
	def __str__(self):
		return "fsec(%s);" % (", ".join(map(str, self.args)))
class Tan(LibraryFunction):
	def __str__(self):
		return "ftan(%s);" % (", ".join(map(str, self.args)))
class Cot(LibraryFunction):
	def __str__(self):
		return "fcot(%s);" % (", ".join(map(str, self.args)))

class Asin(LibraryFunction):
	def __str__(self):
		return "fasin(%s);" % (", ".join(map(str, self.args)))
class Acsc(LibraryFunction):
	def __str__(self):
		return "facsc(%s);" % (", ".join(map(str, self.args)))
class Acos(LibraryFunction):
	def __str__(self):
		return "facos(%s);" % (", ".join(map(str, self.args)))
class Asec(LibraryFunction):
	def __str__(self):
		return "fasec(%s);" % (", ".join(map(str, self.args)))
class Atan(LibraryFunction):
	def __str__(self):
		return "fatan(%s);" % (", ".join(map(str, self.args)))
class Acot(LibraryFunction):
	def __str__(self):
		return "facot(%s);" % (", ".join(map(str, self.args)))


lib = Library(this_lib, "0.0.2", {
	"sin": Sin, 
	"csc": Csc, 
	"cos": Cos, 
	"sec": Sec, 
	"tan": Tan, 
	"cot": Cot, 

	"asin": Asin, 
	"acsc": Acsc, 
	"acos": Acos, 
	"asec": Asec, 
	"atan": Atan,
	"acot": Acot,
	

}, {
	
})
