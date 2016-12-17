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

def make_tmp_dir(file="__tmp"):
	global tmp_file; global exec_file
	if file == "__tmp":
		file = tmp_file
	elif file == "__exec":
		file = exec_file
	if not os.path.exists(os.path.dirname(file)):
		try:
			os.makedirs(os.path.dirname(file))
		except OSError as exc:
			log.err("Making tmp directory", str(exc))

def run_file(out, args=[]):
	make_tmp_dir("__exec")
	if "/" not in out:
		runcmd = "./%s %s" % (out, " ".join(args))
	else:
		runcmd = "%s %s" % (out, " ".join(args))
	log.info("Running", runcmd, 2)
	run_proc = Popen(runcmd, shell=True)
	run_proc.wait()

def init_all_compile(cmpflags, tmpf, execf):
	global tmp_file; global exec_file
	tmp_file = tmpf
	exec_file = execf
	make_tmp_dir()
	compiler.init_cmp(cmpflags)

def get_lib_args():
	import os
	ezcl = os.environ.get('EZC_LIB')
	if ezcl:
		#return "-I%s/include/ -L%s/lib/ -l:libmpfr.a -l:libgmp.a" % (ezcl, ezcl)
		return "-I%s/include/ %s/lib/libmpfr.a %s/lib/libgmp.a" % (ezcl, ezcl, ezcl)
	return "-lmpfr -lgmp"

def compile_tmp(out):
	# Compile the intermediate lang
	cmd = "gcc %s %s -lm -o %s" % (tmp_file, get_lib_args(), out)
	log.warn("Compiling", cmd, 1)
	compile_proc = Popen(cmd, shell=True)
	compile_proc.wait()

def addcode(fs):
	try:
		compiler.add_code(fs)
	except Exception as e:
		log.err(e, 0)

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
	

