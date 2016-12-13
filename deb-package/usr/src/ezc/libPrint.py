from shared import LibraryFunction, Library

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

void var(char name[], mpfr_t a) { 
	mpfr_printf(\"%s : %.*Rf \\n\", name, _prec, a); 
}
void varraw(mpfr_t a) { 
	mpfr_printf(\"%.*Rf\", _prec, a); 
}

void intvar(mpfr_t a) { 
	printf(\"%ld\\n\", mpfr_get_si(a, MPFR_RNDD)); 
}
void intvarraw(mpfr_t a) { 
	printf(\"%ld\", mpfr_get_si(a, MPFR_RNDD)); 
}

void file(char name[], mpfr_t a) { 
	FILE *fp = fopen(name, \"w+\");
	mpfr_fprintf(fp, \"%.*Rf \", _prec, a); 
	fclose(fp);
}

"""

class Echo(LibraryFunction):
	def __str__(self):
		return "echo(\"%s\");" % (" ".join(self.args))
class EchoRaw(LibraryFunction):
	def __str__(self):
		return "echoraw(\"%s\");" % (" ".join(self.args))

class Var(LibraryFunction):
	def __str__(self):
		return "var(\"%s\", %s);" % (self.args[0], self.args[0])
class VarRaw(LibraryFunction):
	def __str__(self):
		return "varraw(%s);" % (self.args[0])
class IntVar(LibraryFunction):
	def __str__(self):
		return "intvar(%s);" % (self.args[0])
class IntVarRaw(LibraryFunction):
	def __str__(self):
		return "intvarraw(%s);" % (self.args[0])
class File(LibraryFunction):
	def __str__(self):
		return "file(\"%s.txt\", %s);" % ((self.args[0], ) * 2)


lib = Library(this_lib, ver, {
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
