def init(functions, operators, order_op):
	"""
	Initializes regexes given functions, operators, and operators' order_op
	"""
		#valid_arg = "[a-z]+"
	valid_ufu = "\@[_a-zA-Z][_a-zA-Z0-9]*"
	valid_c = "[ ]*({(.*)}|(.*);)"
	global valid_operator; global valid_function; global valid_user_function; global valid_noarg_function; global valid_order_op

	flist_regex = "%s" % ("|".join(functions))

	olist_regex = "\\%s" % ("|\\".join(operators))

	set_regex = "%s(%s)" % (valid_assign, valid_arg)
	
	valid_operator = "(?:%s)?(%s%s)?(%s)(%s%s)?" % (valid_assign, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
	#valid_multi_operator = "(%s)(?:%s(%s)%s(%s))+(%s%s%s)" % (valid_arg, valid_break, olist_regex, valid_break, valid_arg, valid_break, valid_arg)
	valid_function = "(?:%s)?(%s)[ ]+((?:%s%s)+)" % (valid_assign, flist_regex, valid_break, valid_arg)
	valid_user_function = "(?:%s)?(%s[ ]+)((?:%s%s)+)" % (valid_assign, valid_ufu, valid_break, valid_arg)
	valid_noarg_function = "(?:%s)?(%s)" % (valid_assign, flist_regex)

	valid_order_op = []
	for x in order_op:
		#c_pat = "(?:%s%s%s)?((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (olist_regex, valid_break, valid_arg,valid_arg_nosign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg_nosign, valid_break, olist_regex, valid_break, valid_arg)
		c_first_exception = "(?:=[ ]*)((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (valid_arg_sign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
		c_pat_sign = "((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (valid_arg_sign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
		c_pat_sign_prev = "(?:\+|\-)((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (valid_arg_sign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
		c_pat_nosign = "((%s%s)(%s)(%s%s%s))(?:%s%s%s)?" % (valid_arg_nosign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
		valid_order_op.append([c_first_exception, c_pat_sign_prev, c_pat_nosign, c_pat_sign])



valid_break = "[ ,><=]*"
valid_get_arg = "\$[0-9]+"
valid_var = "[_a-zA-Z][_a-zA-Z0-9]*|%s" % (valid_get_arg)
valid_const = "(?:\-|\+)?[0-9\.]+"
valid_const_nosign = "[0-9\.]+"
valid_const_sign = "(?:\-|\+)[0-9\.]+"
valid_arg = "(?:%s|%s)" % (valid_var, valid_const)
valid_arg_nosign = "(?:%s|%s)" % (valid_var, valid_const_nosign)
valid_arg_sign = "(?:%s|%s)" % (valid_var, valid_const_sign)
literal_c = ".*;"

valid_return = "return (%s)" % (valid_var)

valid_assign = "(%s)[ ]*=[ ]*" % (valid_var)
valid_set = "(%s)[ ]*=[ ]*(%s)" % (valid_var, valid_const)

valid_declare_user_function = "\[([a-zA-Z0-9_]+)\]((?:\ %s)+)" % (valid_var)
valid_end_user_function = "\[\/([a-zA-Z0-9_]+)\]"

_set_const = "[a-zA-Z0-9_]+[ ]*=[ ]*([a-zA-Z0-9_]+[ ]*=[ ]*(?:\+|\-)?[0-9\.]+)"

valid_printf_varinject = "(@\{[ ]*(%s)[ ]*\})" % (valid_arg)
valid_tmp = "__tmp_[q-z]+"
