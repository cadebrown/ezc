#!/bin/sh

DIR=$1

COPYCC=$2
COPYPY=$3
COPYPYDIR=$4

# copy a c compiler
if [ "$COPYCC" != "" ]; then
	if [ test -e $COPYCC ]; then

	else
		$COPYCC=$(which cc)
	fi
	cp $COPYCC $DIR/src/cc
	printf "\ncc = '{0}/cc'.format(EZC_DIR)\n" >> $DIR/src/ezdata.py
fi

if [ "$COPYPYDIR" = "" ]; then
	COPYPYDIR=/usr/lib/python2.7
fi

# copy a c compiler
if [ "$COPYPY" != "" ]; then
	TOPYDIR=$DIR/src/lib/
	cp ./scripts/python-distrib.sh $DIR/src/python
	cp $COPYPY $DIR/src/python-bin
	#cp -Rf /usr/lib/python2.7 $DIR/src/lib/python2.7
	mkdir -p $TOPYDIR
	echo $COPYPYDIR
	echo $TOPYDIR
	cp -Rf $COPYPYDIR $TOPYDIR

	find $TOPYDIR -type f -name "*.pyc" -delete
	find $TOPYDIR -type f -name "*.pyd" -delete
	find $TOPYDIR -type f -name "*.pyo" -delete

	rm -Rf $TOPYDIR/*/dist-packages
	rm -Rf $TOPYDIR/*/lib-dynload
	rm -Rf $TOPYDIR/*/lib-tk
	rm -Rf $TOPYDIR/*/config-*

	cp ./scripts/python-distrib.sh $DIR/ezc
	cp ./scripts/python-distrib.sh $DIR/ezcc
fi


