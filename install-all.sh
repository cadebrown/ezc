#!/bin/bash

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

EZC_BIN="#!/bin/bash\\n$SRC_INSTALL_DIR/ezcc.py \"\${@}\""

if [[ "$3" == "true" ]]; then
	echo "Installing MPFR from source"
	./make-req.sh $EXE_INSTALL_DIR
else
	# Install dependencies
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		if [[ $(cat /etc/debian_version) ]]; then
			apt install libmpfr-dev
		elif [[ $(cat /etc/fedora-release) ]]; then
			dnf install mpfr-devel
		fi
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		brew install gcc48
		brew install mpfr 
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		pkg install gcc
		pkg install mpfr
	elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
		echo "cygwin may work . . ."
		echo "building mpfr from source:"
		./make-req.sh $EXE_INSTALL_DIR
	else
		echo "Warning: OS not found."
		echo "Building MPFR from source"
		./make-req.sh $EXE_INSTALL_DIR
	fi
fi

echo Installing execs in $EXE_INSTALL_DIR

mkdir -p $EXE_INSTALL_DIR
for UTIL in $UTILS
do
    O_UTIL=$EXE_INSTALL_DIR/$(basename $UTIL)
    ./ezcc.py $UTIL -o $O_UTIL -v 0 -rem
	strip $O_UTIL
done


echo -e $EZC_BIN > $EXE_INSTALL_DIR/ezc
echo -e $EZC_BIN > $EXE_INSTALL_DIR/ezcc

chmod +x $EXE_INSTALL_DIR/ezc
chmod +x $EXE_INSTALL_DIR/ezcc

echo Installing sources in $EXE_INSTALL_DIR

mkdir -p $SRC_INSTALL_DIR
cp $SOURCES $SRC_INSTALL_DIR

echo Installing utils in $UTIL_INSTALL_DIR

echo Done copying

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"
