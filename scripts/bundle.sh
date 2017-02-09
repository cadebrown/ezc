#!/bin/sh
# bundles tar

DIR=$1

PLATFORM=$(./scripts/platform.sh)

ARCHIVE="ezc-${PLATFORM}.tar.xz"

echo "Tarring $DIR into $ARCHIVE"
TO=$PWD
echo "PWD: $TO"

cd $DIR
tar cvzf $TO/$ARCHIVE ./
cd $TO

echo "Done tarring"

