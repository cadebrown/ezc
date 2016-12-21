local:	
	-./scripts/install.sh ~/ezc/ ~/ezc/ true

global:
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/ezc/ true

local-noreq:
	-./scripts/install.sh ~/ezc/ ~/ezc/

global-noreq:
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/

req:
	-./scripts/mpfr.sh

uninstall-local:
	-./scripts/uninstall.sh ~/ezc/ ~/ezc/

uninstall-global:
	-sudo ./scripts/uninstall.sh /usr/bin/ /usr/src/

deb:
	-./scripts/deb.sh

vscode:
	-./scripts/vscode.sh

check:
	-python src/ezcc.py example.ezc -run -v4

check-global:
	/usr/bin/ezcc example.ezc -run -v4