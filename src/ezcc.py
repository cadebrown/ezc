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
	from ezlogging import log
	import ezlogging
	import delegate
	import ezdata
	import parsing


	# set args
	parser = argparse.ArgumentParser(description='Compile EZC Language. v{0} http://github.chemicaldevelopment.us/ezc'.format(ezlogging.version))

	parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
	parser.add_argument('-o', default="a.out", help='Output file')

	parser.add_argument('-v', default=2, type=int, help='Verbosity level')

	parser.add_argument('-tmp', default=ezdata.tmp, type=str, help='Tmp file')

	parser.add_argument('-cc', default=ezdata.cc, type=str, help='C Compiler')
	parser.add_argument('-ccargs', default="", type=str, help='C Compiler arguments')
	parser.add_argument('-args', default="", type=str, help='Arguments ran on executable')

	parser.add_argument('-run', action='store_true', help='Run executable')
	parser.add_argument('-runfile', action='store_true', help='Run from shebang file')

	parser.add_argument('-c', type=str, help='run commandline')
	parser.add_argument('-e', action='store_true', help='run from stdin')

	parser.add_argument('-gsl', '--genstaticlib', action='store_true', help='Generates a static library')

	# flags
	parser.add_argument('-cvars', default="10", help='Number of constants used by compiler')

	args = vars(parser.parse_args())

	if args["tmp"] == '':
		args["tmp"] = "./out.c"

	if args["tmp"][-2:] != ".c":
		args["tmp"] = args["tmp"] + ".c"

	do_run = args["c"] or args["e"] or args["run"] or args["runfile"]

	if args["runfile"]:
		args["args"] = " ".join(args["files"][1:])
		del args["files"][1:]

	log.init(args["v"])

	delegate.init(args)

	try:
		if args["genstaticlib"] or (delegate.get_built_static_hash() != delegate.get_live_static_hash()):
			do_run = do_run and delegate.gen_static_lib()
		if args["e"]:
			to_run = "\n".join(sys.stdin)
			do_run = do_run and delegate.transpile(to_run)
		elif args["c"]:
			do_run = do_run and delegate.transpile(args["c"])
		else:
			do_run = do_run and delegate.compile_files(args["files"])

		if do_run:
			delegate.run_exec()

	except parsing.EZCSyntaxError as e:
		log.err("Compiling", str(e))

	pass

if __name__ == '__main__':
	import sys
	sys.exit(main())