#!/bin/bash

brew install mpfr
apt install libmpfr-dev libmpfr-doc libmpfr4 libmpfr4-dbg
dnf install mpfr mpfr-devel

INSTALL_DIR=$1
LINK=$2
SOURCES="ezcc EZcompiler.py EZlogger.py libMath.py libLoops.py libBasic.py shared_data.py"

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    INSTALL_DIR=/usr/local/ezc
fi

if [[ "$2" == "" ]]; then
    LINK=/usr/bin/ezcc
fi

echo Installing EZC in $INSTALL_DIR

mkdir -p $INSTALL_DIR
cp -t $INSTALL_DIR $SOURCES

echo Done copying

if [[ "$LINK" != "none" ]]; then
    rm $LINK
    echo Now linking to $LINK
    ln -s $INSTALL_DIR/ezcc /usr/bin/ezcc
fi

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"