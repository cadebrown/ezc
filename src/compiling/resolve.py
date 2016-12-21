# -*- coding: utf-8 -*-

from parsing import parser

def get_c_st(ret):
	if not isinstance(ret, list) and parser.is_literal(ret):
		return ret
	else:
		return type_resolve_dict[ret[0]](ret[1][0], ret[1][1])

def reg_args(args):
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
	args = reg_args(args)
	return "%sezc_%s(%s);" % ("".join(args[1]), fname, args[0][0])
def c_call(fname, args):
	args = reg_args(args)
	return "%sezc_%s(%s);" % ("".join(args[1]), fname, ",".join(args[0]))
def c_user_call(fname, args):
	args = reg_args(args)
	return "%s__%s(%s);" % ("".join(args[1]), fname, ", ".join(args[0]))
def c_echo(fname, args):
	return "ezc_%s(\"%s\");" % (fname, " ".join(args))
def c_file(op, args):
	if len(args) < 2:
		args = args + ["ezc.txt"]
	args[0] = parser.get_var(args[0])
	return "ezc_file(%s, \"%s\");" % (args[0], args[1])
def c_prec(fname, args):
	global start
	if parser.is_valid_const(args[0]):
		return "\n\tezc_prec_literal(%s);" % (args[0])
	elif parser.is_valid_arg(args[0]):
		return "\n\tezc_prec_index(%s);" % (args[0].replace("$", ""))
	elif parser.is_valid_var(args[0]):
		return "\n\tezc_prec_f(%s);" % (args[0])
	return ""
def c_call_optional_call(fname, args):
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
	fname = fname.strip()
	if fname in functions_alias:
		return get_function_translate(functions_alias[fname], args)
	if fname in functions_translate_funcs:
		return functions_translate_funcs[fname](fname, args)
	return functions_translate_funcs["__default__"](fname, args)

def get_operator_translate(op, args):
	global operators_translate_funcs
	op = op.strip()
	if op in operators_translate_funcs:
		return operators_translate_funcs[op](op, args)
	return operators_translate_funcs["__default__"](op_map_funcs[op], args)

def get_user_func_translate(ufunc, args):
	ufunc = ufunc.strip()
	return c_user_call(ufunc, args)


var = set()
not_vars = []
protected_words = ["RETURN"]

functions = "if,else,file,fi,for,rof,prec,add,sub,mul,div,pow,mod,\",var,intvar,set,sqrt,\\√,cbrt,min,max,near,trunc,rand,fact,echo,hypot,exp,log,logb,agm,gamma,factorial,zeta,\\ζ,pi,deg,rad,sin,cos,tan,asin,acos,atan,csc,sec,cot,acsc,asec,acot,sinh,cosh,tanh,asinh,acosh,atanh,csch,sech,coth,acsch,asech,acoth".split(",")

operators = "~,^,*,/,÷,%,+,-,~,?".split(",")

order_op = [group.split(",") for group in "?,~,,^,,*,/,÷,%,,+,-".split(",,")]

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
	"!": "fact",
}

operators_translate_funcs = {
	"__default__": get_function_translate
}

functions_alias = {
	"ζ": "zeta",
	"\"": "echo",
	"√": "sqrt"
}

functions_translate_funcs = {
	"__default__": c_call,
	"echo": c_echo,
	"near": c_call_optional_call,
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


type_resolve_dict = {
	"function": get_function_translate,
	"operator": get_operator_translate,
	"user_function": get_user_func_translate
}

