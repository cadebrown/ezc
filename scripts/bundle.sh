#!/bin/bash
# bundles tar

ARCHIVE=$1
DIR=$2
BASE=`basename $DIR`

echo "Tarring $DIR into $ARCHIVE with base $BASE"

tar cJf $ARCHIVE -C $DIR/.. $BASE

echo "Done tarring"
