from shared import LibraryFunction, Library


import re, parser
import lib_linker

this_lib = """
// sets the precision 
void prec_literal(int x) {
	if (x < EZC_PREC) x = EZC_PREC;
	_prec = x;
	mpfr_set_default_prec((int)x);
}
// sets the precision to a cmdline argument
void prec_index(int index) {
	if (index >= _argc) {
		prec_literal(EZC_PREC);
	} else {
		prec_literal(strtoll(_argv[index], NULL, 10));
	}
}
// adds a and b, normal floating point
void add(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_add(r, a, b, GMP_RNDN); }
// subtracts b from a, normal floating point
void sub(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_sub(r, a, b, GMP_RNDN); }
// multiplies a and b, normal floating point
void mul(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_mul(r, a, b, GMP_RNDN); }
// divides a by b, normal floating point
void fdiv(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_div(r, a, b, GMP_RNDN); }
// takes a to the bth power, normal floating point
void fpow(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_pow(r, a, b, GMP_RNDN); }

// truncates a to the nearest int
void ftrunc(mpfr_t r, mpfr_t a) { mpfr_trunc(r, a); }
// computes a - floor(a/b)*b (modulo)
void ffmod(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_fmod(r, a, b, GMP_RNDN);
}
// computes the first multiple of b towards a.
// Example, ftrunc_mult(r, 5, 10) = 0 because 0 is the largest multiple of ten less than or equal to 5
// ftrunc_mult(r, 23, 7) = 21, because 21 = 3 * 7
void ftrunc_mult(mpfr_t r, mpfr_t a, mpfr_t b) {
	mpfr_t __ftrunc_mult_tmp; mpfr_init(__ftrunc_mult_tmp);
	ffmod(__ftrunc_mult_tmp, a, b);
	sub(r, a, __ftrunc_mult_tmp);
	mpfr_clear(__ftrunc_mult_tmp);
}

// echos a message
void echo(char msg[]) { printf("%s\\n", msg); }
// prints a var
void var(char name[], mpfr_t a) { mpfr_printf(\"%s : %.*Rf \\n\", name, _bprec, a); }
// initializes and sets a
void initset(mpfr_t a, char val[]) { mpfr_init(a); mpfr_set_str(a, val, 10, GMP_RNDN); }
// sets a to the value of the text in val
void set(mpfr_t a, char val[]) { mpfr_set_str(a, val, 10, GMP_RNDN); }
// copies a variable
void fset(mpfr_t a, mpfr_t b) { mpfr_set(a, b, GMP_RNDN); }

//finds the maximum of a and b
void _fmax(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_max(r, a, b, GMP_RNDN); }
//finds the minimum of a and b
void _fmin(mpfr_t r, mpfr_t a, mpfr_t b) { mpfr_min(r, a, b, GMP_RNDN); }
// little function to return the sign of r
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
		if parser.is_literal_string(" ".join(self.args)):
			return str(Echo(*self.args))
		else:
			return "var(\"%s\", %s);" % ((self.args[0], ) * 2)
class Echo(LibraryFunction):
	def __str__(self):
		if parser.is_literal_string(" ".join(self.args)):
			return "echo(%s);" % (" ".join(self.args))
		else:
			return str(Var(" ".join(self.args)))

class Min(LibraryFunction):
	def __str__(self):
		return "_fmin(%s);" % (", ".join(map(str, self.args)))
class Max(LibraryFunction):
	def __str__(self):
		return "_fmax(%s);" % (", ".join(map(str, self.args)))

class Trunc(LibraryFunction):
	def __str__(self):
		if len(self.args) == 2:
			return "ftrunc(%s);" % (", ".join(map(str, self.args)))
		else:
			return "ftrunc_mult(%s);" % (", ".join(map(str, self.args)))

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
class Mod(LibraryFunction):
	def __str__(self):
		return "ffmod(%s);" % (", ".join(map(str, self.args)))
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

	"trunc": Trunc, 

	"add": Add, 
	"sub": Sub, 
	"mul": Mul,
	"div": Div, 
	"pow": Pow
}, {
	
	"_": Trunc,

	"+": Add,
	"-": Sub,
	"*": Mul,
	"/": Div,
	"%": Mod,
	"^": Pow,
	"**": Pow
})
