#!/bin/bash
pushd src
SOURCES=`echo *.py */`
popd

UTILS=./utils/*

echo $SOURCES

INSTALL_DIR=$1
SRC_DIR=$2
I_MPFR=$3
ARCHIVE="ezc.tar.xz"

echo Operating System Type: $OSTYPE

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    INSTALL_DIR=~/ezc/
fi

if [ "$2" == "" ] || [ "$2" == "auto" ]; then
    SRC_DIR=~/ezc/src/
fi

PA=`python -c "import os.path; print os.path.relpath('$SRC_DIR', '$INSTALL_DIR')"`

EZC_BIN="#!/usr/bin/python \nimport os; import sys; sys.path.append(os.getcwd()+\"/$PA\"); import ezcc; ezcc.main()"
echo $EZC_BIN

if [[ "$I_MPFR" == "true" ]]; then
	echo "Going to build MPFR"
elif [[ "$I_MPFR" == "false" ]]; then
echo "not installing anything, through source or package manager"
else
	# Install dependencies
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		echo "Asking for sudo to install packages . . ."
		if [[ $(cat /etc/debian_version) ]]; then
			sudo apt install libmpfr-dev
		elif [[ $(cat /etc/fedora-release) ]]; then
			sudo dnf install mpfr-devel
		fi
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "Asking for sudo to install packages . . ."
		sudo brew install gcc48
		sudo brew install mpfr 
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		echo "Asking for sudo to install packages . . ."
		sudo pkg install gcc
		sudo pkg install mpfr
	elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
		echo "cygwin may work . . ."
		echo "building mpfr from source:"
		I_MPFR="true"
	else
		echo "Warning: OS not found."
		echo "Building MPFR from source"
		I_MPFR="true"
	fi
fi


mkdir -p $SRC_DIR

if [ "$I_MPFR" == "true" ]; then
	echo "Installing MPFR from source"
	./scripts/mpfr.sh $SRC_DIR
	pushd $SRC_DIR
	rm -Rf share/
	popd
fi

echo Installing sources in $SRC_DIR

for SRC in $SOURCES
do
	cp -Rf ./src/$SRC $SRC_DIR/$SRC
done

echo -e $EZC_BIN > $INSTALL_DIR/ezc
echo -e $EZC_BIN > $INSTALL_DIR/ezcc

chmod +x $INSTALL_DIR/ezc
chmod +x $INSTALL_DIR/ezcc

if [[ "$I_MPFR" == "true" ]]; then
	printf "\nEZC_LIB=\"-w -I%%s/include/ %%s/lib/libmpfr.a %%s/lib/libgmp.a\" %% (EZC_DIR, EZC_DIR, EZC_DIR)\n" >> $SRC_DIR/ezdata.py
fi

echo Installing execs in $INSTALL_DIR

for UTIL in $UTILS
do
    O_UTIL=$INSTALL_DIR/$(basename $UTIL)
	echo $UTIL
    $INSTALL_DIR/ezc $UTIL -o $O_UTIL -v1
	strip $O_UTIL
done


echo Done copying

echo Removing uneeded dirs

pushd $INSTALL_DIR
rm -Rf share/
popd

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"

echo "Creating"

tar cJf $ARCHIVE $INSTALL_DIR

echo "Done"


