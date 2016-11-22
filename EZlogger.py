name = "EZCC"

version = "2.1.1"

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

verbosity = 0

def init(_verbose=0):
	global verbosity
	verbosity = _verbose

def base_print(col, task, st, verbose=0):
	global verbosity
	if verbose >= 0 and verbose <= verbosity:
		print "%s v%s: \n  %s:" % (col+name, WARNING+version+col, task)
		
		if isinstance(st, list):
			for s in st:
				print "    %s" % (OKGREEN + s)
		else:
			print "    %s" % (OKGREEN + st)
		print "%s" % (HEADER) 

def info(task, st, v=2):
    base_print(OKBLUE, task, st, v)

def warn(task, st, v=1):
    base_print(WARNING, task, st, v)

def err(task, error, v=0):
    base_print(FAIL, task, error, v)

