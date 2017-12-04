#!/bin/bash


mkdir -p build/ && cd build/

curl -L https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz > gmp-6.1.2.tar.xz

tar xfv gmp-6.1.2.tar.xz

cd gmp-6.1.2

./configure --enable-shared --disable-static --prefix=$PWD/../../out

make -j4

make install

