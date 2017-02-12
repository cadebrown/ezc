# -*- coding: utf-8 -*-
import ezdata

import compiling
from compiling import resolve

import parsing
from parsing import parser

from ezlogging import log

def get_c_file():
	"""
	Return transpiled EZC into C code, which can be compiled by cc
	This should be called after add_code and right before compile step.
	"""
	return compiling.includes + compiling.default_file + compiling.user_funcs + compiling.start + compiling.main + compiling.end

def add_c_code(file_contents):
	compiling.default_file += file_contents

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
	to_read = True
	c_read = True
	compiling.line_num = 0
	for line in lines:
		res = ""
		compiling.line_num += 1
		if parser.is_literal(line):
			cinfo = parsing.EZCInfo("C Code detected", line, compiling.line_num)
			log.info("Compiling", str(cinfo))
			res += "\n\t" + line
		else:
			if parser.is_user_function(line):
				compiling.is_func = not compiling.is_func
				if compiling.is_func:
					resolve.go_in_func()
				else:
					resolve.go_out_func()
				compiling.current_func = parser.is_user_function(line)[2]
				compiling.user_funcs += parser.is_user_function(line)[0]
				compiling.not_vars = parser.is_user_function(line)[1]
			else:
				if "###" in line:
					line = line[:line.index("###")]
					to_read = not to_read
				if "#" in line:
					line = line[:line.index("#")]
				if line != "" and (to_read or c_read):
					res += compile_line(line)
				c_read = to_read
		if compiling.is_func:
			compiling.user_funcs += res
		else:
			compiling.main += res
