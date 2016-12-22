#!/usr/bin/python

def main():
	"""
	The method that start EZC compiler.
	Contains:
	  - Argument parsing
	  - Interaction with delegate
	"""
	import argparse
	import os, sys
	import subprocess

	sys.dont_write_bytecode=True

	# our other file
	#import EZlogger as log
	from logging import log
	import delegate

	# set args
	parser = argparse.ArgumentParser(description='Compile EZC Language. v%s http://github.chemicaldevelopment.us/ezc' % (log.version))
	parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
	parser.add_argument('-o', default="a.out", help='Output file')

	parser.add_argument('-v', default=0, type=int, help='Verbosity level')

	parser.add_argument('-tmp', default="`mktemp \"${TMPDIR:-/tmp}/XXXXX.c\"`", help='Tmp file')

	parser.add_argument('-cc', default="cc", help='C Compiler')
	parser.add_argument('-ccargs', default="", help='C Compiler arguments')
	parser.add_argument('-args', default="", help='Arguments ran on executable')

	parser.add_argument('-run', action='store_true', help='Run executable')
	parser.add_argument('-runfile', action='store_true', help='Run from shebang file')

	parser.add_argument('-c', type=str, help='run commandline')
	parser.add_argument('-e', action='store_true', help='run from stdin')

	# flags
	parser.add_argument('-cvars', default="10", help='Number of constants used by compiler')

	args = vars(parser.parse_args())

	if "mktemp" in args["tmp"]:
		#args["tmp"] = os.popen("echo " + args["tmp"]).read()
		args["tmp"] = subprocess.check_output("echo " + args["tmp"], shell=True).strip()
		#print ret
	if args["tmp"] == '':
		args["tmp"] = "./out.c"

	do_run = args["c"] or args["e"] or args["run"] or args["runfile"]

	if args["runfile"]:
		args["args"] = list(args["files"][1:])
		del args["files"][1:]

	log.init(args["v"])

	delegate.init(args)

	if args["e"]:
		to_run = "\n".join(sys.stdin)
		delegate.transpile(to_run)
	elif args["c"]:
		delegate.transpile(args["c"])
	else:
		delegate.compile_files(args["files"])

	if do_run:
		delegate.run_exec()

	pass

if __name__ == '__main__':
	import sys
	sys.exit(main())