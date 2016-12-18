#!/bin/bash

rm -rf deb-package

FIN_EXE_PATH=/usr/bin/
FIN_SRC_PATH=/usr/src/

EXE_PATH=deb-package/$FIN_EXE_PATH
SRC_PATH=deb-package/$FIN_SRC_PATH
DOC_PATH=deb-package/usr/share/doc/ezc/

EZC_BIN="#!/bin/bash\\n$FIN_SRC_PATH/ezcc.py \"\${@}\""
DIRS="deb-package/DEBIAN/ $DOC_PATH"

for DIR in $DIRS
do
	mkdir -p $DIR
done
FL="ezcc.deb"

# Do generic install
./install-all.sh $EXE_PATH $SRC_PATH

rm -rf $SRC_PATH/*.pyc

# Replace exe files (bc this is in whole filesystem)
echo -e $EZC_BIN > $EXE_PATH/ezc
echo -e $EZC_BIN > $EXE_PATH/ezcc

echo "Now copying in debian files"
cp CONTROL deb-package/DEBIAN/control
cp COPYRIGHT $DOC_PATH/copyright
gzip -n -9 -c changelog > $DOC_PATH/changelog.gz

# For bundling
chmod -R 0755 deb-package/usr/
chmod 0755 $EXE_PATH/*
chmod 0644 $SRC_PATH/*.py
chmod 0755 $SRC_PATH/ezcc.py
chmod 0644 $DOC_PATH/*

printf "\nDeb file:\n"
echo $FL

printf "\n\nReport on .deb file:\n"
fakeroot dpkg-deb --build deb-package $FL && lintian $FL

#rm -rf deb-package

