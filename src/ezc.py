import argparse
import ezlogging
import ezc2pc
import pc2py

parser = argparse.ArgumentParser(description='Compile EZC Language. v{0}'.format(ezlogging.version))

parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
parser.add_argument('-o', default="o.py", help='Output file')

for SUB_OPT in ezc2pc.SUB_OPTS:
	parser.add_argument('-{0}{1}'.format(ezc2pc.SUB_OPT_STR, SUB_OPT), nargs='*', help="Automatically passed to {0}".format(ezc2pc.SUB_NAME))

for SUB_OPT in pc2py.SUB_OPTS:
	parser.add_argument('-{0}{1}'.format(pc2py.SUB_OPT_STR, SUB_OPT), nargs='*', help="Automatically passed to {0}".format(pc2py.SUB_NAME))

args = parser.parse_args()

SUB_OPT_PC = []

vargs = vars(args)
for SUB_OPT in vargs:
	if SUB_OPT.startswith(ezc2pc.SUB_OPT_STR) and vargs[SUB_OPT] != None:
		SUB_OPT_PC = SUB_OPT_PC + ["-{0}".format(SUB_OPT.replace(ezc2pc.SUB_OPT_STR, "", 1))]
		for LIST_OPT in vargs[SUB_OPT]:
			SUB_OPT_PC = SUB_OPT_PC + [LIST_OPT]

SUB_OPT_PY = []

for SUB_OPT in vargs:
	if SUB_OPT.startswith(pc2py.SUB_OPT_STR) and vargs[SUB_OPT] != None:
		SUB_OPT_PY = SUB_OPT_PY + ["-{0}".format(SUB_OPT.replace(pc2py.SUB_OPT_STR, "", 1))]
		for LIST_OPT in vargs[SUB_OPT]:
			SUB_OPT_PY = SUB_OPT_PY + [LIST_OPT]

ezc2pc.main(args.files)

pc2py.main([x + ".pc" for x in args.files])


