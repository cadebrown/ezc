#!/bin/bash
pyinstaller --onefile ezcc.py EZcompiler.py EZlogger.py libBasic.py libLoops.py libMath.py shared_data.py

if [ "$USER" != "root" ]; then
	echo "Need to be root!"
	exit
fi

VERSION=$1
if [ "$VERSION" == "" ]; then
	echo "Default version"
	VERSION="0.0.1"
fi

cp dist/ezcc deb-package/usr/bin/ezcc
gzip -9 -c changelog > deb-package/usr/share/doc/ezcc/changelog.gz

echo "Copied in, now making deb file"

FL="ezcc_"$VERSION"_amd64.deb"

echo $FL

dpkg-deb --build deb-package $FL && lintian $FL



