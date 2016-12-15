from shared import LibraryFunction, Library, rem_var

import re, parser
import lib_linker

ver = "0.0.2"

this_lib = """
/*

	libPrint

*/
void echo(char msg[]) { 
	printf("%s\\n", msg); 
}
void echoraw(char msg[]) { 
	printf("%s", msg); 
}

void varbraw(mpfr_t a, int base, int size) {
	if (size < 0) size = 0;
	if (base < 2) base = 2;
	if (base > 62) base = 62; 
	mpfr_out_str(stdout, base, size, a, MPFR_RNDN);
}

void varb(char name[], mpfr_t a, int base, int size) {
	if (size < 0) size = 0;
	if (base < 2) base = 2;
	if (base > 62) base = 62; 
	printf(\"%s : \", name); 
	mpfr_out_str(stdout, base, size, a, MPFR_RNDN);
	printf("\\n");
}

void var(char name[], mpfr_t a) { 
	mpfr_printf(\"%s : %.*Rf \\n\", name, _prec, a); 
}

void varraw(mpfr_t a) { 
	mpfr_printf(\"%.*Rf\", _prec, a); 
}

void intvar(char name[], mpfr_t a) { 
	mpz_t _t_; mpz_init(_t_);
	mpfr_get_z(_t_, a, GMP_RNDZ);
	mpfr_printf(\"%s : %Zd\\n\", name, _t_); 
}
void intvarraw(mpfr_t a) { 
	mpz_t _t_; mpz_init(_t_);
	mpfr_get_z(_t_, a, GMP_RNDZ);
	mpfr_printf(\"%Zd\", _t_); 
}

void file(char name[], mpfr_t a) { 
	FILE *fp = fopen(name, \"w+\");
	mpfr_fprintf(fp, \"%.*Rf \", _prec, a); 
	fclose(fp);
}

"""

class GetArg(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		if len(self.args) < 2:
			self.args.append("1")
		if len(self.args) < 3:
			self.args.append("10")
		map(rem_var, self.args[1:])
		self.args = map(parser.unvar, self.args)
		return "fset(%s, _get_arg_base(%s, %s));" % (self.args[0], self.args[1], self.args[2])

class Echo(LibraryFunction):
	def __str__(self):
		map(rem_var, self.args)
		self.args = map(parser.unvar, self.args)
		return "echo(\"%s\");" % (" ".join(self.args))
class EchoRaw(LibraryFunction):
	def __str__(self):
		map(rem_var, self.args)
		self.args = map(parser.unvar, self.args)
		return "echoraw(\"%s\");" % (" ".join(self.args))

class VarB(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		if len(self.args) < 2:
			self.args.append("10")
		if len(self.args) < 3:
			self.args.append("0")
		map(rem_var, [self.args[1], self.args[2]])
		_base = parser.unvar(self.args[1])
		_size = parser.unvar(self.args[2])
		return "varb(\"%s\", %s, %s, %s);" % (self.args[0], self.args[0], _base, _size)

class VarBRaw(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		if len(self.args) < 2:
			self.args.append("10")
		if len(self.args) < 3:
			self.args.append("0")
		map(rem_var, [self.args[1], self.args[2]])
		_base = parser.unvar(self.args[1])
		_size = parser.unvar(self.args[2])
		return "varbraw(%s, %s, %s);" % (self.args[0], _base, _size)

class Var(LibraryFunction):
	def __str__(self):
		return "var(\"%s\", %s);" % (self.args[0], self.args[0])
class VarRaw(LibraryFunction):
	def __str__(self):
		return "varraw(%s);" % (self.args[0])
class IntVar(LibraryFunction):
	def __str__(self):
		return "intvar(\"%s\", %s);" % (self.args[0], self.args[0])
class IntVarRaw(LibraryFunction):
	def __str__(self):
		return "intvarraw(%s);" % (self.args[0])
class File(LibraryFunction):
	def __str__(self):
		return "file(\"%s.txt\", %s);" % (self.args[0], self.args[0])


lib = Library(this_lib, ver, {
	"get_arg": GetArg,

	"varb": VarB,
	"varb_raw": VarBRaw,
	"var": Var,
	"var_raw": VarRaw,

	"int_var": IntVar,
	"int_var_raw": IntVarRaw,

	"echo": Echo,
	"echo_raw": EchoRaw,

	"\"\"": Echo,

	"file": File

}, {

})
