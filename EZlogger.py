name = "EZCC"

version = "2.2.5"

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

verbosity = 0

def print_single(st, offset, c):
	print "%s%s> %s" % ("  "*(offset+1), c[0], c[1] + st)

def base_print(col, task, st, verbose=0):
	global verbosity
	if verbose >= 0 and int(verbose) <= int(verbosity):
		if isinstance(st, list):
			for s in st:
				print_single(s, st.index(s), col)
		else:
			print_single(st, 0, col)
		print "%s" % (HEADER) 

def info(task, st, v=2):
    base_print((OKGREEN, OKBLUE), task, st, v)

def warn(task, st, v=1):
    base_print((WARNING, OKBLUE), task, st, v)

def err(task, error, v=0):
    base_print((FAIL, WARNING), task, error, v)

def init(_verbose=0):
	global verbosity
	verbosity = _verbose
	if int(verbosity) >= 0:
		print "%s %sv%s: \n%s" % (HEADER+name, OKGREEN, WARNING+version, ENDC)
def end():
	print "%s\n" % (ENDC)