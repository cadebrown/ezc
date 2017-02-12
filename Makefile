###
# GPLv3
#
# To develop, run `sudo make dev`
#
# For a full local install and upload, run `make build build-utils bundle upload`
#
# To build from source and build dependencies, run `make build`.
# This will install in ./ezc/
#
# For a global install, run `sudo make global`
#
# To use, set DIR= whatever you want to install binaries. Normally, SRC= should not be used, as it should be $DIR/src/
# However, installing globally, use `sudo make DIR=/usr/bin/ SRC=/usr/src/ezc/`
#
# I've implemented some basic linking, so this will still work.
# You can also set REQ=false to use a package manager (like brew.sh, apt, dnf, pkg, etc.)
#
# To make a package (.deb, .rpm, etc), run `make package`
# to build and upload the package, run `make package upload-package`
#
# Don't worry if you don't have one. If it can't find one, it builds MPFR from source anyway.
# For info, please email <info@chemicaldevelopment.us>
# If you already have MPFR installed, run make KEEP_REQ=true
###

# These three are the only ones that should be changed!
# directory to install to
DIR=${PWD}/ezc/
# source directory to store sources into
SRC=${DIR}/src/
# whether or not to use a built version of requirements (GMP, MPFR, etc)
REQ=true
# if requirements are already built, do we just keep the existing ones?
KEEP_REQ=false

# Global defaults
GLOBAL_DIR=/usr/bin/
GLOBAL_SRC=/usr/src/ezc/

EZCC=${DIR}/ezc

CC=/usr/bin/cc
PY=/usr/bin/python
PYDIR=/usr/lib/python2.7 

# what file to test
EX_FILE=example.ezc
# what flags to run
EZCC_FLAGS=-run -v5

# what scripts directory
SCRIPTS=./scripts/

SH=/bin/sh

# a bunch of useful macros

# builds in directory (install)
build: install
	echo Built

# install with set values, uninstalling first
install:
	-${SH} ${SCRIPTS}/install.sh ${DIR} ${SRC} ${REQ} ${KEEP_REQ}

clean:
	rm -rf ./ezc/

# just update, don't rebuild mpfr
update:
	-${SH} ${SCRIPTS}/install.sh ${DIR} ${SRC} false true

copyutils:
	-${SH} ${SCRIPTS}/copyutils.sh ${DIR}

# uninstall from DIR and SRC
uninstall:
	-${SH} ${SCRIPT}/uninstall.sh ${DIR} ${SRC}

# use this makefile
global:
	sudo $(MAKE) DIR=${GLOBAL_DIR} SRC=${GLOBAL_SRC} install

# check installation
check:
	${SH} ${SCRIPTS}/check.sh ${EZCC}

# bundles software
distrib:
	${SH} ${SCRIPTS}/distrib.sh ${DIR} ${CC} ${PY} ${PYDIR}

# makes a REQ, and puts it in SRC (so the compiler can get it later)
req:
	-${SH} ${SCRIPTS}/req.sh ${SRC}

# bundles into a tar. It figures out the name of the archive
bundle:
	-${SH} ${SCRIPTS}/bundle.sh ${DIR}

# makes a .deb, .rpm, .app, .pkg etc
package:
	-${SH} ${SCRIPTS}/package.sh

# uploads package
upload-package:
	-${SH} ${SCRIPTS}/uploadpackage.sh

# publishes Visual Studio Code Extension (requires GPG key that only I have)
vsce:
	-${SH} ${SCRIPTS}/vsce.sh

# uploads and prints a link to the file to download
upload:
	-${SH} ${SCRIPTS}/upload.sh
