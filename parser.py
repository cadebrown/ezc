"""

Parses function calls, operators, definintions, etc

"""

import re
import compiler

# valid variable regex

def is_literal(line):
	return len(re.findall(literal_c, line)) != 0

valid_break = "[ ,><=]*"
valid_get_arg = "\$[0-9]+"
valid_var = "[_a-zA-Z][_a-zA-Z0-9]*|%s" % (valid_get_arg)
valid_const = "(?:\-|\+)?[0-9\.]+"
valid_const_nosign = "[0-9\.]+"
valid_arg = "(?:%s|%s)" % (valid_var, valid_const)
valid_arg_nosign = "(?:%s|%s)" % (valid_var, valid_const_nosign)
literal_c = ".*;"

valid_return = "return (%s)" % (valid_var)

valid_assign = "(%s)[ ]*=[ ]*" % (valid_var)
#print valid_assign
valid_set = "(%s)[ ]*=[ ]*(%s)" % (valid_var, valid_const)

valid_declare_user_function = "\[([a-zA-Z0-9_]+)\]((?:\ %s)+)" % (valid_var)
valid_end_user_function = "\[\/([a-zA-Z0-9_]+)\]"

# global vars are set in set_regex()
#valid_operator = ""
#valid_function = ""
#valid_user_function = ""
#freeform_regex = ""
#usrf_regex = ""

# 2 + -3 - 5

