name = "EZCC"

version = "0.1.0"

info = """%s v%s
Chemical Development 2016

chemicaldevelopment.us
""" % (name, version)


def init():
    print info

def err(task, err):
    print "%s: \n\t%s" % (task, err)