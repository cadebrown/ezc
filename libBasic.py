from shared import LibraryFunction, Library


import re, parser
import lib_linker

this_lib = """
/*

	libBasic

*/
void prec_literal(long long x) {
	if (x < EZC_PREC) x = EZC_PREC;
	_prec = x;
	mpfr_set_default_prec((long long)_bprec);
}

void prec_index(int index) {
	if (index < _argc) {
		prec_literal(strtoll(_argv[index], NULL, 10));
	}
}
void prec_init() {
	if (getenv("EZC_PREC") == NULL) {
		if (_argc > 1) {
			int _t_max = EZC_PREC, __tmp_i;
			for (__tmp_i = 1; __tmp_i < _argc; ++__tmp_i) {
				_t_max = (_t_max > strlen(_argv[__tmp_i])) ? _t_max : strlen(_argv[__tmp_i]); 
			}
			prec_literal(_t_max);
		} else {
			prec_literal(EZC_PREC);
		}
	} else {
		prec_literal(strtol(getenv("EZC_PREC"), NULL, 10));
	}	
}

void fadd(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_add(r, a, b, MPFR_RNDD); 
}
void fsub(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_sub(r, a, b, MPFR_RNDD); 
}
void fmul(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_mul(r, a, b, MPFR_RNDD); 
}
void fdiv(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_div(r, a, b, MPFR_RNDD);
}
void fpow(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_pow(r, a, b, MPFR_RNDD); 
}

void fmodulo(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_fmod(r, a, b, MPFR_RNDD);
}

void ftrunc(mpfr_t r, mpfr_t a) { 
	mpfr_trunc(r, a); 
}
void ftrunc_mult(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __ftrunc_mult_tmp; mpfr_init(__ftrunc_mult_tmp);
	mpfr_fmod(__ftrunc_mult_tmp, a, b, MPFR_RNDD);
	mpfr_sub(r, a, __ftrunc_mult_tmp, MPFR_RNDD);
	mpfr_clear(__ftrunc_mult_tmp);
}

void initset(mpfr_t a, char val[]) { 
	mpfr_init(a); 
	mpfr_set_str(a, val, 10, MPFR_RNDD); 
}
void set(mpfr_t a, char val[]) { 
	mpfr_set_str(a, val, 10, MPFR_RNDD); 
}
void fset(mpfr_t a, mpfr_t b) { 
	mpfr_set(a, b, MPFR_RNDD); 
}

void fmaximum(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_max(r, a, b, MPFR_RNDD); 
}
void fminimum(mpfr_t r, mpfr_t a, mpfr_t b) { 
	mpfr_min(r, a, b, MPFR_RNDD); 
}
int fsgn(mpfr_t r) { 
	return mpfr_sgn(r); 
}

"""

class Prec(LibraryFunction):
	def __str__(self):
		if "_get_arg(" in self.args[0]:
			lib_linker.set_prec("prec_index(%s);" % self.args[0].replace("_get_arg(", "").replace(")", ""))
		else:
			lib_linker.set_prec("prec_literal(%s);" % ( re.findall(parser.consts, self.args[0])[0][1] ))
		return ""

class Set(LibraryFunction):
	def __str__(self):
		if re.match(parser.valid_const, self.args[1]):
			return "set(%s, \"%s\");" % (self.args)
		elif re.match(parser.valid_var, self.args[1]):
			return "fset(%s, %s);" % (self.args)

class Min(LibraryFunction):
	def __str__(self):
		return "fminimum(%s);" % (", ".join(map(str, self.args)))
class Max(LibraryFunction):
	def __str__(self):
		return "fmaximum(%s);" % (", ".join(map(str, self.args)))

class Trunc(LibraryFunction):
	def __str__(self):
		if len(self.args) == 2:
			return "ftrunc(%s, %s);" % (self.args[0], self.args[1])
		else:
			return "ftrunc_mult(%s, %s, %s);" % (self.args[0], self.args[2], self.args[1])

class Add(LibraryFunction):
	def __str__(self):
		return "fadd(%s);" % (", ".join(map(str, self.args)))
class Sub(LibraryFunction):
	def __str__(self):
		return "fsub(%s);" % (", ".join(map(str, self.args)))
class Mul(LibraryFunction):
	def __str__(self):
		return "fmul(%s);" % (", ".join(map(str, self.args)))
class Div(LibraryFunction):
	def __str__(self):
		return "fdiv(%s);" % (", ".join(map(str, self.args)))
class Mod(LibraryFunction):
	def __str__(self):
		return "fmodulo(%s);" % (", ".join(map(str, self.args)))
class Pow(LibraryFunction):
	def __str__(self):
		return "fpow(%s);" % (", ".join(map(str, self.args)))


lib = Library(this_lib, "0.0.2", {
	"prec": Prec, 
	"set": Set,

	"min": Min, 
	"max": Max, 

	"trunc": Trunc, 

	"add": Add, 
	"sub": Sub, 
	"mul": Mul,
	"div": Div, 
	"pow": Pow
}, {
	
	"~": Trunc,

	"+": Add,
	"-": Sub,
	"*": Mul,
	"/": Div,
	"%": Mod,
	"^": Pow,
	"**": Pow
})
