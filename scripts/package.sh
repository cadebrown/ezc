#!/bin/sh

PLATFORM=$(./scripts/platform.sh)

if [ "$PLATFORM" = "linux" ]; then
	if [ $(cat /etc/debian_version) ]; then
		bash  ./scripts/deb.sh "${@}"
	elif [ $(cat /etc/fedora-release) ]; then
		echo "Fedora/RPM not supported"
	fi
elif [ "$PLATFORM" == "mac" ]; then
	echo "macOS not supported"
elif [ "$PLATFORM" == "bsd" ]; then
	echo "BSD not supported"
else
	echo "Cant find the OS"
	exit -1
fi


