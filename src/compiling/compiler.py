# -*- coding: utf-8 -*-
from parsing import parser
from logging import log
import ezdata
import resolve

def init():
	parser.init(resolve.functions, resolve.operators, resolve.order_op)

def get_c_file():
	#return default_file + get_var_inits()[0] + user_funcs + start + get_var_inits()[1] + main + end
	res = default_file + user_funcs + start + main + end
	return res

def add_code(file_contents):
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