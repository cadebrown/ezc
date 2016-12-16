
location=/usr/

install:
	./install-all.sh ${location}/bin/ ${location}/src/

install-local:
	./install-all.sh ~/ezc/ ~/ezc/src/

install-deb: deb
	sudo dpkg --force-architecture -i ezcc.deb
	sudo apt install -f

deb:
	./build-deb.sh


