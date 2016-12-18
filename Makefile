
local:	
	-./install-all.sh ~/ezc/ ~/ezc/ true

global:
	-./install-all.sh /usr/bin/ /usr/src/ezc/ true

local-noreq:
	-./install-all.sh ~/ezc/ ~/ezc/

global-noreq:
	-./install-all.sh /usr/bin/ /usr/src/

req:
	-./make-req.sh ~/ezc/

uninstall-local:
	-./uninstall.sh ~/ezc/ ~/ezc/

uninstall-global:
	-./uninstall.sh /usr/bin/ /usr/src/