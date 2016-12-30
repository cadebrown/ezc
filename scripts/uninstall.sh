#!/bin/bash
cd src
SOURCES=`echo *.py */`
cd ..
UTILS=./utils/*

echo $SOURCES

EXE_INSTALL_DIR=$1
SRC_INSTALL_DIR=$2

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    EXE_INSTALL_DIR=/usr/bin/
fi
if [ "$2" == "" ] || [ "$2" == "auto" ]; then
    SRC_INSTALL_DIR=/usr/src/ezc/
fi

echo Uninstalling sources from $SRC_INSTALL_DIR

for SRC in $SOURCES
do
	rm -Rf $SRC_INSTALL_DIR"/"$SRC
done

echo Uninstalling execs from $EXE_INSTALL_DIR

for UTIL in $UTILS
do
    O_UTIL=$(basename $UTIL)
	rm $EXE_INSTALL_DIR/$O_UTIL
done

rm $EXE_INSTALL_DIR/ezc
rm $EXE_INSTALL_DIR/ezcc

echo Done removing

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"
