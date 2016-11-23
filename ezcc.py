#!/usr/bin/python

import argparse
import os, sys
from subprocess import Popen, PIPE

defaults = {
	"o": "a.out",
	"v": "2",
}

defaults_tmp = {
	"o": "/tmp/a.out",
	"v": "0",
}

# set args
parser = argparse.ArgumentParser(description='Compile EZC Language. v2.1.1 http://github.chemicaldevelopment.us/ezc')
parser.add_argument('files', metavar='files', type=str, nargs='*', help='files to compile')
#parser.add_argument('-l', metavar='libs', type=str, nargs='+', help='libraries to include (unstable)')
parser.add_argument('-tmp', default="/tmp/out.c", help='Tmp directory')
parser.add_argument('-o', default="__default__", help='Output file')
parser.add_argument('-v', default="__default__", type=str, help='Verbosity level')
parser.add_argument('-rem', action='store_true', help='Remove temp files')
parser.add_argument('-remexec', action='store_true', help='Remove executable files')
parser.add_argument('-run', action='store_true', help='Run executable')
parser.add_argument('-c', type=str, help='run commandline')
parser.add_argument('-e', action='store_true', help='run from stdin')
args = parser.parse_args()
dargs = vars(args)

# our other file
import EZlogger as log
import compiler_delegate as cmp

is_tmp = args.run or args.c or args.e

def is_default(x):
	return x == "__default__"

for x in defaults:
	if is_default(dargs[x]):
		if is_tmp:
			dargs[x] = defaults_tmp[x]
		else:
			dargs[x] = defaults[x]

log.init(dargs["v"])

if not isinstance(dargs["files"], list):
	dargs["files"] = list(dargs["files"])

cmp.init_all_compile(dargs["tmp"], dargs["o"])

if dargs["e"]:
	to_run = "\n".join(sys.stdin)
	cmp.compile_c_option(to_run, args["o"])
elif dargs["c"]:
	cmp.compile_c_option(dargs["c"], dargs["o"])
else:
	cmp.compile_files(dargs["files"], dargs["o"])


if is_tmp:
	args.rem = True
	args.remexec = True
	cmp.run_file(dargs["o"])

if args.rem:
	cmp.remove_file(dargs["tmp"])
if args.remexec:
	cmp.remove_file(dargs["o"])

log.end()