# -*- coding: utf-8 -*-
import re, parser, log
import ezdata

is_func = False

default_file=ezdata.EZC_C

var = set()

def c_call(fname, args):
	args = map(parser.get_var, args)
	global var
	for arg in args:
		if re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	return "ezc_%s(%s);" % (fname, ", ".join(args))

def c_user_call(fname, args):
	args = map(parser.get_var, args)
	global var
	for arg in args:
		if re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	#print fname
	#print args
	return "__%s(%s);" % (fname, ", ".join(args))

def c_echo(fname, args):
	return "ezc_%s(\"%s\");" % (fname, " ".join(args))

def c_prec(fname, args):
	global start
	if re.findall(parser.valid_get_arg, args[0]) and re.findall(parser.valid_get_arg, args[0])[0] == args[0]:
		start += "ezc_prec_index(%s);" % (args[0].replace("$", ""))
		return ""
	elif re.findall(parser.valid_const, args[0]) and re.findall(parser.valid_const, args[0])[0] == args[0]:
		start += "\nezc_prec_literal(%s);" % (args[0])
		return ""
	elif re.findall(parser.valid_var, args[0]) and re.findall(parser.valid_var, args[0])[0] == args[0]:
		return "ezc_prec_f(%s);" % (args[0])
	return ""

def c_call_optional_call(fname, args):
	args = map(parser.get_var, args)
	
	global var
	for arg in args:
		if re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	return "\n\tezc_%s_%d(%s);" % (fname, len(args), ", ".join(args))

def arg_call(op, args):
	return c_call(op_map_funcs[op], args)

def c_if_call(op, args):
	args = map(parser.get_var, args)
	global var
	for arg in args:
		if re.findall(parser.valid_var, arg) and re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	return "\n\tif (mpfr_cmp(%s, %s) %s 0) {" % (args[0], args[2], args[1])

def c_for_call(op, args):
	if len(args) <= 3:
		args = args + ["ezc_next_const(\"1\")"]
	args = map(parser.get_var, args)
	global var
	for arg in args:
		if re.findall(parser.valid_var, arg) and re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	return "\n\tfor (ezc_set(%s, %s); mpfr_cmp(%s, %s) == -ezc_sgn(%s); ezc_add(%s, %s, %s)) {" % (args[0], args[1], args[0], args[2], args[3], args[0], args[0], args[3])

def c_file(op, args):
	if len(args) < 2:
		args = args + ["ezc.txt"]
		
	args[0] = map(parser.get_var, [args[0]])[0]
	global var
	for arg in args[0:1]:
		if re.findall(parser.valid_var, arg) and re.findall(parser.valid_var, arg)[0] == arg:
			var.add(arg)
	return "ezc_file(%s, \"%s\");" % (args[0], args[1])

def c_elseblock(op, args):
	return "} else {"

def c_endblock(op, args):
	return "}"


declare_function = "function "

functions = "if,else,fi,for,rof,prec,add,sub,mul,div,pow,mod,\",var,intvar,file,set,sqrt,\\√,cbrt,min,max,near,trunc,rand,fact,echo,hypot,exp,log,logb,agm,gamma,factorial,zeta,\\ζ,pi,deg,rad,sin,cos,tan,asin,acos,atan,csc,sec,cot,acsc,asec,acot,sinh,cosh,tanh,asinh,acosh,atanh,csch,sech,coth,acsch,asech,acoth".split(",")

#operators = "+,-,*,/,^,%,~,?,!,√,ζ".split(",")
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

functions_alias = {
	#"?": "rand",
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
}

def get_function_translate(fname, args):
	global functions_translate_funcs
	fname = fname.strip()
	if fname in functions_alias:
		return get_function_translate(functions_alias[fname], args)
	if fname in functions_translate_funcs:
		return functions_translate_funcs[fname](fname, args)
	return functions_translate_funcs["__default__"](fname, args)

operators_translate_funcs = {
	"__default__": get_function_translate
}

def get_operator_translate(op, args):
	global operators_translate_funcs
	op = op.strip()
	if op in operators_translate_funcs:
		return operators_translate_funcs[op](op, args)
	return operators_translate_funcs["__default__"](op_map_funcs[op], args)

def get_user_func_translate(ufunc, args):
	ufunc = ufunc.strip()
	return c_user_call(ufunc, args)

def get_var_inits():
	res = ["", ""]
	for x in var:
		res[0] += "\n\tmpfr_t %s;" % (x)
		if not re.findall("tmp_[0-9]+v", x):
			res[1] += "\n\tmpfr_init(%s);" % (x)
	return res
user_funcs=""""""

start="""int main(int argc, char *argv[]) {
	ezc_init(argc, argv);"""

main = """"""

end = """
    return 0;
}
"""


def add_code(file_contents):
	#lib_linker.reset_vars()
	lines = file_contents.split("\n")
	"""if declare_function in lines[0]:
		lines[0] = lines[0].replace(":", "")
		ctf = lines[0].replace(declare, "").split(" ")
		lines = lines[1:]
		name = ctf[0]
		args = ctf[1:]
		argss = ", mpfr_t ".join([""] + args)[2:]
		res = compile_lines(lines)
		#user_funcs += usrf % (name, argss, lib_linker.get_var_inits(args) + res)
	else:"""
	add_compile_lines(lines)
	#main += res

def add_compile_lines(lines):
	
	parser.set_regex()

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
			try:
				if "###" in line:
					line = line[:line.index("###")]
					to_read = not to_read
				if "#" in line:
					line = line[:line.index("#")]
				if line != "" and (to_read or c_read):
					#print line
					#print parser.parse_line(line)
					if ":" in line:
						res += "\n\t%s" % ("\n\t".join(map(parser.parse_line, line.split(":"))))
					else:
						res += "\n\t%s" % (parser.parse_line(line))
				c_read = to_read
			except Exception as e:
				log.err("Compiling", ["(Line %d) Error while parsing" % line_num, line])
				log.err("Compiling", str(e))
				print str(e)
		if is_func:
			user_funcs += res
		else:
			main += res

def get_c_file():
	return default_file + get_var_inits()[0] + user_funcs + start + get_var_inits()[1] + main + end
	#return start + lib_linker.get_lib_code() + user_funcs + main + lib_linker.get_prec_init() + var_inits + main_code + end

