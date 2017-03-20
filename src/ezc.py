###                     EZC src/ezc.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#
#  The main source file for EZC. Takes in source and object files to compile.
#
#  TODO:
#    * Use ezlogging/ for formatted output
#
###

def main(argv):

	import argparse
	import ezlogging
	from ezlogging import log
	import ezc2pasm

	subprogs = [ezc2pasm]

	parser = argparse.ArgumentParser(description='Compile EZC Language. v{0}'.format(ezlogging.version))

	parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
	parser.add_argument('-c', default=None, help='Compiles a string of text')
	parser.add_argument('-v', type=int, default=1, help='Verbosity level')
	parser.add_argument('-o', default="o.py", help='Output file')

	for subprog in subprogs:
		for SUB_OPT in subprog.SUB_OPTS:
			parser.add_argument('-{0}{1}'.format(subprog.SUB_OPT_STR, SUB_OPT), nargs='*', default=None, help="Automatically passed to {0}".format(subprog.SUB_NAME))

	args = parser.parse_args(argv)

	log.init(args.v)

	SUB_OPTS = { }

	for subprog in subprogs:
		SUB_OPTS[subprog] = []

	vargs = vars(args)
	for subprog in subprogs:
		for SUB_OPT in vargs:
			if SUB_OPT.startswith(subprog.SUB_OPT_STR) and vargs[SUB_OPT] != None:
				SUB_OPTS[subprog] = SUB_OPTS[subprog] + ["-{0}".format(SUB_OPT.replace(subprog.SUB_OPT_STR, "", 1))]
				for LIST_OPT in vargs[SUB_OPT]:
					SUB_OPTS[subprog] = SUB_OPTS[subprog] + [LIST_OPT]
	if args.c:
		SUB_OPTS[ezc2pasm] += ["-c", args.c]

	ezc2pasm.main(args.files + SUB_OPTS[ezc2pasm])

if __name__ == "__main__":
	import sys
	main(sys.argv[1:])
	exit()
else:
	print ("Please run as main program!")