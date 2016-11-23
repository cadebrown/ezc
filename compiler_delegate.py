import EZlogger as log
from subprocess import Popen, PIPE
import os
import EZcompiler as compiler


tmp_file = ""
exec_file = ""


def remove_file(fn):
	clearcmd = "rm %s" % (fn)
	log.info("Clean", clearcmd, 1)
	clear_proc = Popen(clearcmd, shell=True)
	clear_proc.wait()

def make_tmp_dir():
	global tmp_file
	if not os.path.exists(os.path.dirname(tmp_file)):
		try:
			os.makedirs(os.path.dirname(tmp_file))
		except OSError as exc:
			log.err("Making tmp directory", str(exc))

def run_file(out):
	if "/" not in out:
		runcmd = "./%s" % (out)
	else:
		runcmd = "%s" % (out)
	log.info("Running", runcmd, 2)
	run_proc = Popen(runcmd, shell=True)
	run_proc.wait()

def init_all_compile(tmpf, execf):
	global tmp_file; global exec_file
	tmp_file = tmpf
	tmp_exec = execf
	make_tmp_dir()
	compiler.init_cmp()

def compile_tmp(out):
	# Compile the intermediate lang
	cmd = "gcc %s -lmpfr -lgmp -lm -o %s" % (tmp_file, out)
	log.info("Compiling", cmd, 1)
	compile_proc = Popen(cmd, shell=True)
	compile_proc.wait()

def addcode(fs):
	compiler.add_code(fs)

def compile_files(sources, out):
	global tmp_file
	# loops through, compiling and saving
	for src in sources:
		addcode(open(src, "r").read())

	# we write C file
	outf = open(tmp_file, "w+")
	outf.write(compiler.get_c_file())
	outf.close()

	compile_tmp(out)

def compile_c_option(text, out):
	addcode(text)
	outf = open(tmp_file, "w+")
	outf.write(compiler.get_c_file())
	outf.close()
	compile_tmp(out)
	

