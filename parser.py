import re
import shared

def register_var(varname):
	if re.findall(valid_var, varname) == [varname]:
		shared.var_set.add(varname)
		return True
	return False

valid_var = "[_a-zA-Z][_a-zA-Z0-9]*"
valid_const = "(\+|\-)?[0-9\.]+"
valid_arg = "(?:\$|\-|\+)?[a-zA-Z0-9._]+"
break_regex = "[ ]*[ ,<>]?[ ]*"
sp_regex = "[ ]*[ ,]?"
f_delim = "[ ]+"

set_regex = ""
functions_regex = ""
operators_regex = ""
freeform_regex = ""


def set_regex():
	global set_regex; global functions_regex; global operators_regex; global freeform_regex

	flist_regex = "%s" % ("|".join(shared.functions.keys()))
	operator_escs = []
	for i in shared.operators.keys():
		operator_escs.append("\\" + "\\".join(iter(i)))
	olist_regex = "%s" % ("|".join(operator_escs))
	assign_regex = "(%s)[ ]*=[ ]*" % (valid_var)
	set_regex = "%s(%s)" % (assign_regex, valid_arg)
	functions_regex = "[ ]*((?:%s)?(?:(%s)[ ]*((?:%s%s)*)))" % (assign_regex, flist_regex, break_regex, valid_arg)
	operators_regex = "[ ]*((?:%s)(?:(%s)%s%s(%s)%s%s(%s)))" % (assign_regex, valid_arg, break_regex, f_delim, olist_regex, f_delim, break_regex, valid_arg)
	freeform_regex = "[ ]*(%s) ([. ]*)" % (flist_regex)

def get_var(text):
	if "$" in text:
		return "args[%s]" % (text.replace("$", ""))
	else:
		return "%s" % (text)

def parse_func(call):
	call = clean_func(call)
	map(register_var, call[2].split())
	return call[0], str(shared.functions[call[1]](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def parse_oper(call):
	call = clean_oper(call)
	map(register_var, call[2].split())
	return call[0], str(shared.operators[call[1]](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def parse_freeform(call):
	call = clean_free(call)
	return call[0], str(shared.functions[call[1]](*map(get_var, filter(None, re.split(sp_regex, call[2])))))

def clean_func(call):
	return call[0], call[2], " ".join([call[1], call[3]])

def clean_oper(call):
	return call[0], call[3], " ".join([call[1], call[2], call[4]])

def clean_free(call):
	return call


def parse_line(line):
	if re.findall(operators_regex, line):
		line = parse_oper(re.findall(operators_regex, line)[0])[1]
	elif re.findall(functions_regex, line):
		line = parse_func(re.findall(functions_regex, line)[0])[1]
	elif re.findall(set_regex, line) and "set" not in line:
		return parse_line(line.replace("=", "= set"))
	elif re.findall(freeform_regex, line):
		line = parse_freeform(re.findall(freeform_regex, line)[0])[1]
	else:
		print line
	return line