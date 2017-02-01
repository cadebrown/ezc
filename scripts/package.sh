#!/bin/bash

# Install dependencies
if [[ "$OSTYPE" == "linux-gnu" ]]; then
	if [[ $(cat /etc/debian_version) ]]; then
		bash  ./scripts/deb.sh "${@}"
	elif [[ $(cat /etc/fedora-release) ]]; then
		echo "Fedora/RPM not supported"
		# todo add .rpm package
	fi
elif [[ "$OSTYPE" == *"BSD" ]]; then
	echo "BSD not supported"
elif [[ "$OSTYPE" == "darwin"* ]]; then
	# todo add .app
	echo "macOS not supported"
else
	exit -1
fi
