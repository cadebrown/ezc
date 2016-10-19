name = "EZCC"

version = "0.0.1"

info = """%s v%s
Chemical Development 2016

chemicaldevelopment.us
""" % (name, version)


def init():
    print info

def err(task, st):
    print "%s: \n\t%s: \n\t\t" % (name, task, st)