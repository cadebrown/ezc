#!/bin/sh

UTILS=./utils/*.ezc

INSTALL_DIR=./

echo Compiling utils

for UTIL in $UTILS
do
	O_UTIL=$INSTALL_DIR/$(basename $UTIL .ezc)
	./ezc $UTIL -o $O_UTIL
done

echo Done with utils
