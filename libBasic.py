from shared import LibraryFunction, Library


import re, parser
import lib_linker

this_lib = """

void prec_literal(int x) {
	if (x < EZC_PREC) x = EZC_PREC;
	_prec = x;
	mpfr_set_default_prec((int)x);
}

void prec_index(int index) {
	if (index >= _argc) {
		prec_literal(EZC_PREC);
	} else {
		prec_literal(strtoll(_argv[index], NULL, 10));
	}
}

void add(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_add(r, a, b, GMP_RNDN); }
void sub(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_sub(r, a, b, GMP_RNDN); }
void mul(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_mul(r, a, b, GMP_RNDN); }
void fdiv(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_div(r, a, b, GMP_RNDN); }
void fpow(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_pow(r, a, b, GMP_RNDN); }

void echo(char msg[]) { printf("%s\\n", msg); }
void var(char name[], mpfr_t a) { mpfr_printf(\"%s : %.*Rf \\n\", name, _bprec, a); }
void set(mpfr_t a, char val[]) { mpfr_set_str(a, val, 10, GMP_RNDN); }
void fset(mpfr_t a, mpfr_t b) { mpfr_set(a, b, GMP_RNDN); }

void _fmax(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_max(r, a, b, GMP_RNDN); }
void _fmin(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_min(r, a, b, GMP_RNDN); }
int _fsgn(mpfr_t r) { return mpfr_sgn(r); }

"""

class Prec(LibraryFunction):
	def __str__(self):
		if "args[" in self.args[0]:
			lib_linker.set_prec("prec_index(%s);" % self.args[0].replace("args[", "").replace("]", ""))
		else:
			lib_linker.set_prec("prec_literal(%s);" % (self.args[0]))
		return ""

class Set(LibraryFunction):
	def __str__(self):
		if re.match(parser.valid_const, self.args[1]):
			return "set(%s, \"%s\");" % (self.args)
		elif re.match(parser.valid_var, self.args[1]):
			return "fset(%s, %s);" % (self.args)
class Var(LibraryFunction):
	def __str__(self):
		return "var(\"%s\", %s);" % ((self.args[0], ) * 2)
class Echo(LibraryFunction):
	def __str__(self):
		return "echo(%s);" % (self.args)

class Min(LibraryFunction):
	def __str__(self):
		return "_fmin(%s);" % (", ".join(map(str, self.args)))
class Max(LibraryFunction):
	def __str__(self):
		return "_fmax(%s);" % (", ".join(map(str, self.args)))

class Add(LibraryFunction):
	def __str__(self):
		return "add(%s);" % (", ".join(map(str, self.args)))
class Sub(LibraryFunction):
	def __str__(self):
		return "sub(%s);" % (", ".join(map(str, self.args)))
class Mul(LibraryFunction):
	def __str__(self):
		return "mul(%s);" % (", ".join(map(str, self.args)))
class Div(LibraryFunction):
	def __str__(self):
		return "fdiv(%s);" % (", ".join(map(str, self.args)))
class Pow(LibraryFunction):
	def __str__(self):
		return "fpow(%s);" % (", ".join(map(str, self.args)))


libBasic = Library(this_lib, "0.0.2", {
	"prec": Prec, 
	"set": Set,
	"var": Var,
	"echo": Echo,

	"min": Min, 
	"max": Max, 

	"add": Add, 
	"sub": Sub, 
	"mul": Mul,
	"div": Div, 
	"pow": Pow
}, {
	
	"+": Add,
	"-": Sub,
	"*": Mul,
	"/": Div,
	"^": Pow,
	"**": Pow
})
