# -*- coding: utf-8 -*-

from parsing import parser, regexes

from ezlogging import log
import re


def get_c_st(ret):
	"""
	String that is C code from ret
	If ret is literal C code, ret is returned. Otherwise, we resolve the C call.
	"""
	if not isinstance(ret, list) and parser.is_literal(ret):
		return ret
	else:
		if not isinstance(ret, list):
			return ""
		return type_resolve_dict[ret[0]](ret[1][0], ret[1][1])

def reg_args(args):
	"""
	Regularizes arguments:
	  - Adds init statements if neccessary, and assures that vars qualify
	"""
	if isinstance(args, str):
		return reg_args([args])
	args = map(parser.get_var, args)
	ret = []
	global var
	for arg in args:
		if parser.is_valid_arg(arg) and arg not in var and "_tmp_" not in arg and arg not in not_vars and arg not in protected_words:
			var.add(arg)
			ret.append("mpfr_t %s; mpfr_init(%s);" % (arg, arg))
	return (args, ret)

def c_noarg_call(fname, args):
	"""
	Represents a call to a function with no arguments
	"""
	args = reg_args(args)
	return "%sezc_%s(%s);" % ("".join(args[1]), fname, args[0][0])
def c_call(fname, args):
	"""
	A typical C call to most functions.
	"""
	args = reg_args(args)
	return "%s ezc_%s(%s);" % ("".join(args[1]), fname, ",".join(args[0]))
def c_user_call(fname, args):
	"""
	Calls a user defined function
	"""
	args = reg_args(args)
	return "%s ezc_%s(%s);" % ("".join(args[1]), fname.replace("@", ""), ", ".join(args[0]))
def c_prompt(fname, args):
	"""
	Special case for the `prompt` function
	"""
	rargs = args[1:]
	args = [args[0]]
	args = reg_args(args)
	return "%s\nezc_%s(%s, %s);" % ("".join(args[1]), fname, args[0][0], " ".join(rargs).replace("'", "\""))
def c_echo(fname, args):
	"""
	Special case for the `echo` function
	"""
	return "ezc_%s(\"%s\");" % (fname, " ".join(args))
def c_printf(fname, args):
	"""
	Special case for the `printf` function
	"""
	global printf_replaces
	val = " ".join(args).replace("'", "\"")

	val = val.replace(",", " \\b,")
	val = val.replace("\\", " \\b\\")

	s_args = val.split("\"")[1].split("@")[1:]

	for i in range(0, len(s_args)):
		s_args[i] = "@"+s_args[i].split(" ")[0]

	p_args = val.split("\"")[-1].split(",")[1:]
	for i in range(0, len(p_args)):
		p_args[i] = p_args[i].split(" ")[1]


	i = 0
	for x in re.findall(regexes.valid_printf_varinject, val):
		p_args = p_args + [x[1]]
		val = val.replace(x[0], "@F", 1)
		s_args[i] = "@F"
		i += 1


	if len(p_args) != len(s_args):
		log.err("Transferring arguments", ["ERROR: Different number of formats and arguments", s_args, p_args])

	for x, f in printf_convert_args:
		for i in range(0, len(s_args)):
			if s_args[i] == x:
				p_args[i] = f(p_args[i])

	val = "\"%s\"" % (val.split("\"")[1])
	for fr, to in printf_replaces:
		val = val.replace(fr, to)

	p_args_str = ""
	if len(p_args) > 0:
		p_args_str = ", %s" % (", ".join(p_args))
	
	ret = "mpfr_%s(%s%s);" % (fname, val, p_args_str)
	return ret
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
	return "%sif (mpfr_cmp(%s, %s) %s 0) {" % ("".join(args[1]), args[0][0], args[0][2], args[0][1])
def c_for_call(op, args):
	if len(args) <= 3:
		args = args + ["ezc_next_const(\"1\")"]
	args = reg_args(args)
	return "%sfor (ezc_set(%s, %s); mpfr_cmp(%s, %s) == -ezc_sgn(%s); ezc_add(%s, %s, %s)) {" % ("".join(args[1]), args[0][0], args[0][1], args[0][0], args[0][2], args[0][3], args[0][0], args[0][0], args[0][3])
