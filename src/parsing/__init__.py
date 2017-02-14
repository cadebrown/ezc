
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