import parsing

from parsing import parser
from ezlogging import log

import re

import compiling


def get_c_st(ret):
	"""
	String that is C code from ret
	If ret is literal C code, ret is returned. Otherwise, we resolve the C call.
	"""
	if not isinstance(ret, list) and parser.is_literal(ret):
		return ret
	else:
		if ret == "":
			return ""
		if not isinstance(ret, list):
			return ret
		return compiling.type_resolve_dict[ret[0]](ret[1][0], ret[1][1])
	return ret

def reg_args(args):
	"""
	Regularizes arguments:
	  - Adds init statements if neccessary, and assures that vars qualify
	"""
	if isinstance(args, str):
		return reg_args([args])
	args = list(map(parser.get_var, args))
	ret = []

	for arg in args:
		if parser.is_valid_arg(arg) and "_tmp_" not in arg and arg not in compiling.not_vars and arg not in compiling.protected_words and (arg not in compiling.var):
			compiling.var.add(arg)
			if arg not in compiling.var_lvl:
				compiling.var_lvl[arg] = compiling.c_lvl
			ret.append("mpfr_t {0}; mpfr_init({0});".format(arg))
	return (args, ret)

def go_in_func():
	compiling.var_tmp = compiling.var.copy()
	compiling.var = compiling._var.copy()
	compiling._var = compiling.var_tmp

	compiling.var_lvl_tmp = compiling.var_lvl.copy()
	compiling.var_lvl = compiling._var_lvl.copy()
	compiling._var_lvl = compiling.var_lvl_tmp
	
	compiling.c_lvl_tmp = compiling.c_lvl + 0
	compiling.c_lvl = compiling._c_lvl + 0
	compiling._c_lvl = compiling.c_lvl_tmp


def go_out_func():
	compiling.var_tmp = compiling.var.copy()
	compiling.var = compiling._var.copy()
	compiling._var = set()

	compiling.var_lvl_tmp = compiling.var_lvl.copy()
	compiling.var_lvl = compiling._var_lvl.copy()
	compiling._var_lvl = {}
	
	compiling.c_lvl_tmp = compiling.c_lvl + 0
	compiling.c_lvl = compiling._c_lvl + 0
	compiling._c_lvl = 0


def go_up_lvl():
	compiling.c_lvl += 1

def go_down_lvl():
	_tovar = compiling.var.copy()
	for arg in compiling.var:
		if compiling.var_lvl[arg] == compiling.c_lvl:
			_tovar.remove(arg)

	compiling.var = _tovar
	compiling.c_lvl -= 1


def c_noarg_call(fname, args):
	"""
	Represents a call to a function with no arguments
	"""
	args = reg_args(args)
	return "{0}ezc_{1}({2});".format("".join(args[1]), fname, args[0][0])
def c_call(fname, args):
	"""
	A typical C call to most functions.
	"""
	args = reg_args(args)
	return "{0}ezc_{1}({2});".format("".join(args[1]), fname, ",".join(args[0]))
def c_user_call(fname, args):
	"""
	Calls a user defined function
	"""
	args = reg_args(args)
	return "{0}ezc_{1}({2});".format("".join(args[1]), fname.replace("@", ""), ",".join(args[0]))
def c_prompt(fname, args):
	"""
	Special case for the `prompt` function
	"""
	compiling.full_line = parser.compiling.full_line.replace("'", "\"")
	pstring = compiling.full_line.split("\"")[1]

	args = reg_args(args)
	return "{0}ezc_{1}({2}, \"{3}\");".format("".join(args[1]), fname, args[0][0], pstring)
def c_echo(fname, args):
	"""
	Special case for the `echo` function
	"""
	return "ezc_%s(\"%s\");" % (fname, " ".join(args))
