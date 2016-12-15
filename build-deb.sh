#!/bin/bash

rm -rf deb-package

EXE_PATH=deb-package/usr/bin/
SRC_PATH=deb-package/usr/src/
DOC_PATH=deb-package/usr/share/doc/ezc/

EZC_BIN="#\!/bin/bash\\n$SRC_INSTALL_DIR/ezcc.py \"\${@}\""

DIRS="deb-package/DEBIAN/ $DOC_PATH"

for DIR in $DIRS
do
	mkdir -p $DIR
done
FL="ezcc.deb"

# Do generic install
./install-all.sh $EXE_PATH $SRC_PATH

# Replace exe files (bc linking)
echo $EZC_BIN > $EXE_PATH/ezc
echo $EZC_BIN > $EXE_PATH/ezcc

echo "Now copying in debian files"
cp CONTROL deb-package/DEBIAN/control
cp COPYRIGHT $DOC_PATH/copyright
gzip -n -9 -c changelog > $DOC_PATH/changelog.gz

# neccessary 
#printf "\nChanging file permissions to build deb package\n"
#chmod 0755 $DIRS/*
#chmod 0755 deb-package/usr/src/ezc/utils/*
#chmod 0755 $DIRS
#chmod 0644 deb-package/usr/share/doc/ezc/*
#chmod 0644 deb-package/usr/src/ezc/*.py
#chmod 0755 deb-package/usr/src/ezc/ezcc.py
#chmod 0755 deb-package/usr/bin/*
chmod -R 0755 deb-package/usr/
chmod 0755 $EXE_PATH/*
chmod 0644 $SRC_PATH/*.py
chmod 0644 $DOC_PATH/*

printf "\nDeb file:\n"
echo $FL

printf "\n\nReport on .deb file:\n"
fakeroot dpkg-deb --build deb-package $FL && lintian $FL

rm -rf deb-package

