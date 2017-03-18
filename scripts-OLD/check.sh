#!/bin/sh
# checks all examples

EXAMPLES=./examples/*/

echo "Checking $EXAMPLES"

FROM=$PWD

for EX in $EXAMPLES
do
	cd $EX
		make check EZCC=$1
		rc=$?; if [ $rc != 0 ]; then exit $rc; fi
	cd $FROM
done


