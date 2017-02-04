#!/bin/sh

UTILS=./utils/*.ezc

INSTALL_DIR=./
EXE_DIR=./

if [ "$1" != "" ]; then
	EXE_DIR="$1"
fi


echo Compiling utils

for UTIL in $UTILS
do
	O_UTIL=$INSTALL_DIR/$(basename $UTIL .ezc)
	$EXE_DIR/ezc $UTIL -o $O_UTIL
done

echo Done with utils
