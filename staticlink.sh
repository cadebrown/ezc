#!/bin/bash
# this statically links an object

TMP="/tmp/libezctmp.so"

LGMP="libezc.gmp.so"
gcc -shared -l:libgmp.a -l:libmpfr.a -l:$PWD/ezc/ezc.gmp/$LGMP -o $TMP
cp $TMP $LGMP

