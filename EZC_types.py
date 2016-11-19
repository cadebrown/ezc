class Library():
	def __init__(self, text, version, funcs, opers):
		self.ver = version
		self.text = text
		self.funcs = funcs
		self.opers = opers

	def __str__(self):
		return self.text

class LibraryFunction():
    def __init__(self, *args):
        self.args = args

	def __str__(self):
		return self.__class__.__name__ + "(%s);" % (", ".join(self.args))
