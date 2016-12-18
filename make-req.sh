#!/bin/bash
GMP_TAR=https://gmplib.org/download/gmp/gmp-6.1.1.tar.xz
MPFR_TAR=http://www.mpfr.org/mpfr-current/mpfr-3.1.5.tar.xz
#$1
BUILD_DIR=$1

mkdir -p $BUILD_DIR
cd $BUILD_DIR

curl $GMP_TAR > gmp.tar.xz
tar xfv gmp.tar.xz
cd gmp*
./configure --disable-shared --enable-static --prefix=$BUILD_DIR
make install
cd ..

curl $MPFR_TAR > mpfr.tar.xz
tar xvf mpfr.tar.xz
cd mpfr*
./configure --disable-shared --enable-static --prefix=$BUILD_DIR --with-gmp=$BUILD_DIR
make install
cd ..

#export EZC_LIB=$BUILD_DIR

rm gmp* mpfr* -Rf
