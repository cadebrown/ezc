#!/bin/bash
RED='\033[0;31m'
NC='\033[0m'

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

printf "\n\n${RED}Download the file here:\n\n"


curl --progress-bar --upload-file $ARCHIVE https://transfer.sh/$ARCHIVE

printf "${NC}\n\nDone\n\n"