def c_elseblock(op, args):
	return "} else {"
def c_endblock(op, args):
	return "}"

def get_function_translate(fname, args):
	"""
	Translates fname into a function, and calls it with args.
	Returns the C code to perform this.

	This uses the variables function_alias, functions_translate_funcs to look it up
	"""
	fname = fname.strip()
	if fname in functions_alias:
		return get_function_translate(functions_alias[fname], args)
	if fname in functions_translate_funcs:
		return functions_translate_funcs[fname](fname, args)
	return functions_translate_funcs["__default__"](fname, args)

def get_operator_translate(op, args):
	"""
	Similar to get_function_translate, but for operators

	This uses op_map_funcs to loop it up
	"""
	global operators_translate_funcs
	op = op.strip()
	if op in operators_translate_funcs:
		return operators_translate_funcs[op](op, args)
	return operators_translate_funcs["__default__"](op_map_funcs[op], args)

def get_user_func_translate(ufunc, args):
	"""
	Similar to get_function_translate, but for user functions
	"""
	ufunc = ufunc.strip()
	return c_user_call(ufunc, args)


def printf_Z(val):
	return "ezc_mpzstr(%s)" % (val)


printf_convert_args = [
	("@Z", printf_Z)
]

printf_replaces = [
	("@", "%"),
	("%F", "%Rf"),
	("%Z", "%s")	
]

var = set()
"""Variables registered"""

not_vars = []
"""Specific to not use tmp vars (user functions)"""

protected_words = ["RETURN", "NaN", "INF", "NINF", "set"]
"""Constants"""

functions = "binomcoef,binompdf,binomcdf,normalpdf,normalcdf,erf,free,prompt,if,else,file,fi,for,rof,prec,add,sub,mul,div,pow,mod,\",mpz,var,rawvar,intvar,rawintvar,set,sqrt,\\√,cbrt,min,max,near,trunc,rand,fact,echo,printf,hypot,exp,log,agm,gamma,factorial,zeta,\\ζ,pi,deg,rad,sin,cos,tan,asin,acos,atan,csc,sec,cot,acsc,asec,acot,sinh,cosh,tanh,asinh,acosh,atanh,csch,sech,coth,acsch,asech,acoth".split(",")
"""Callable functions"""

operators = "~,^,choose,!,*,/,÷,%,+,-,~,?".split(",")
"""Callable operators"""

order_op = [group.split(",") for group in "?,!,~,choose,,^,,*,/,÷,%,,+,-".split(",,")]
"""Order of operators"""

op_map_funcs = {
	"+": "add",
	"-": "sub",
	"*": "mul",
	"/": "div",
	"÷": "div",
	"^": "pow",
	"%": "mod",
	"~": "near",
	"?": "rand",
	"choose": "binomcoef",
	"!": "fact",
}
"""Maps operators to functions"""

operators_translate_funcs = {
	"__default__": get_function_translate
}
"""Translates operators to functions"""

functions_alias = {
	"ζ": "zeta",
	"\"": "echo",
	"√": "sqrt"
}
"""Aliases for functions"""

functions_translate_funcs = {
	"__default__": c_call,
	"echo": c_echo,
	"printf": c_printf,
	"prompt": c_prompt,
	"near": c_call_optional_call,
	"log": c_call_optional_call,
	"rand": c_call_optional_call,
	"prec": c_prec,
	"if": c_if_call,
	"for": c_for_call,
	"fi": c_endblock,
	"else": c_elseblock,
	"rof": c_endblock,
	"file": c_file,
	"pi": c_noarg_call
}
"""Type of call for each function"""


type_resolve_dict = {
	"function": get_function_translate,
	"operator": get_operator_translate,
	"user_function": get_user_func_translate
}
"""Types to resolve function"""
