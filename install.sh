#!/bin/sh
INSTALL_DIR=/usr/local/ezc
SOURCES="ezcc EZcompiler.py EZlogger.py"
LINK=/usr/bin/ezcc

rm $LINK

echo Installing EZC in $INSTALL_DIR

mkdir -p $INSTALL_DIR
cp -t $INSTALL_DIR $SOURCES

echo Done copying

echo Now linking to $LINK

ln -s $INSTALL_DIR/ezcc /usr/bin/ezcc