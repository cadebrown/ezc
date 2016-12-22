# -*- coding: utf-8 -*-
from parsing import parser
from logging import log
import ezdata
import resolve

def init():
	"""
	Initialize parsing with functions from resolve module
	"""
	parser.init(resolve.functions, resolve.operators, resolve.order_op)

def get_c_file():
	"""
	Return transpiled EZC into C code, which can be compiled by cc
	This should be called after add_code and right before compile step.
	"""
	return default_file + user_funcs + start + main + end

def add_code(file_contents):
	"""
	Args:
		file_contents: Raw string read from file that is to be processed
	"""
	lines = file_contents.split("\n")
	add_compile_lines(lines)

def compile_line(line):
	ret = ""
	if ":" in line:
		to_proc = line.split(":")
	else:
		to_proc = [line]
	for x in to_proc:
		for y in parser.parse_line(x):
			ret += "\n\t%s" % (resolve.get_c_st(y))
	return ret

def add_compile_lines(lines):
	"""
	Takes an array-like of EZC code which then is translated to C, and added to variables so that it can be compiled by a C compiler.
	"""
	global main; global user_funcs; global is_func
	to_read = True
	c_read = True
	line_num = 0
	for line in lines:
		res = ""
		line_num += 1
		if parser.is_literal(line):
			log.info("Compiling", ["(Line %d) is C code" % line_num, "%s" % (get_c(line))])
			res += "\n\t" + parser.get_c(line)
		else:
			if parser.is_user_function(line):
				is_func = not is_func
				user_funcs += parser.is_user_function(line)[0]
				resolve.not_vars = parser.is_user_function(line)[1]
			else:
				if "###" in line:
					line = line[:line.index("###")]
					to_read = not to_read
				if "#" in line:
					line = line[:line.index("#")]
				if line != "" and (to_read or c_read):
					res += compile_line(line)
				c_read = to_read
		if is_func:
			user_funcs += res
		else:
			main += res

user_funcs=""""""

start="""int main(int argc, char *argv[]) {
	ezc_init(argc, argv);"""

main = """"""

end = """
    return 0;
}
"""

is_func = False

default_file=ezdata.EZC_C