# sets the regex. This should be called after lib_linker.register_lib has been called for all imports
def set_regex():
	#valid_arg = "[a-z]+"
	valid_ufu = "\@[_a-zA-Z][_a-zA-Z0-9]*"
	valid_c = "[ ]*({(.*)}|(.*);)"
	global valid_operator; global valid_function; global valid_user_function; global valid_noarg_function#; global valid_multi_operator
	global valid_order_op

	flist_regex = "%s" % ("|".join(compiler.functions))

	olist_regex = "\\%s" % ("|\\".join(compiler.operators))

	set_regex = "%s(%s)" % (valid_assign, valid_arg)
	
	valid_operator = "(?:%s)?(%s%s)?(%s)(%s%s)?" % (valid_assign, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
	#valid_multi_operator = "(%s)(?:%s(%s)%s(%s))+(%s%s%s)" % (valid_arg, valid_break, olist_regex, valid_break, valid_arg, valid_break, valid_arg)
	valid_function = "(?:%s)?(%s)[ ]+((?:%s%s)+)" % (valid_assign, flist_regex, valid_break, valid_arg)
	valid_user_function = "(?:%s)?(%s[ ]+)((?:%s%s)+)" % (valid_assign, valid_ufu, valid_break, valid_arg)
	valid_noarg_function = "(?:%s)?(%s)" % (valid_assign, flist_regex)

	valid_order_op = []
	for x in compiler.order_op:
		c_pat = "(?:%s%s%s)?((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (olist_regex, valid_break, valid_arg,valid_arg_nosign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg_nosign, valid_break, olist_regex, valid_break, valid_arg)
		#c_pat = "(?:(?:\+|\-)%s)((%s%s)(%s)(%s%s))|" % (valid_break, valid_arg, valid_break, "\\"+x, valid_break, valid_arg)
		#print c_pat
		valid_order_op.append(c_pat)

	#print valid_operator
	#print valid_function
	#print valid_user_function

c_l = None

# resolves reference. For example, $3 would give the third commandline argument
def get_var(text):
	if "$" in text:
		return "ezc_get_arg(%s)" % (text.replace("$", ""))
	elif re.findall(valid_var, text):
		return text
	elif re.findall(valid_const, text):
		return "ezc_next_const(\"%s\")" % text
	else:
		return "%s" % (text)

def parse_return(call):
	#call = [call[1], (call[0] + call[2]).split()]
	return get_statement("RETURN = %s" % (call))

def parse_func(call):
	call = [call[1], ("%s %s" % (call[0], call[2])).split()]
	return compiler.get_function_translate(call[0], call[1])

def parse_oper(call):
	global c_l		
	if re.findall(valid_set, c_l) and not call[1]:
		return get_statement(c_l.replace("=", "= set"))
	call = [call[2], ("%s %s %s" % (call[0], call[1], call[3])).split()]
	return compiler.get_operator_translate(call[0], call[1])

def parse_user_func(call):
	call = [call[1].replace("@", ""), ("%s %s" % (call[0], call[2])).split()]
	return compiler.get_user_func_translate(call[0], call[1])

def parse_noarg_func(call):
	call = [call[1], call[0]]
	return compiler.get_function_translate(call[0], [call[1]])

def get_statement(line):
	global valid_assign; global c_l
	line = re.sub(' +', ' ', line)
	c_l = line
	if re.findall(valid_return, line):
		line = parse_return(re.findall(valid_return, line)[0])
	elif re.findall(valid_function, line):
		line = parse_func(re.findall(valid_function, line)[0])
	elif re.findall(valid_operator, line):
		line = parse_oper(re.findall(valid_operator, line)[0])
	elif re.findall(valid_user_function, line):
		line = parse_user_func(re.findall(valid_user_function, line)[0])
	elif re.findall(valid_noarg_function, line):
		line = parse_noarg_func(re.findall(valid_noarg_function, line)[0])
	elif re.findall(valid_assign, line) and "set" not in line:
		line = get_statement(line.replace("=", "= set "))
		
	return line

def parse_op_resolve(match):
	if match[0] != '':
		return [match[0], match[2], [match[1], match[3]]]
	else:
		return [match[4], match[6], [match[5], match[7]]]

def resolve_operators(line):
	ret = []
	k_t = False
	for pat in valid_order_op:
		if len(re.findall(pat, line)) >= 1 and not k_t:
			k_t = True
			global needed_var
			needed_var += 1
			resolve = parse_op_resolve(re.findall(pat,line)[0])
			var = "tmp_%dv" % (needed_var)
			st = "%s = %s" % (var, resolve[0])
			line = line.replace(resolve[0], var, 1)
			ret.append(st)
			recurse = resolve_operators(line)
			for n in recurse:
				ret.append(n)
	if not k_t:
		ret.append(line)
	return ret

# parses a line. This looks for operators, userfunctions, functions, assignment, functions with no arguments, and then freeform(default).
def parse_line(line):	
	global valid_declare_user_function; global valid_end_user_function
	#print valid_declare_user_function
	#print re.findall(valid_declare_user_function, line)
	if re.findall(valid_declare_user_function, line):
		res = re.findall(valid_declare_user_function, line)[0]
		compiler.user_funcs += "void __%s(mpfr_t RETURN, mpfr_t %s) {" % (res[0], ", mpfr_t ".join(res[1].split()))
		compiler.is_func = True		
		return ""
	if re.findall(valid_end_user_function, line):
		compiler.user_funcs += "}"
		compiler.is_func = False
		return ""
	global needed_var

	try:
		_never_use = needed_var
	except:
		needed_var = 0

	#needed_var = 0
	line_expand = expand_line(line).replace("(", "").replace(")", "").split("\n")
	to_process = []
	#to_process = line_expand
	for x in line_expand:
		for y in resolve_operators(x):
			#print y
			to_process.append(y)
	res = ""
	
	for x in to_process:
		res += get_statement(x) + "\n"
		
	return res

def get_nested(line):
	to_ret = []
	c_group = ""
	paren_level = 0
	has_been_1 = False
	for char in line:
		if char == ')':
			paren_level -= 1
		if paren_level >= 1:
			c_group += char
		if paren_level == 0 and has_been_1:
			if c_group:
				to_ret.append(c_group)
			c_group = ""
		if char == '(':
			has_been_1 = True
			paren_level += 1
	return to_ret

def expand_line(line):
	global unexpanded_line; global needed_var
	if get_nested(line):
		to_do = get_nested(line)
		ret = ""
		for x in range(0, len(to_do)):
			needed_var += 1
			tmp_var = "tmp_%dv" % (needed_var)
		
			line = line.replace(to_do[x], tmp_var, 1)
			ret += expand_line(tmp_var + " = " + to_do[x]) + "\n"
		return ret + line
	return line