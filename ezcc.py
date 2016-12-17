#!/usr/bin/python

import argparse
import os, sys
from subprocess import Popen, PIPE

# our other file
import EZlogger as log
import compiler_delegate as cmp

defaults = {
	"o": "a.out",
	"v": "1",
}

defaults_tmp = {
	"o": "/tmp/a.out",
	"v": "0",
}

cfl = ["vars"]

# set args
parser = argparse.ArgumentParser(description='Compile EZC Language. v2.3.0 http://github.chemicaldevelopment.us/ezc')
parser.add_argument('files', metavar='files', type=str, nargs='*', help='files to compile')
parser.add_argument('-o', default="__default__", help='Output file')

parser.add_argument('-v', default="__default__", type=str, help='Verbosity level')

parser.add_argument('-tmp', default="/tmp/out.c", help='Tmp file')

parser.add_argument('-cc', default="gcc", help='C Compiler')
parser.add_argument('-ccargs', default="", help='C Compiler arguments')
parser.add_argument('-args', default="", help='Arguments ran on executable')

parser.add_argument('-run', action='store_true', help='Run executable')
parser.add_argument('-runfile', action='store_true', help='Run from shebang file')

parser.add_argument('-rem', action='store_true', help='Remove temp files')
parser.add_argument('-remexec', action='store_true', help='Remove executable files')

parser.add_argument('-c', type=str, help='run commandline')
parser.add_argument('-e', action='store_true', help='run from stdin')
# flags
parser.add_argument('-cvars', default="10", help='Number of constants used by compiler')
args = parser.parse_args()
is_tmp = args.run or args.c or args.e or args.runfile

dargs = vars(args)

if args.runfile:
	dargs["args"] = list(args.files[1:])
	del args.files[1:]


def is_default(x):
	return x == "__default__"

for x in defaults:
	if is_default(dargs[x]):
		if is_tmp:
			dargs[x] = defaults_tmp[x]
		else:
			dargs[x] = defaults[x]

log.init(dargs["v"], is_tmp)

cmpflags = dict()

for fl in cfl:
	cmpflags[fl] = dargs["c%s" % (fl)]


log.info("Flags", ["%s: %s" % (x, cmpflags[x]) for x in cmpflags])


if not isinstance(dargs["files"], list):
	dargs["files"] = list(dargs["files"])

cmp.init_all_compile(cmpflags, dargs["tmp"], dargs["o"], dargs["ccargs"])

if dargs["e"]:
	to_run = "\n".join(sys.stdin)
	cmp.compile_c_option(to_run, dargs["o"])
elif dargs["c"]:
	cmp.compile_c_option(dargs["c"], dargs["o"])
else:
	cmp.compile_files(dargs["files"], dargs["o"])


if is_tmp:
	args.rem = True
	args.remexec = True
	cmp.run_file(dargs["o"], dargs["args"])

if args.rem:
	cmp.remove_file(dargs["tmp"])
if args.remexec:
	cmp.remove_file(dargs["o"])

log.end()