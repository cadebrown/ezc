#!/bin/sh

# todo: add copy python

DIR=$1

COPYCC=$2

# copy a c compiler
if [ "$COPYCC" != "" ]; then
	cp $COPYCC $DIR/src/cc
	printf "cc = \"{0}/cc\".format(EZC_DIR)" >> $DIR/src/ezdata.py
fi
