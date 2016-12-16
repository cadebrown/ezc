#!/bin/bash

EZC_BIN="#\!/bin/bash\\n$SRC_INSTALL_DIR/ezcc.py \"\${@}\""

SOURCES=*.py
UTILS=./utils/*

EXE_INSTALL_DIR=$1
SRC_INSTALL_DIR=$2

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    EXE_INSTALL_DIR=/usr/bin/
fi
if [ "$2" == "" ] || [ "$2" == "auto" ]; then
    SRC_INSTALL_DIR=/usr/src/ezc/
fi

# Install dependencies
if [[ "$OSTYPE" == "linux-gnu" ]]; then
	apt install libmpfr-dev libmpfr-doc libmpfr4 libmpfr4-dbg
	dnf install mpfr mpfr-devel
elif [[ "$OSTYPE" == "darwin"* ]]; then
	brew install gcc48
	brew install mpfr 
elif [[ "$OSTYPE" == "freebsd"* ]]; then
	pkg install gcc
	pkg install mpfr
elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
	echo "Error! Cygwin not supported"
	exit 1
else
	echo "Warning: OS not found."
	exit 1
fi

echo Installing execs in $EXE_INSTALL_DIR

mkdir -p $EXE_INSTALL_DIR
for UTIL in $UTILS
do
    O_UTIL=$EXE_INSTALL_DIR/$(basename $UTIL)
    ./ezcc.py $UTIL -o $O_UTIL -v 0 -rem
	strip $O_UTIL
done


printf $EZC_BIN > $EXE_INSTALL_DIR/ezc
printf $EZC_BIN > $EXE_INSTALL_DIR/ezcc

chmod +x $EXE_INSTALL_DIR/ezc
chmod +x $EXE_INSTALL_DIR/ezcc

echo Installing sources in $EXE_INSTALL_DIR

mkdir -p $SRC_INSTALL_DIR
cp $SOURCES $SRC_INSTALL_DIR

echo Installing utils in $UTIL_INSTALL_DIR

echo Done copying

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"