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


for src in sources:
    src = src.split("/")[-1]

log.info("Sources", sources)


compiler.init_cmp()

# loops through, compiling and saving
for i in range(0, len(sources)):
    src = sources[i]
    log.info("Transpiling", "%s" % (src))
    fsrc = open(src, "r").read()

    # EZC is compiled to C
    compiler.add_code(fsrc)
    
    # we write C file
fout = open("tmp/out.c", "w+")
fout.write(compiler.get_c_file())
fout.close()

# Compile the intermediate lang
cmd = "cc %s -lmpfr -lm -o %s" % ("tmp/out.c", args.o)
clearcmd = "rm %s" % (" ".join("tmp/out.c"))
log.info("Compilation", cmd)
compile_proc = Popen(cmd, shell=True)
compile_proc.wait()

if args.r:
    log.info("Clean", "Removing Files")
    clear_proc = Popen(clearcmd, shell=True)
    clear_proc.wait()

log.info("Executable", args.o)
