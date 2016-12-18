"""

Parses function calls, operators, definintions, etc

"""

import re
import compiler

# valid variable regex

def is_literal(line):
	return len(re.findall(literal_c, line)) != 0


valid_break = "[ ,]*"
valid_get_arg = "\$[0-9]+"
valid_var = "[_a-zA-Z][_a-zA-Z0-9]*|%s" % (valid_get_arg)
valid_const = "(?:\+|\-)?[0-9\.]+"
valid_arg = "(?:%s|%s)" % (valid_var, valid_const)
literal_c = ".*;"

valid_assign = "(%s)[ ]?=[ ]?" % (valid_var)


# global vars are set in set_regex()
valid_operator = ""
valid_function = ""
valid_user_function = ""
#freeform_regex = ""
#usrf_regex = ""

# sets the regex. This should be called after lib_linker.register_lib has been called for all imports
def set_regex():

	#valid_arg = "[a-z]+"
	
	valid_ufu = "\@[_a-zA-Z][_a-zA-Z0-9]*"
	valid_c = "[ ]*({(.*)}|(.*);)"
	global valid_operator; global valid_function; global user_valid_function

	flist_regex = "%s" % ("|".join(compiler.functions))

	olist_regex = "\\%s" % ("|\\".join(compiler.operators))

	set_regex = "%s(%s)" % (valid_assign, valid_arg)
	
	valid_operator = "(?:%s)?(%s%s)?(%s)(%s%s)?" % (valid_assign, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
	valid_function = "(?:%s)?(%s)((?:%s%s)+)" % (valid_assign, flist_regex, valid_break, valid_arg)
	valid_user_function = "(?:%s)?(%s)((?:%s%s)+)" % (valid_assign, valid_ufu, valid_break, valid_arg)

	#print valid_operator
	#print valid_function
	#print valid_user_function


# resolves reference. For example, $3 would give the third commandline argument
def get_var(text):
	if "$" in text:
		return "ezc_get_arg(%s)" % (text.replace("$", ""))
	elif re.findall(valid_var, text):
		return text
	elif re.findall(valid_const, text):
		return "ezc_next_const(\"%s\")" % text
	elif not re.findall(consts, text):
		return "%s" % (text)

def parse_func(call):
	call = [call[1], (call[0] + call[2]).split()]
	#call[1] = map(get_var, call[1])
	return compiler.get_function_translate(call[0], call[1])

def parse_oper(call):
	call = [call[2], ("%s %s %s" % (call[0], call[1], call[3])).split()]
	
	#call[1] = map(get_var, call[1])
	#print compiler.get_operator_translate(call[0], call[1])	
	return compiler.get_operator_translate(call[0], call[1])


def get_statement(line):
	global valid_assign
	line = re.sub(' +', ' ', line)
	if re.findall(valid_operator, line):
		line = parse_oper(re.findall(valid_operator, line)[0])
	elif re.findall(valid_function, line):		
		line = parse_func(re.findall(valid_function, line)[0])
	elif re.findall(valid_assign, line) and "set" not in line:
		line = get_statement(line.replace("=", "= set"))
	return line

needed_var = 0

# parses a line. This looks for operators, userfunctions, functions, assignment, functions with no arguments, and then freeform(default).
def parse_line(line):
	global needed_var
	needed_var = 0
	line = expand_line(line).replace("(", "").replace(")", "").split("\n")
	res = ""
	for x in line:
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
			tmp_var = "tmp_%d" % (needed_var)
		
			line = line.replace(to_do[x], tmp_var, 1)
			ret += expand_line(tmp_var + " = " + to_do[x]) + "\n"
		return ret + line
	return line