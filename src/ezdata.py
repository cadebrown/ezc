import os
import tempfile
EZC_DIR=os.path.dirname(os.path.realpath(__file__))
# python file to execute
exec(open(EZC_DIR + "/data.txt").read())
