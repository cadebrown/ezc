from subprocess import Popen, PIPE
import os
import compiler, parser
import log

args = None

def init(_args):
	global args
	args = _args

def remove_file(fn):
	clearcmd = "rm %s" % (fn)
	log.info("Clean", clearcmd)
	clear_proc = Popen(clearcmd, shell=True)
	clear_proc.wait()

def run_exec():
	global args
	if "/" not in args["o"]:
		runcmd = "./%s %s" % (args["o"], args["args"])
	else:
		runcmd = "%s %s" % (args["o"], args["args"])
	log.info("Running", runcmd)
	run_proc = Popen(runcmd, shell=True)
	run_proc.wait()

def get_lib_args():
	import ezdata
	res = "-lm "
	if ezdata.EZC_LIB:
		res += " " + ezdata.EZC_LIB
	else:
		res += " -lmpfr -lgmp"
	return res

def compile_exec():
	# Compile the intermediate lang
	global args
	cmd = "%s %s %s %s -o %s" % (args["cc"], args["ccargs"], args["tmp"], get_lib_args(), args["o"])
	log.info("Compiling", cmd)
	compile_proc = Popen(cmd, shell=True)
	compile_proc.wait()

def addcode(fs):
	try:
		compiler.add_code(fs)
	except Exception as e:
		log.err("Adding code", e)

def compile_files(sources):
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
	global args
	addcode(text)
	outf = open(args["tmp"], "w+")
	outf.write(compiler.get_c_file())
	outf.close()
	compile_exec()
