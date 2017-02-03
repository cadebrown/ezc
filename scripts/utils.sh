#!/bin/sh

UTILS=./utils/*.ezc

INSTALL_DIR=$1

echo Installing execs in $INSTALL_DIR

for UTIL in $UTILS
do
	O_UTIL=$INSTALL_DIR/$(basename $UTIL .ezc)
	#echo $UTIL
	$INSTALL_DIR/ezc $UTIL -o $O_UTIL -v1 -ccargs " -O1"
	strip --strip-unneeded $O_UTIL
done

echo Done with utils
