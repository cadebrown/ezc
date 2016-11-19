from libBasic import libBasic
from libMath import libMath
from libLoops import libLoops
import lib_linker

from parser import parse_line, set_regex, register_var
from shared import start, main, end

main_code = ""

def init_cmp():
	map(lib_linker.register_lib, [libBasic, libMath, libLoops])
	set_regex()

def add_code(file_contents):
	global main_code
	main_code += compile_lines(file_contents.split("\n"))

def compile_lines(lines):
	res = ""
	to_read = True
	c_read = True
	for line in lines:
		if "###" in line:
			line = line[:line.index("###")]
			to_read = not to_read
		if "#" in line:
			line = line[:line.index("#")]
		if line != "" and (to_read or c_read):
			if ";" in line:
				res += "%s" % ("\n\t".join(map(parse_line, line.split(";"))))
			else:
				res += "\n\t%s" % (parse_line(line))
		c_read = to_read
	return res
def get_c_file():
	return start + lib_linker.get_lib_code() + main + lib_linker.get_var_inits() + main_code + end

