#!/usr/bin/python

import argparse
import os, sys
from subprocess import Popen, PIPE

# set args

parser = argparse.ArgumentParser(description='Compile EZC Language. v2.1.1 http://github.chemicaldevelopment.us/ezc')
parser.add_argument('files', metavar='files', type=str, nargs='*', help='files to compile')
#parser.add_argument('-l', metavar='libs', type=str, nargs='+', help='libraries to include (unstable)')
parser.add_argument('-o', default="a.o", help='Output file')
parser.add_argument('-v', default=2, type=int, help='Verbosity level')
parser.add_argument('-tmpd', default="/tmp/", help='Tmp directory')
parser.add_argument('-rem', action='store_true', help='Remove temp files')
parser.add_argument('-remexec', action='store_true', help='Remove executable files')
parser.add_argument('-run', action='store_true', help='Run executable')
parser.add_argument('-c', type=str, help='run commandline')
parser.add_argument('-e', action='store_true', help='run from stdin')
args = parser.parse_args()

# our other file
import EZlogger as log
import compiler_delegate as cmp

log.init(args.v)

if not isinstance(args.files, list):
	args.files = [args.files]

cmp.init_all_compile(args.tmpd)

if args.e:
	to_run = "\n".join(sys.stdin)
	cmp.compile_c_option(to_run, args.o)
elif args.c:
	cmp.compile_c_option(args.c, args.o)
else:
	args.files = list(args.files)
	cmp.compile_files(args.files, args.o)


if args.run or args.c or args.e:
	args.rem = True
	args.remexec = True
	cmp.run_file(args.o)

if args.rem:
	cmp.remove_file()

if args.remexec:
	cmp.remove_exec(args.o)