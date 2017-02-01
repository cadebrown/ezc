#!/bin/bash


# Install dependencies
if [[ "$OSTYPE" == "linux-gnu" ]]; then
	echo "Asking for sudo to install packages . . ."
	if [[ $(cat /etc/debian_version) ]]; then
		apt-get install libmpfr-dev
	elif [[ $(cat /etc/fedora-release) ]]; then
		dnf install mpfr-devel
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	echo "Asking for sudo to install packages . . ."
	echo "Need homebrew"
	brew install gcc48
	brew install mpfr 
elif [[ "$OSTYPE" == *"BSD" ]]; then
	echo "Asking for sudo to install packages . . ."
	pkg install gcc
	pkg install mpfr
	pkg install python
	pkg install bash
elif [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
	echo "cygwin wont work for dev machine"
	exit -1
else
	echo "Warning: OS not found."
	echo "Cant develop on this OS"
	exit -1
fi


