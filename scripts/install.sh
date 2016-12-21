#!/bin/bash
cd src
SOURCES=*
cd ..

UTILS=./utils/*

echo $SOURCES

EXE_INSTALL_DIR=$1
SRC_INSTALL_DIR=$2
I_MPFR=$3

echo Operating System Type: $OSTYPE

if [ "$1" == "" ] || [ "$1" == "auto" ]; then
    EXE_INSTALL_DIR=/usr/bin/
fi
if [ "$2" == "" ] || [ "$2" == "auto" ]; then
    SRC_INSTALL_DIR=/usr/src/ezc/
fi

EZC_BIN="#!/bin/bash\\n$SRC_INSTALL_DIR/ezcc.py \"\${@}\""

if [[ "$I_MPFR" == "true" ]]; then
	echo "Installing MPFR from source"
	
	./scripts/mpfr.sh $SRC_INSTALL_DIR
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
		./scripts/mpfr.sh $SRC_INSTALL_DIR
	else
		echo "Warning: OS not found."
		echo "Building MPFR from source"
		I_MPFR="true"
		./scripts/mpfr.sh $SRC_INSTALL_DIR
	fi
fi

echo Installing sources in $SRC_INSTALL_DIR

mkdir -p $SRC_INSTALL_DIR
cp -Rf ./src/$SOURCES $SRC_INSTALL_DIR

if [[ "$I_MPFR" == "true" ]]; then
	printf "\nEZC_LIB=\"-w -I$SRC_INSTALL_DIR/include/ $SRC_INSTALL_DIR/lib/libmpfr.a $SRC_INSTALL_DIR/lib/libgmp.a\"\n" >> $SRC_INSTALL_DIR/ezdata.py
fi

echo Installing execs in $EXE_INSTALL_DIR

mkdir -p $EXE_INSTALL_DIR
for UTIL in $UTILS
do
    O_UTIL=$EXE_INSTALL_DIR/$(basename $UTIL)
    $SRC_INSTALL_DIR/ezcc.py $UTIL -o $O_UTIL -v1
	strip $O_UTIL
done

echo -e $EZC_BIN > $EXE_INSTALL_DIR/ezc
echo -e $EZC_BIN > $EXE_INSTALL_DIR/ezcc

chmod +x $EXE_INSTALL_DIR/ezc
chmod +x $EXE_INSTALL_DIR/ezcc


echo Done copying

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"
echo "Done"
