from shared import LibraryFunction, Library

import re, parser
import lib_linker

this_lib = """

"""

# used for the for loop
for_st="""
for (fset(%s, __start); mpfr_cmp(%s, __stop) == -_fsgn(__step); add(%s, %s, __step)) {
"""

class If(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		return "if (mpfr_cmp(%s, %s) %s 0) {" % (self.args[0], self.args[2], self.args[1])
class For(LibraryFunction):
	def __str__(self):
		self.args = list(self.args)
		import EZcompiler as cmp
		for_macro = "__start = %s : __stop = %s : " % (self.args[1], self.args[2])
		if len(self.args) > 3:
			for_macro += "__step = %s" % (self.args[3])
		else:
			for_macro += "__step = 1.0"
		inits = "\n\t" + cmp.compile_lines([for_macro])
		res = "%s " % (inits)
		res += for_st % ((self.args[0], )*4)
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


libLoops = Library(this_lib, "0.0.2", {
	"label": Label, 
	"goto": Goto, 
	"if": If, 
	"for": For, 
	"fi": End,
	"rof": End 

}, {

})
