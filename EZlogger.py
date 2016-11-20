name = "EZCC"

version = "2.1.0"

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

def init():
    base_print(OKBLUE, "EZCC", "")

def base_print(col, task, st):
    print "%s v%s: \n  %s:" % (col+name, WARNING+version+col, task)
    
    if isinstance(st, list):
        for s in st:
            print "    %s" % (OKGREEN + s)
    else:
        print "    %s" % (OKGREEN + st)
    print "%s" % (HEADER) 

def info(task, st):
    base_print(OKBLUE, task, st)

def warn(task, st):
    base_print(WARNING, task, st)

def err(task, error):
    base_print(FAIL, task, error)

