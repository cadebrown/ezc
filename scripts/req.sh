#!/bin/sh

GMP_TAR="https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz"
MPFR_TAR="http://www.mpfr.org/mpfr-current/mpfr-3.1.5.tar.xz"

BUILD_DIR=$1
TO=$PWD

# get abs path
cd "$BUILD_DIR"
BUILD_DIR="$PWD"
cd $TO

echo Installing GMP and MPFR in $BUILD_DIR

mkdir -p $BUILD_DIR
cd $BUILD_DIR

curl $GMP_TAR > gmp.tar.xz
tar xfv gmp.tar.xz >/dev/null 2>&1
cd gmp-*
./configure --disable-shared --enable-fat --enable-static --prefix=$BUILD_DIR >/dev/null 2>&1
make install >/dev/null 2>&1
cd ..

curl $MPFR_TAR > mpfr.tar.xz
tar xvf mpfr.tar.xz  >/dev/null 2>&1
cd mpfr-*
./configure --disable-shared --enable-fat --enable-static --prefix=$BUILD_DIR --with-gmp=$BUILD_DIR
make install
cd ..

rm -Rf gmp* mpfr* share/ lib/*.la

strip --strip-unneeded lib/*.a
