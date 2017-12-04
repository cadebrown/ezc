#!/bin/bash


mkdir -p build/ && cd build/

curl -L http://www.mpfr.org/mpfr-current/mpfr-3.1.6.tar.xz > mpfr-3.1.6.tar.xz

tar xfv mpfr-3.1.6.tar.xz

cd mpfr-3.1.6

./configure --enable-shared --disable-static --with-gmp=$PWD/../../out --prefix=$PWD/../../out

make -j4

make install

