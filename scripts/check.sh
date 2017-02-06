#!/bin/bash
# checks all examples

EXAMPLES=./examples/*/

echo "Checking $EXAMPLES"

for EX in $EXAMPLES
do
	pushd $EX
		make check EZCC=$1
		rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
	popd
done


