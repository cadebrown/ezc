"""

Parses function calls, operators, definintions, etc

"""

import re, regexes

def is_user_function(line):
	if re.findall(regexes.valid_declare_user_function, line):
		res = re.findall(regexes.valid_declare_user_function, line)[0]
		return "void __%s(mpfr_t RETURN, mpfr_t %s) {" % (res[0], ", mpfr_t ".join(res[1].split()))
	elif re.findall(regexes.valid_end_user_function, line):
		return "}"

def __fits(pattern, text):
	r = re.findall(pattern, text) 
	return r and r[0] == text

def is_valid_arg(arg):		
	return __fits(regexes.valid_arg, arg)

def is_valid_const(const):
	return __fits(regexes.valid_const, const)

def is_valid_var(var):
	return __fits(regexes.valid_var, var)

def is_literal(line):
	return len(re.findall(regexes.literal_c, line)) != 0

# parses a line. This looks for operators, userfunctions, functions, assignment, functions with no arguments, and then freeform(default).
def parse_line(line):
	global needed_var
	try:
		_never_use = needed_var
	except:
		needed_var = 0

	#needed_var = 0
	to_process = expand_line(line)
	ret = []
	for x in to_process:
		for y in resolve_operators(x):
			ret.append(y)
	return map(get_statement, ret)

def get_statement(line):
	global valid_assign; global c_l
	line = re.sub(' +', ' ', line)
	c_l = line
	k_t = True
	for op in order_parse:
		if k_t:
			rr = re.findall(op[0], line)
			if rr:
				k_t = False
				line = op[1](rr[0])
	if k_t and re.findall(regexes.valid_assign, line) and "set" not in line:
		line = get_statement(line.replace("=", "= set "))
	return line

def get_tmp_var():
	global needed_var
	res = "_tmp_%d" % (needed_var)
	needed_var += 1
	return res

def expand_line(line):
	global unexpanded_line
	if get_nested(line):
		to_do = get_nested(line)
		ret = []
		var_tod = set()
		for x in range(0, len(to_do)):
			tmp_var = get_tmp_var()
			var_tod.add(tmp_var)
			line = line.replace(to_do[x], tmp_var, 1)
			ret.append("mpfr_t %s; mpfr_init (%s);" % (tmp_var, tmp_var))
			res_exp = expand_line(tmp_var + " = " + to_do[x])
			if isinstance(res_exp, list):
				for xx in res_exp:
					ret.append(xx)
			else:
				ret.append(res_exp)
		ret.append(line.replace("(", " ").replace(")", " "))
		for vv in var_tod:
			to_add = "mpfr_clear (%s);" % (tmp_var)
			if to_add not in ret:
				ret.append(to_add)
		return ret
	return [line]

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

# resolves reference. For example, $3 would give the third commandline argument
def get_var(text):
	if "$" in text:
		return "ezc_get_arg(%s)" % (text.replace("$", ""))
	elif is_valid_var(text):
		return text
	elif is_valid_const(text):
		return "ezc_next_const(\"%s\")" % text
	else:
		return "%s" % (text)

c_l = None

def parse_return(call):
	#call = [call[1], (call[0] + call[2]).split()]
	print call
	return get_statement("RETURN = %s" % (call))

def parse_func(call):
	call = [call[1], ("%s %s" % (call[0], call[2])).split()]
	return ["function", [call[0], call[1]]]

def parse_oper(call):
	global c_l
	if re.findall(regexes.valid_set, c_l) and not call[1]:
		return get_statement(c_l.replace("=", "= set"))
	call = [call[2], ("%s %s %s" % (call[0], call[1], call[3])).split()]
	return ["operator", [call[0], call[1]]]

def parse_user_func(call):
	call = [call[1].replace("@", ""), ("%s %s" % (call[0], call[2])).split()]
	return ["user_function", [call[0], call[1]]]

def parse_noarg_func(call):
	call = [call[1], call[0]]
	return ["function", [call[0], call[1]]]

def parse_op_resolve(match):
	if match[0] != '':
		return [match[0], match[2], [match[1], match[3]]]
	else:
		return [match[4], match[6], [match[5], match[7]]]

def resolve_operators(line):
	ret = []
	k_t = False
	for pats in regexes.valid_order_op:
		for pat in pats:
			if len(re.findall(pat, line)) >= 1 and not k_t:
				k_t = True
				resolve = parse_op_resolve(re.findall(pat,line)[0])
				tmp_var = get_tmp_var()
				st = "%s = %s" % (tmp_var, resolve[0])
				line = line.replace(resolve[0], tmp_var, 1)
				ret.append("mpfr_t %s; mpfr_init(%s);" % (tmp_var, tmp_var))
				ret.append(st)
				recurse = resolve_operators(line)
				for n in recurse:
					ret.append(n)
				ret.append("mpfr_clear (%s);" % (tmp_var))
	if not k_t:
		ret.append(line)
	return ret


# sets the regex. This should be called after lib_linker.register_lib has been called for all imports
def init(functions, operators, order_op):
	regexes.init(functions, operators, order_op)
	global order_parse
	order_parse = [(regexes.valid_return, parse_return), (regexes.valid_function, parse_func), (regexes.valid_operator, parse_oper), (regexes.valid_user_function, parse_user_func), (regexes.valid_noarg_function, parse_noarg_func)]
	