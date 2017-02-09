#!/bin/sh

UNAMESTR=$(uname -s)

case "$UNAMESTR" in
	"Linux") PLATFORM="linux";;
	"Darwin") PLATFORM="mac";;
	*"BSD") PLATFORM="bsd";;
	*) PLATFORM="custom";;
	#*) PLATFORM="build";;
esac

echo ${PLATFORM}
