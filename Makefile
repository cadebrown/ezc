
local:	
	-./install-all.sh ~/ezc/ ~/ezc/ true

global:
	-./install-all.sh /usr/bin/ /usr/src/ true

local-noreq:
	-./install-all.sh ~/ezc/ ~/ezc/

global-noreq:
	-./install-all.sh /usr/bin/ /usr/src/

req:
	-./make-req.sh ~/ezc/

uninstall:
	-./uninstall.sh

uninstall-local:
	-./uninstall.sh ~/ezc/ ~/ezc/