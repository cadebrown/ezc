#!/bin/bash
# bundles tar

DIR=$1
BASE=`basename $DIR`

if [[ "$OSTYPE" == "linux-gnu" ]]; then
	TYPE="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
	TYPE="mac"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
	TYPE="bsd"
elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
	TYPE="cygwin"
else
	TYPE="generic"
fi

ARCHIVE="$TYPE.tar.xz"



echo "Tarring $DIR into $ARCHIVE with base $BASE"

tar cJf $ARCHIVE -C $DIR/.. $BASE

echo "Done tarring"
