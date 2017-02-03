#!/bin/sh

UTILS=./utils/*.ezc

INSTALL_DIR=$1

echo Installing util sources in $INSTALL_DIR
mkdir -p $INSTALL_DIR/utils

for UTIL in $UTILS
do
	O_UTIL=$INSTALL_DIR/$UTIL
	cp $UTIL $O_UTIL
done
