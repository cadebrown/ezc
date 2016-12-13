#!/bin/bash

rm -rf deb-package
DIRS="deb-package/DEBIAN/ deb-package/bin/ deb-package/usr/ deb-package/usr/bin/ deb-package/usr/share/ deb-package/usr/share/doc/ deb-package/usr/share/doc/ezc/ deb-package/usr/src deb-package/usr/src/ezc/ deb-package/usr/src/ezc/utils/"

for DIR in $DIRS
do
    mkdir -p $DIR
done

SRC="./ezcc.py "./*.py
UTILS=./utils/*
FL="ezcc.deb"

printf "\nSources for compiler: \n"
for SR in $SRC
do
	printf "$(basename $SR) "
done
printf "\n\nUtilities included: \n"
for UTIL in $UTILS
do
	printf "$(basename $UTIL) "
done
printf "\n"

#VERSION=$1
#if [ "$VERSION" == "" ]; then
#	echo "Default version"
#	VERSION="latest"
#fi

cp $SRC deb-package/usr/src/ezc/
cp $UTILS deb-package/usr/src/ezc/utils/

for UTIL in $UTILS
do
    O_UTIL=deb-package/usr/bin/$(basename $UTIL)
    ./ezcc.py $UTIL -o $O_UTIL
	strip $O_UTIL
done

cp ezc_bin deb-package/bin/ezc
cp ezc_bin deb-package/bin/ezcc

echo "Now copying in debian files"
cp CONTROL deb-package/DEBIAN/control
cp COPYRIGHT deb-package/usr/share/doc/ezc/copyright
gzip -n -9 -c changelog > deb-package/usr/share/doc/ezc/changelog.gz

# neccessary 
printf "\nChanging file permissions to build deb package\n"
#chmod 0755 $DIRS/*
chmod 0755 deb-package/usr/src/ezc/utils/*
chmod 0755 $DIRS
chmod 0644 deb-package/usr/share/doc/ezc/*
chmod 0644 deb-package/usr/src/ezc/*.py
chmod 0755 deb-package/usr/src/ezc/ezcc.py
chmod 0755 deb-package/usr/bin/*

printf "\nDeb file:\n"
echo $FL

printf "\n\nReport on .deb file:\n"
fakeroot dpkg-deb --build deb-package $FL && lintian $FL

rm -rf deb-package

