local:	
	-./install.sh ~/ezc/ ~/ezc/ true

global:
	-./install.sh /usr/bin/ /usr/src/ezc/ true

local-noreq:
	-./install.sh ~/ezc/ ~/ezc/

global-noreq:
	-./install.sh /usr/bin/ /usr/src/

req:
	-./build-scripts/mpfr.sh

uninstall-local:
	-./uninstall.sh ~/ezc/ ~/ezc/

uninstall-global:
	-./uninstall.sh /usr/bin/ /usr/src/

deb:
	-./build-scripts/deb.sh

test-local:
	-~/ezc/ezcc.py -c "prec 1000:var (sqrt 2)" -v5
