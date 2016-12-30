local: uninstall-local
	-./scripts/install.sh ~/ezc/ ~/ezc/src/ true

global: uninstall-global
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/ezc/ true


local-noreq: uninstall-local
	-./scripts/install.sh ~/ezc/ ~/ezc/src/

global-noreq: uninstall-global
	-sudo ./scripts/install.sh /usr/bin/ /usr/src/ezc/


uninstall-local:
	-./scripts/uninstall.sh ~/ezc/ ~/ezc/src/

uninstall-global:
	-sudo ./scripts/uninstall.sh /usr/bin/ /usr/src/ezc/


req:
	-./scripts/mpfr.sh ~/ezc/src/


bundle: local
	-./scripts/bundle.sh ~/ezc/

deb:
	-./scripts/deb.sh

vscode:
	-./scripts/vscode.sh

check:
	python src/ezcc.py example.ezc -run -v4

check-local:
	~/ezc/ezc example.ezc -run -v4

check-global:
	/usr/bin/ezcc example.ezc -run -v4

html:
	./scripts/html.sh