#!/bin/bash


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
if [[ "$1" == "" ]]; then
	ARCHIVE=./ezc-*.tar.xz
else
	ARCHIVE="$1"
fi

ARCHIVE=`echo $ARCHIVE`

echo "Uploading archive $ARCHIVE"

echo "Download the file here:"

transfer() { if [ $# -eq 0 ]; then echo "No arguments specified. Usage:\necho transfer /tmp/test.md\ncat /tmp/test.md | transfer test.md"; return 1; fi 
tmpfile=$( mktemp -t transferXXX ); if tty -s; then basefile=$(basename "$1" | sed -e 's/[^a-zA-Z0-9._-]/-/g'); curl --progress-bar --upload-file "$1" "https://transfer.sh/$basefile" >> $tmpfile; else curl --progress-bar --upload-file "-" "https://transfer.sh/$1" >> $tmpfile ; fi; cat $tmpfile; rm -f $tmpfile; }

transfer $ARCHIVE
