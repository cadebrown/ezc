"""

Parses function calls, operators, definintions, etc

"""

import re
import shared
import EZlogger as log

# registers a variable, and checks if it is a valid name and then adds it to the global list of names
def register_var(varname):
	if re.findall(valid_var, varname) == [varname] and re.findall(valid_const, varname) != [varname] and re.findall(literal_str, varname) != [varname] and "_next_const" not in varname:# and re.findall(const_call, varname) != [varname]:
		shared.var_set.add(varname)
		return True
	return False

# valid variable regex
valid_var = "[_a-zA-Z][_a-zA-Z0-9]*"
# valid numberical constant
valid_const = "(\+|\-)?[0-9]+"
# valid argument to a function
valid_arg = "(?:\")?(?:\$|\-|\+)?[_a-zA-Z0-9\.]+(?:\")?"
# valid break regex (between function params)
break_regex = "[ ,<>]+"
# splitting arguments
sp_regex = "[ ]*[ ,]?"
# between functions
f_delim = "[ ]+"
# a valid user defined function (from other files)
usrf_id = "(@([_a-zA-Z][_a-zA-Z0-9]*))"
# regex for literal c code
literal_c = "[ ]*({(.*)}|(.*);)"
# literals
consts = "(_next_const(.*))"
literal_str = "\".*\""

def is_literal(line):
	return len(re.findall(literal_c, line)) != 0
def get_c(line):
	code = re.findall(literal_c, line)[0][1:]
	code = filter(None, code)[0]
	if code[-1] != ";":
		code = code + ";"
	return code

def is_literal_string(line):
	return len(re.findall(literal_str, line)) != 0

# global vars are set in set_regex()
set_regex = ""
functions_regex = ""
function_noarg_regex = ""
operators_regex = ""
freeform_regex = ""
usrf_regex = ""

# sets the regex. This should be called after lib_linker.register_lib has been called for all imports
def set_regex():
	global set_regex; global functions_regex; global operators_regex; global freeform_regex; global usrf_regex; global function_noarg_regex
	# list of regex function names
	flist_regex = "%s" % ("|".join(shared.functions.keys()))
	# escapes operator lists
	operator_escs = []
	for i in shared.operators.keys():
		operator_escs.append("\\" + "\\".join(iter(i)))
	# joins the escaped operators
	olist_regex = "%s" % ("|".join(operator_escs))
	# this is valid to assign to (includes = sign)
	assign_regex = "(%s)[ ]*=[ ]*" % (valid_var)
	# valid set value, i.e. a = 1.23423
	set_regex = "%s(%s)" % (assign_regex, valid_arg)
	# function regex to parse for a function call
	functions_regex = "[ ]*((?:%s[ ]+)?(?:((%s))((?:%s%s)+)))" % (assign_regex, flist_regex, break_regex, valid_arg)
	# this parses a function with no arguments (degenerate functions like rof and fi)
	function_noarg_regex = "(%s)" % (flist_regex)
	# operator call
	operators_regex = "[ ]*((?:%s)(?:((%s)%s)?(%s)(%s(%s))?))" % (assign_regex, valid_arg, f_delim, olist_regex, f_delim, valid_arg)
	# a free form call (used for non-norm functions)
	freeform_regex = "[ ]*(%s) ([. ]*)" % (flist_regex)
	# user function
	usrf_regex = "((?:%s)?\@(%s)[ ]*((%s[ ]*)*))" % (assign_regex, valid_var, valid_arg)

# resolves reference. For example, $3 would give the third commandline argument
def get_var(text):
	if "$" in text:
		return "args[%s]" % (text.replace("$", ""))
	elif re.findall(valid_const, text):
		return "_next_const(\"%s\")" % text
	elif not re.findall(consts, text):
		return "%s" % (text)

# parses a function, and returns [existing, replace]. This is used to replace the call with transpiled c code. This is for all parse_* methods
def parse_func(call):
	call = clean_func(call)
	map(register_var, call[2].split())
	return call[0], str(shared.functions[call[1]](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def parse_func_noarg(call):
	call = clean_func_noarg(call)
	return call[0], str(shared.functions[call[1]]())

def parse_oper(call):
	call = clean_oper(call)
	map(register_var, call[2].split())
	return call[0], str(shared.operators[call[1]](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def parse_freeform(call):
	call = clean_free(call)
	return call[0], str(shared.functions["__base"](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def parse_usrf(call):
	call = clean_usrf(call)
	return call[0], "___" + str(shared.functions["__base"](*map(get_var, filter(None, re.split(sp_regex, call[1] + " " + call[2])))))


# cleans a regex call into: [existing, function name, arglist]. Arglist is spaces in between (or any other thing used in sp_regex). This is the same for all clean_*
def clean_func(call):
	return call[0], call[3], " ".join([call[1], call[4]])

def clean_func_noarg(call):
	return call, call

def clean_oper(call):
	return call[0], call[4], " ".join([call[1], call[3], call[6]])

def clean_free(call):
	return call

def clean_usrf(call):
	return call[0], call[2], " ".join([call[1], call[3]])

# parses a line. This looks for operators, userfunctions, functions, assignment, functions with no arguments, and then freeform(default).
def parse_line(line):
	if re.findall(operators_regex, line):
		line = parse_oper(re.findall(operators_regex, line)[0])[1]
	elif re.findall(usrf_regex, line):
		line = parse_usrf(re.findall(usrf_regex, line)[0])[1]
	elif re.findall(functions_regex, line):		
		line = parse_func(re.findall(functions_regex, line)[0])[1]
	elif re.findall(set_regex, line) and "set" not in line:
		line = parse_line(line.replace("=", "= set"))
	elif re.findall(function_noarg_regex, line):
		line = parse_func_noarg(re.findall(function_noarg_regex, line)[0])[1]
	elif re.findall(freeform_regex, line):
		line = parse_freeform(re.findall(freeform_regex, line)[0])[1]
	else:
		log.warn("Parsing", ["\"%s\"" % (line), "Warning: line does not fit normal syntax.", "Interpreting as straight C code"])
		line = line + ";"
		print line
	return line