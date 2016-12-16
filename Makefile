
location=/usr/

install:
	./install-all.sh ${location}/bin/ ${location}/src/

local:
	./install-all.sh ~/ezc/ ~/ezc/src/

deb:
	./build-deb.sh


