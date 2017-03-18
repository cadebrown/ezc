#!/bin/sh
# bundles tar

DIR=$1

PLATFORM=$(./scripts/platform.sh)
ARCHSTR=$(uname -m)
VERSION=$(cat VERSION)

ARCHIVE="ezc-${PLATFORM}-${ARCHSTR}.tar.xz"

echo "Creating $ARCHIVE"
TO=$PWD

cd $DIR
tar cvzf $TO/$ARCHIVE ./
cd $TO

