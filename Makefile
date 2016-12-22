local:	
	-./scripts/install.sh ~/ezc/ true

global:
	-sudo ./scripts/install.sh /usr/bin/ true

local-noreq:
	-./scripts/install.sh ~/ezc/

global-noreq:
	-sudo ./scripts/install.sh /usr/bin/

req:
	-./scripts/mpfr.sh

uninstall-local:
	-./scripts/uninstall.sh ~/ezc/

uninstall-global:
	-sudo ./scripts/uninstall.sh /usr/bin/

deb:
	-./scripts/deb.sh

vscode:
	-./scripts/vscode.sh

check:
	python src/ezcc.py example.ezc -run -v4

check-local:
	~/ezc/ezcc example.ezc -run -v4

check-global:
	/usr/bin/ezcc example.ezc -run -v4