
import ezlogging
import compiling

class EZCSyntaxError(Exception):

	def __init__(self, string, line, linenum, index=None):
		self.string = string
		self.line = line
		self.linenum = linenum
		self.index = index
		if self.index == None:
			self.index = len(line) - 1

	def __str__(self):
		show_line = self.line
		before = (ezlogging.BOLD + ezlogging.INVERT + ezlogging.RED)
		after = (ezlogging.RBOLD + ezlogging.RINVERT + ezlogging.LGRAY)
		show_line = self.line[0:self.index-1] + self.line[self.index-1]
		if len(self.line) > self.index:
			show_line += (before) + self.line[self.index:] + (after)
		show_line_after = " " * self.index + (before) + "^" + (after)
		return self.string + " on line {0} at index {1}\n{2}\n{3}".format(self.linenum, self.index, show_line, show_line_after)

class EZCInfo():

	def __init__(self, string, line, linenum, index=None):
		self.string = string
		self.line = line
		self.linenum = linenum
		self.index = index
		if self.index == None:
			self.index = len(line) - 1

	def __str__(self):
		show_line = self.line
		before = (ezlogging.BOLD + ezlogging.GREEN)
		after = (ezlogging.RBOLD + ezlogging.LGRAY)
		show_line = self.line[0:self.index-1] + self.line[self.index-1]
		if len(self.line) > self.index:
			show_line += (before) + self.line[self.index:] + (after)
		show_line_after = " " * self.index + (before) + "^" + (after)
		return self.string + " on line {0} at index {1}\n{2}\n{3}".format(self.linenum, self.index, show_line, show_line_after)

valid_break = "[ ,><=]*"
valid_get_arg = "\$[0-9]+"
valid_var = "[_a-zA-Z][_a-zA-Z0-9]*|%s" % (valid_get_arg)
valid_const = "(?:\-|\+)?[0-9\.]+(?:e[0-9]+)?"
valid_const_nosign = "[0-9\.][0-9\.]+(?:e[0-9]+)?"
valid_const_sign = "(?:\-|\+)[0-9\.]+(?:e[0-9]+)?"
valid_arg = "(?:%s|%s)" % (valid_var, valid_const)
valid_arg_nosign = "(?:%s|%s)" % (valid_var, valid_const_nosign)
valid_arg_sign = "(?:%s|%s)" % (valid_var, valid_const_sign)
literal_c = ".*;|\{.*\}"

valid_arg_end = "[a-zA-Z0-9\._]"

valid_return = "return (%s)" % (valid_var)

valid_assign = "(%s)[ ]*=[ ]*" % (valid_var)
valid_set = "(%s)[ ]*=[ ]*(%s)" % (valid_var, valid_const)

valid_declare_user_function = "\[([a-zA-Z0-9_]+)\]((?:\ %s)+)" % (valid_var)
valid_end_user_function = "\[\/([a-zA-Z0-9_]+)\]"

_set_const = "[a-zA-Z0-9_]+[ ]*=[ ]*([a-zA-Z0-9_]+[ ]*=[ ]*(?:\+|\-)?[0-9\.]+)"

valid_printf_varinject = "(@\{[ ]*(%s)[ ]*\})" % (valid_arg)
valid_tmp = "__tmp_[q-z]+"


valid_ufu = "\@[_a-zA-Z][_a-zA-Z0-9]*"
valid_c = "[ ]*({(.*)}|(.*);)"

flist_regex = "%s" % ("|".join(compiling.functions))

olist_regex = "\\%s" % ("|\\".join(compiling.operators))

set_regex = "%s(%s)" % (valid_assign, valid_arg)

valid_operator = "(?:%s)?(%s%s)?(%s)(%s%s)?" % (valid_assign, valid_arg, valid_break, olist_regex, valid_break, valid_arg)
#valid_multi_operator = "(%s)(?:%s(%s)%s(%s))+(%s%s%s)" % (valid_arg, valid_break, olist_regex, valid_break, valid_arg, valid_break, valid_arg)
valid_function = "(?:%s)?(%s)[ ]+((?:%s%s)+)" % (valid_assign, flist_regex, valid_break, valid_arg)
valid_user_function = "(?:%s)?(%s[ ]+)((?:%s%s)+)" % (valid_assign, valid_ufu, valid_break, valid_arg)
valid_noarg_function = "(?:%s)?(%s)" % (valid_assign, flist_regex)

valid_order_op = []
for x in compiling.order_op:
	c_pat_sign = "((%s%s)(%s)(%s%s%s))" % (valid_arg_sign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break)
	c_pat_nosign = "((%s%s)(%s)(%s%s%s))" % (valid_arg_nosign, valid_break, "\\"+"|\\".join(x), valid_break, valid_arg, valid_break)
	valid_order_op.append([c_pat_nosign, c_pat_sign])

