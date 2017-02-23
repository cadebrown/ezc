#!/bin/sh
RED='\033[31m'
NC='\033[0m'
BOLD='\033[1m'

PLATFORM=$(./scripts/platform.sh)
ARCHSTR=$(./scripts/arch.sh)

ARCHIVE=ezc-$PLATFORM-$ARCHSTR.tar.xz

echo "Uploading $ARCHIVE"

printf "\n\n${RED}${BOLD}Download the file here:\n\n"


transfer() { if [ $# -eq 0 ]; then echo "No arguments specified. Usage:\necho transfer /tmp/test.md\ncat /tmp/test.md | transfer test.md"; return 1; fi 
tmpfile=$( mktemp -t transferXXX ); if tty -s; then basefile=$(basename "$1" | sed -e 's/[^a-zA-Z0-9._-]/-/g'); curl --progress-bar --upload-file "$1" "https://transfer.sh/$basefile" >> $tmpfile; else curl --progress-bar --upload-file "-" "https://transfer.sh/$1" >> $tmpfile ; fi; cat $tmpfile; rm -f $tmpfile; }

transfer $ARCHIVE

printf "${NC}\n\nDone\n\n"
