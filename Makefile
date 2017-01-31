###
# GPLv3
#
# To develop, run `sudo make dev`
#
# For a full local install and upload, run `make build bundle upload`
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
DIR=./ezc/
# source directory to store sources into
SRC=${DIR}/src/
# whether or not to use a built version of requirements (GMP, MPFR, etc)
REQ=true
# if requirements are already built, do we just keep the existing ones?
KEEP_REQ=false

# Global defaults
GLOBAL_DIR=/usr/bin/
GLOBAL_SRC=/usr/src/ezc/

# what file to test
EX_FILE=example.ezc
# what flags to run
EZCC_FLAGS=-run -v5

# what scripts directory
SCRIPTS=./scripts/

# a bunch of useful macros
INSTALL_SCRIPT=${SCRIPTS}/install.sh
UNINSTALL_SCRIPT=${SCRIPTS}/uninstall.sh
BUNDLE_SCRIPT=${SCRIPTS}/bundle.sh
UPLOAD_SCRIPT=${SCRIPTS}/upload.sh
PACKAGE_SCRIPT=${SCRIPTS}/package.sh
UPLOAD_PACKAGE_SCRIPT=${SCRIPTS}/uploadpackage.sh
VSCE_SCRIPT=${SCRIPTS}/vsce.sh
DOCS_SCRIPT=${SCRIPTS}/docs.sh
REQ_SCRIPT=${SCRIPTS}/req.sh


# builds in directory (install)
build: install
	echo Built

# install with set values, uninstalling first
install: uninstall
	-${INSTALL_SCRIPT} ${DIR} ${SRC} ${REQ} ${KEEP_REQ}

# just update, don't rebuild mpfr
update:
	-${INSTALL_SCRIPT} ${DIR} ${SRC} false true

# uninstall from DIR and SRC
uninstall:
	-${UNINSTALL_SCRIPT} ${DIR} ${SRC}

# use this makefile
global:
	sudo $(MAKE) DIR=${GLOBAL_DIR} SRC=${GLOBAL_SRC} install

# check installation
check:
	${DIR}/ezcc ${EX_FILE} ${EZCC_FLAGS}

# makes a REQ, and puts it in SRC (so the compiler can get it later)
req:
	-${REQ_SCRIPT} ${SRC}

# bundles into a tar. It figures out the name of the archive
bundle:
	-${BUNDLE_SCRIPT} ${DIR}

# makes a .deb, .rpm, .app, .pkg etc
package:
	-${PACKAGE_SCRIPT}

# uploads package
upload-package:
	-${UPLOAD_PACKAGE_SCRIPT}

# publishes Visual Studio Code Extension (requires GPG key that only I have)
vsce:
	-${VSCE_SCRIPT}

# uploads and prints a link to the file to download
upload:
	-${UPLOAD_SCRIPT}
