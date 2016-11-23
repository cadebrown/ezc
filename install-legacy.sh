#!/bin/bash

INSTALL_DIR=$1
LINK=$2
SOURCES=*.py

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    INSTALL_DIR=/usr/local/ezc
fi

if [[ "$2" == "" ]]; then
    LINK=/usr/bin/ezcc
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
	apt install libmpfr-dev libmpfr-doc libmpfr4 libmpfr4-dbg
	dnf install mpfr mpfr-devel
elif [[ "$OSTYPE" == "darwin"* ]]; then
	brew install mpfr 
	brew install gcc48
elif [[ "$OSTYPE" == "freebsd"* ]]; then
	echo "Error! freebsd not supported yet"
	exit 1
elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
	echo "Error! Cygwin not supported"
	exit 1
else
	echo "Warning: OS not found."
	exit 1
fi

echo Installing EZC in $INSTALL_DIR

mkdir -p $INSTALL_DIR
cp -t $INSTALL_DIR $SOURCES

echo Done copying

if [[ "$LINK" != "none" ]]; then
    rm $LINK
    echo Now linking to $LINK
    ln -s $INSTALL_DIR/ezcc $LINK
fi

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"