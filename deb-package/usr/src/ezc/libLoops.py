from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """
"""

# used for the for loop
for_st="""
for (fset(%s, %s); mpfr_cmp(%s, %s) == -fsgn(%s); fadd(%s, %s, %s)) {
"""

class If(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		return "if (mpfr_cmp(%s, %s) %s 0) {" % (self.args[0], self.args[2], self.args[1])
class For(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		import EZcompiler as cmp
		if len(self.args) <= 3:
			self.args = self.args + ["_next_const(\"1.0\")"]
		res = "\n\t" + for_st % (self.args[0], self.args[1], self.args[0], self.args[2], self.args[3], self.args[0], self.args[0], self.args[3])
		return res

class End(LibraryFunction):
	def __str__(self):
		return "}"

class Label(LibraryFunction):
	def __str__(self):
		return "%s :;" % (self.args[0])
class Goto(LibraryFunction):
	def __str__(self):
		return "goto %s;" % (self.args[0])


lib = Library(this_lib, "0.0.2", {
	"label": Label, 
	"goto": Goto, 

	"if": If, 
	"for": For, 
	
	"fi": End,
	"rof": End 
}, {

})
