#!/bin/bash


mkdir -p build/ && cd build/

GMP_DIR=$PWD/../out MPFR_DIR=$PWD/../out cmake .. -DCMAKE_INSTALL_PREFIX=$PWD/../out -DCMAKE_PREFIX_PATH=$PWD/../out

make -j4

make install

