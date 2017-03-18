#!/bin/sh


# Install dependencies

PLATFORM=$(./scripts/platform.sh)

if [ "$PLATFORM" = "linux" ]; then
	if [ $(cat /etc/debian_version) ]; then
		apt-get install libmpfr-dev
	elif [ $(cat /etc/fedora-release) ]; then
		dnf install mpfr-devel
	fi
elif [ "$PLATFORM" == "mac" ]; then
	if [ "$(brew -h)" ]; then
		echo You have homebrew
	else
		echo "Installing homebrew"
		/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	fi
	brew install gcc48
	brew install mpfr 
elif [ "$PLATFORM" == "bsd" ]; then
	pkg install gcc
	pkg install mpfr
	pkg install python
	pkg install bash
else
	echo "Cant develop on this OS"
	exit -1
fi


