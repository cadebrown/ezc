#EZC_LIB="-I%s/include/ %s/lib/libmpfr.a %s/lib/libgmp.a"
# store data here, for simple flags, and our starter for EZC
import os

EZC_DIR=os.path.dirname(__file__)
EZC_LIB="-lmpfr -lgmp"

EZC_DOGFOOD=open(EZC_DIR + "/EZC_LIB.ezc").read()

EZC_C=open(EZC_DIR + "/EZC_LIB.c").read()