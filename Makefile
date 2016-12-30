HERE_DIR="./ezc/"
HERE_SRC_DIR="${HERE_DIR}/src/"

LOCAL_DIR="~/ezc/"
LOCAL_SRC_DIR="${LOCAL_DIR}/src"

GLOBAL_DIR="/usr/bin/"
GLOBAL_SRC_DIR="/usr/src/ezc"

EXAMPLE_FILE="./example.ezc"

SCRIPTS="./scripts/"

INSTALL="${SCRIPTS}/install.sh"
UNINSTALL="${SCRIPTS}/uninstall.sh"
BUNDLE="${SCRIPTS}/bundle.sh"

REQ="$SCRIPTS/mpfr.sh"


here: uninstall-here
	-${INSTALL} ${HERE_DIR} ${HERE_SRC_DIR} true

local: uninstall-local
	-${INSTALL} ${LOCAL_DIR} ${LOCAL_SRC_DIR} true

global: uninstall-global
	-sudo ${INSTALL} ${GLOBAL_DIR} ${LOCAL_SRC_DIR} true


here-noreq: uninstall-here
	-${INSTALL} ${HERE_DIR} ${HERE_SRC_DIR}

local-noreq: uninstall-local
	-${INSTALL} ${LOCAL_DIR} ${LOCAL_SRC_DIR}

global-noreq: uninstall-global
	-sudo ${INSTALL} ${GLOBAL_DIR} ${LOCAL_SRC_DIR}

uninstall-here:
	-${UNINSTALL} ${HERE_DIR} ${HERE_SRC_DIR}

uninstall-local:
	-${UNINSTALL} ${LOCAL_DIR} ${LOCAL_SRC_DIR}

uninstall-global:
	-sudo ${UNINSTALL} ${GLOBAL_DIR} ${GLOBAL_SRC_DIR}

check:
	python src/ezcc.py example.ezc -run -v4

check-here:
	${HERE_DIR}/ezc example.ezc -run -v4

check-local:
	${LOCAL_DIR}/ezc example.ezc -run -v4

check-global:
	${GLOBAL_DIR}/ezc example.ezc -run -v4


req:
	-${REQ} ./req/


bundle: local
	-./scripts/bundle.sh ezc.tar.xz ~/ezc/

deb:
	-./scripts/deb.sh



vscode:
	-./scripts/vscode.sh




html:
	./scripts/html.sh