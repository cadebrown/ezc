import argparse
import ezlogging
import ezc2pasm

subprogs = [ezc2pasm]

parser = argparse.ArgumentParser(description='Compile EZC Language. v{0}'.format(ezlogging.version))

parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
parser.add_argument('-o', default="o.py", help='Output file')

for subprog in subprogs:
	for SUB_OPT in subprog.SUB_OPTS:
		parser.add_argument('-{0}{1}'.format(subprog.SUB_OPT_STR, SUB_OPT), nargs='*', help="Automatically passed to {0}".format(subprog.SUB_NAME))

args = parser.parse_args()

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

ezc2pasm.main(args.files)


