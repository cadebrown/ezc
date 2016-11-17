#!/usr/bin/python

import argparse
import os
from subprocess import Popen, PIPE

# set args

parser = argparse.ArgumentParser(description='Compile EZC Language. http://github.chemicaldevelopment.us/ezc')
parser.add_argument('files', metavar='files', type=str, nargs='+', help='files to compile')
parser.add_argument('-l', metavar='lib', type=str, nargs='+', help='libraries to include')
parser.add_argument('-o', default="a.o", help='Output file')
parser.add_argument('-r', action='store_true', help='Remove temp files')
args = parser.parse_args()

# our other file
import EZcompiler as compiler
import EZlogger as log

log.init()

# get sources
sources = args.files

# get libs
libs = args.l
if not libs:
    libs = []

if not os.path.exists("tmp"):
    os.makedirs("tmp")

# these are C lang files
c_files = []

for src in sources:
    src = src.split("/")[-1]
    c_files.append("tmp/%s.c" % (src))

log.info("Sources", sources)

# loops through, compiling and saving
for i in range(0, len(sources)):
    src = sources[i]
    out = c_files[i]
    log.info("Transpiling", "%s to %s" % (src, out))
    fsrc = open(src, "r")
    # split on newline
    lines = fsrc.read().split("\n")
    read = True

    # close file
    fsrc.close()

    # EZC is compiled to C
    str_cmp = compiler.compile_lines(lines, libs)
    
    # we write C file
    fout = open(out, "w+")
    fout.write(str_cmp)
    fout.close()

# Compile the intermediate lang
cmd = "cc %s -lmpfr -lm -o %s" % (" ".join(c_files), args.o)
clearcmd = "rm %s" % (" ".join(c_files))
log.info("Compilation", cmd)
compile_proc = Popen(cmd, shell=True)
compile_proc.wait()

if args.r:
    log.info("Clean", "Removing Files")
    clear_proc = Popen(clearcmd, shell=True)
    clear_proc.wait()

log.info("Executable", args.o)