def c_printf(fname, args):
	"""
	Special case for the `printf` function
	"""

	args = reg_args(args)

	compiling.full_line = compiling.full_line.replace("'", "\"")

	string_print = compiling.full_line.split("\"")[1]

	fmt_args = []
	for x in string_print.split("@")[1:]:
		if x[0] == '[':
			ls = x.split("]")[0].replace("[", "").split(",")
			#x = x.replace('[', "").replace("]", "").split(",")
			x = x[0:x.index("]")+1]
			fmt_args.append((x, ls[0], ls[1:]))
		else:
			for fmt in sorted(compiling.printf_convert_fmt.keys(), key=len, reverse=True):
				if x.startswith(fmt):
					fmt_args.append((fmt, fmt, (None, )))

	#fmt_args = [x[0] for x in string_print.split("@")[1:]]
	print_args = [x for x in args[0] if x != ""]

	ci = 0
	for fmt in fmt_args:
		string_print = string_print.replace("@" + fmt[0], compiling.printf_convert_fmt[fmt[1]], 1)
		
		to_call = fmt[2]
		if len(to_call) == 0 or to_call[0] is None:
			print_args[ci] = compiling.printf_convert_args[fmt[1]](print_args[ci])
		else:
			print_args[ci] = compiling.printf_convert_args[fmt[1]](print_args[ci], to_call)

		ci += 1

	return "{0}mpfr_{1}(\"{2}\"{3});".format("".join(args[1]), fname, string_print, "".join([", "+x for x in print_args]))

def c_file(op, args):
	"""
	Special case for the `file` function

	"""
	if len(args) < 2:
		args = args + ["ezc.txt"]
	args[0] = parser.get_var(args[0])
	return "ezc_file(%s, \"%s\");" % (args[0], args[1])
def c_prec(fname, args):
	"""
	Special case for the `prec` funcion.
	Can take:
	  - An expression (`prec (2 * 100 / 50)`)
	  - A commandline argument (`prec $1`)
	  - A constant
	"""
	global start
	if parser.is_valid_arg(args[0]) and "$" in args[0]:
		return "\n\tezc_prec_index(%s);" % (args[0].replace("$", ""))
	elif parser.is_valid_const(args[0]):
		return "\n\tezc_prec_literal(%s);" % (args[0])
	elif parser.is_valid_var(args[0]):
		return "\n\tezc_prec_f(%s);" % (args[0])
	return ""
def c_call_optional_call(fname, args):
	"""
	For overloaded methods, the length of arguments change the function called.
	"""
	args = reg_args(args)
	return "%sezc_%s_%d(%s);" % ("".join(args[1]), fname, len(args[0]), ", ".join(args[0]))
def arg_call(op, args):
	return c_call(op_map_funcs[op], args)
def c_if_call(op, args):
	args = reg_args(args)
	go_up_lvl()
	return "%sif (ezc_cmp(%s, %s) %s 0) {" % ("".join(args[1]), args[0][0], args[0][2], args[0][1])
def c_for_call(op, args):
	if len(args) <= 3:
		args = args + ["ezc_next_const(\"1\")"]
	args = reg_args(args)
	go_up_lvl()
	return "%sfor (ezc_set(%s, %s); mpfr_cmp(%s, %s) == -ezc_sgn(%s); ezc_add(%s, %s, %s)) {" % ("".join(args[1]), args[0][0], args[0][1], args[0][0], args[0][2], args[0][3], args[0][0], args[0][0], args[0][3])
def c_elseblock(op, args):
	return "} else {"
def c_endblock(op, args):
	go_down_lvl()
	return "}"

def get_function_translate(fname, args):
	"""
	Translates fname into a function, and calls it with args.
	Returns the C code to perform this.

	This uses the variables function_alias, functions_translate_funcs to look it up
	"""
	fname = fname.strip()
	if fname in compiling.functions_alias:
		return get_function_translate(compiling.functions_alias[fname], args)
	if fname in compiling.functions_translate_funcs:
		return compiling.functions_translate_funcs[fname](fname, args)
	return compiling.functions_translate_funcs["__default__"](fname, args)

def get_operator_translate(op, args):
	"""
	Similar to get_function_translate, but for operators

	This uses op_map_funcs to loop it up
	"""
	op = op.strip()
	if op in compiling.operators_translate_funcs:
		return compiling.operators_translate_funcs[op](op, args)
	return compiling.operators_translate_funcs["__default__"](compiling.op_map_funcs[op], args)

def get_user_func_translate(ufunc, args):
	"""
	Similar to get_function_translate, but for user functions
	"""
	ufunc = ufunc.strip()
	return c_user_call(ufunc, args)


def printf_Z(val, settings=[]):
	return "ezc_mpzstr({0})".format(val)

def printf_F(val, settings=["ezc_prec"]):
	return "{0}, {1}".format(settings[0], val)

def printf_f(val, settings=[10]):
	return "{0}, {1}".format(settings[0], val)

def printf_sf(val, settings=[10]):
	return "{0}, {1}".format(settings[0], val)
