#!/bin/bash
#SOURCES=`echo *.py */`
pushd src
SOURCES=`find ./ -type f \( -iname \*.c -o -iname \*.py  -o -iname \*.ezc \)`
popd

UTILS=./utils/*

#echo $SOURCES

for SRC in $SOURCES; do
	echo $SRC
done

INSTALL_DIR=$1
SRC_DIR=$2
I_MPFR=$3
KEEP_MPFR=$4
ARCHIVE="ezc.tar.xz"

echo Operating System Type: $OSTYPE

PA=`python -c "import os.path; print os.path.relpath('$SRC_DIR', '$INSTALL_DIR')"`
EZC_BIN="#!/usr/bin/python \nimport os; import sys; sys.path.append(os.path.dirname(os.path.abspath(__file__))+\"/$PA\"); import ezcc; ezcc.main()"
echo $EZC_BIN

if [ "$KEEP_MPFR" != "true" ]; then
	if [[ "$I_MPFR" == "true" ]]; then
		echo "Going to build MPFR"
	else
		# Install dependencies
		if [[ "$OSTYPE" == "linux-gnu" ]]; then
			echo "Asking for sudo to install packages . . ."
			if [[ $(cat /etc/debian_version) ]]; then
				sudo apt-get install libmpfr-dev
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
		./scripts/req.sh $SRC_DIR
		pushd $SRC_DIR
		rm -Rf share/
		popd
	fi
fi

echo Installing sources in $SRC_DIR

for SRC in $SOURCES
do
	parentname="$(basename "$(dirname "$SRC")")"
	mkdir -p $SRC_DIR/$parentname
	cp ./src/$SRC $SRC_DIR/$SRC
done

echo -e $EZC_BIN > $INSTALL_DIR/ezc
echo -e $EZC_BIN > $INSTALL_DIR/ezcc

chmod +x $INSTALL_DIR/ezc
chmod +x $INSTALL_DIR/ezcc

if [ "$I_MPFR" == "true" ] || [ "$KEEP_MPFR" == "true" ]; then
	printf "\nEZC_LIB=\"-w -I%%s/include/ %%s/lib/libmpfr.a %%s/lib/libgmp.a\" %% (EZC_DIR, EZC_DIR, EZC_DIR)\n" >> $SRC_DIR/ezdata.py
fi

echo Installing execs in $INSTALL_DIR

for UTIL in $UTILS
do
    O_UTIL=$INSTALL_DIR/$(basename $UTIL)
	echo $UTIL
    $INSTALL_DIR/ezc $UTIL -o $O_UTIL -v1
	#strip $O_UTIL
done


echo Done copying

echo Removing uneeded dirs

pushd $INSTALL_DIR
rm -Rf share/
popd

pushd $SRC_DIR
DTFMT=`date +'%Y-%m-%d %H:%M:%S %z'`
printf "\ndate=\"$DTFMT\"" >> ezlogging/log.py
popd

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"

echo "Done"


