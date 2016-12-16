# -*- coding: utf-8 -*-
from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
/*

	libTrig

*/

int get_use_deg() {
	char *x = getenv("EZC_DEG");
	if (x == NULL) return 0;
	if ((strcmp(x, "no") == 0) || (strcmp(x, "NO") == 0) || (strcmp(x, "0") == 0) || (strcmp(x, "-1") == 0)) return 0;
	return 1;
}

void fdeg(mpfr_t r, mpfr_t a) {
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_const_pi(__tmp, EZC_RND);
	mpfr_mul_ui(r, a, 180, EZC_RND);
	mpfr_div(r, r, __tmp, EZC_RND);

	mpfr_clear(__tmp);
}
void frad(mpfr_t r, mpfr_t a) {
	mpfr_t __tmp; mpfr_init(__tmp);
	mpfr_const_pi(__tmp, EZC_RND);
	mpfr_mul(r, a, __tmp, EZC_RND);
	mpfr_div_ui(r, r, 180, EZC_RND);

	mpfr_clear(__tmp);
}
void fsin(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_sin(r, r, EZC_RND); 
}
void fcsc(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_csc(r, r, EZC_RND); 
}
void fcos(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_cos(r, r, EZC_RND); 
}
void fsec(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_sec(r, r, EZC_RND); 
}
void ftan(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_tan(r, r, EZC_RND); 
}
void fcot(mpfr_t r, mpfr_t a) { 
	if (get_use_deg()) { frad(r, a); } else { fset(r, a); }
	mpfr_cot(r, r, EZC_RND); 
}

void fasin(mpfr_t r, mpfr_t a) { 
	mpfr_asin(r, a, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void facsc(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_asin(r, r, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void facos(mpfr_t r, mpfr_t a) { 
	mpfr_acos(r, a, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void fasec(mpfr_t r, mpfr_t a) { 
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_acos(r, r, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void fatan(mpfr_t r, mpfr_t a) { 
	mpfr_atan(r, a, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void fatan2(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_atan2(r, a, b, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}
void facot(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_atan(r, r, EZC_RND); 
	if (get_use_deg()) fdeg(r, r); 
}

void fsinh(mpfr_t r, mpfr_t a) {
	mpfr_sinh(r, a, EZC_RND);
}
void fcsch(mpfr_t r, mpfr_t a) {
	mpfr_csch(r, a, EZC_RND);
}
void fcosh(mpfr_t r, mpfr_t a) {
	mpfr_cosh(r, a, EZC_RND);
}
void fsech(mpfr_t r, mpfr_t a) {
	mpfr_sech(r, a, EZC_RND);
}
void fhtanh(mpfr_t r, mpfr_t a) {
	mpfr_tanh(r, a, EZC_RND);
}
void fhcoth(mpfr_t r, mpfr_t a) {
	mpfr_coth(r, a, EZC_RND);
}


void fasinh(mpfr_t r, mpfr_t a) {
	mpfr_asinh(r, a, EZC_RND);
	mpfr_div_ui(r, a, 180, EZC_RND);
}
void facsch(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_asinh(r, r, EZC_RND);
}
void facosh(mpfr_t r, mpfr_t a) {
	mpfr_acosh(r, a, EZC_RND);
}
void fasech(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_acosh(r, r, EZC_RND);
}
void fatanh(mpfr_t r, mpfr_t a) {
	mpfr_atanh(r, a, EZC_RND);
}
void facoth(mpfr_t r, mpfr_t a) {
	mpfr_ui_div(r, 1, a, EZC_RND);
	mpfr_atanh(r, r, EZC_RND);
}


"""

class Deg(LibraryFunction):
	def __str__(self):
		return "fdeg(%s);" % (", ".join(map(str, self.args)))
class Rad(LibraryFunction):
	def __str__(self):
		return "frad(%s);" % (", ".join(map(str, self.args)))


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

class Sinh(LibraryFunction):
	def __str__(self):
		return "fsinh(%s);" % (", ".join(map(str, self.args)))
class Csch(LibraryFunction):
	def __str__(self):
		return "fcsch(%s);" % (", ".join(map(str, self.args)))
class Cosh(LibraryFunction):
	def __str__(self):
		return "fcosh(%s);" % (", ".join(map(str, self.args)))
class Sech(LibraryFunction):
	def __str__(self):
		return "fsech(%s);" % (", ".join(map(str, self.args)))
class Tanh(LibraryFunction):
	def __str__(self):
		return "ftanh(%s);" % (", ".join(map(str, self.args)))
class Coth(LibraryFunction):
	def __str__(self):
		return "fcoth(%s);" % (", ".join(map(str, self.args)))

class Asinh(LibraryFunction):
	def __str__(self):
		return "fasinh(%s);" % (", ".join(map(str, self.args)))
class Acsch(LibraryFunction):
	def __str__(self):
		return "facsch(%s);" % (", ".join(map(str, self.args)))
class Acosh(LibraryFunction):
	def __str__(self):
		return "facosh(%s);" % (", ".join(map(str, self.args)))
class Asech(LibraryFunction):
	def __str__(self):
		return "fasech(%s);" % (", ".join(map(str, self.args)))
class Atanh(LibraryFunction):
	def __str__(self):
		return "fatanh(%s);" % (", ".join(map(str, self.args)))
class Acoth(LibraryFunction):
	def __str__(self):
		return "facoth(%s);" % (", ".join(map(str, self.args)))



lib = Library(this_lib, "0.0.2", {
	"deg": Deg, 
	"rad": Rad, 


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
	
	"sinh": Sinh, 
	"csch": Csch, 
	"cosh": Cosh, 
	"sech": Sech, 
	"tanh": Tanh, 
	"coth": Coth, 

	"asinh": Asinh, 
	"acsch": Acsch, 
	"acosh": Acosh, 
	"asech": Asech, 
	"atanh": Atanh, 
	"acoth": Acoth, 
}, {
	
})
