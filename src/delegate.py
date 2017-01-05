from subprocess import Popen, PIPE

import os

from compiling import compiler
from parsing import parser
from ezlogging import log

args = None

def init(_args):
	"""
	Initializes the required libraries for tasks 

	"""
	global args
	args = _args
	compiler.init()

def remove_file(fn):
	"""
	Removes a file
	"""
	clearcmd = "rm %s" % (fn)
	log.info("Clean", clearcmd)
	clear_proc = Popen(clearcmd, shell=True)
	clear_proc.wait()

def run_exec():
	"""
	Runs the compiled executable
	"""
	global args
	if "/" not in args["o"]:
		runcmd = "./%s %s" % (args["o"], args["args"])
	else:
		runcmd = "%s %s" % (args["o"], args["args"])
	log.info("Running", runcmd)
	run_proc = Popen(runcmd, shell=True)
	run_proc.wait()

def get_lib_args():
	"""
	Returns the c compiler's linking options for gmp and mpfr
	"""
	import ezdata
	res = "-lm "
	if ezdata.EZC_LIB:
		res += " " + ezdata.EZC_LIB + " "
	else:
		res += " -lmpfr -lgmp"
	return res

def compile_exec():
	"""
	Compiles the executable.
	There should have already been called transpile, compile_files, or addcode
	"""
	global args
	cmd = "%s %s %s %s -o %s" % (args["cc"], args["ccargs"], args["tmp"], get_lib_args(), args["o"])
	log.info("Compiling", cmd)
	compile_proc = Popen(cmd, shell=True)
	compile_proc.wait()

def addcode(fs):
	"""
	Transforms EZC code into C code
	"""
	compiler.add_code(fs)

def compile_files(sources):
	"""
	A list of file names
	"""
	global args
	# loops through, compiling and saving
	for src in sources:
		addcode(open(src, "r").read())

	# we write C file
	outf = open(args["tmp"], "w+")
	outf.write(compiler.get_c_file())
	outf.close()

	compile_exec()

def transpile(text):
	"""
	Transpiles text into C, and compiles
	"""
	global args
	addcode(text)
	outf = open(args["tmp"], "w+")
	outf.write(compiler.get_c_file())
	outf.close()
	compile_exec()
