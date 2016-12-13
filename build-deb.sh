#!/bin/bash

SRC="./ezcc.py "./*.py
UTIL=./util/*
echo $SRC

rm dist/ezcc
#pyinstaller --onefile $SRC

#if [ "$USER" != "root" ]; then
#	echo "Need to be root!"
#	exit
#fi

VERSION=$1
if [ "$VERSION" == "" ]; then
	echo "Default version"
	VERSION="latest"
fi
mkdir -p deb-package/usr/src/ezc/
#cp dist/ezcc deb-package/usr/bin/ezcc
cp $SRC deb-package/usr/src/ezc/
cp $UTIL deb-package/usr/bin/
gzip -n -9 -c changelog > deb-package/usr/share/doc/ezcc/changelog.gz

echo "Copied in, now making deb file"

FL="ezcc_"$VERSION"_amd64.deb"

echo $FL

chmod 0755 deb-package/bin/
chmod 0755 deb-package/usr/
chmod 0755 deb-package/usr/bin/
chmod 0755 deb-package/usr/share/
chmod 0755 deb-package/usr/share/doc/
chmod 0755 deb-package/usr/share/doc/ezcc
#chown root:root deb-package/usr/share/doc/ezcc
chmod 0644 deb-package/usr/share/doc/ezcc/copyright
chmod 0644 deb-package/usr/share/doc/ezcc/changelog.gz

chmod 0755 deb-package/bin/*
chmod 0755 deb-package/usr/bin/*


fakeroot dpkg-deb --build deb-package $FL && lintian $FL



