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

printf "\n\nDownload the file here:\n\n"


curl --upload-file $ARCHIVE https://transfer.sh/$ARCHIVE

printf "\n\nDone\n\n"
