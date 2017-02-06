#EZC_LIB="-I%s/include/ %s/lib/libmpfr.a %s/lib/libgmp.a"
# store data here, for simple flags, and our starter for EZC
import os

EZC_DIR=os.path.dirname(__file__)

EZC_LIB="-lmpfr -lgmp -L{0}/lib/ -I{0}/include/".format(EZC_DIR)

cc = "cc"
tmp = "`mktemp /tmp/XXXXX`"

EZC_DOGFOOD=open(EZC_DIR + "/EZC_LIB.ezc").read()
EZC_C=open(EZC_DIR + "/EZC_LIB.c").read()

EZC_STATIC_LIB="{0}/lib/EZC.o".format(EZC_DIR)
EZC_STATIC_LIB_HASH="{0}/lib/EZC.o.hash".format(EZC_DIR)

### ADDED BY BUNDLER

