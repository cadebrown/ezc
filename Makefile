local:	
	-./scripts/install.sh ~/ezc/ ~/ezc/src/ true

global:
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/ezc/ true


local-noreq:
	-./scripts/install.sh ~/ezc/ ~/ezc/src/

global-noreq:
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/ezc/


uninstall-local:
	-./scripts/uninstall.sh ~/ezc/ ~/ezc/src/

uninstall-global:
	-sudo ./scripts/uninstall.sh /usr/bin/ /usr/src/ezc/


req:
	-./scripts/mpfr.sh


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

html:
	./scripts/html.sh