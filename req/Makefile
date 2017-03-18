
cc=cc
export CC=$(cc)


gmp.src:
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/; make install

mpfr.src:
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/ --with-gmp=$(PWD)/; make install

mpc.src:
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/ --with-gmp=$(PWD)/ --with-mpfr=$(PWD)/; make install

elf.src:
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/; make install

gcc.src:
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/ --with-gmp=$(PWD)/ --with-mpfr=$(PWD)/ --with-mpc=$(PWD)/ --with-libelf=$(PWD)/; make; make install

tcc.src:
	cd $@; ./configure --prefix=$(PWD)/; make install

python.src:
	cd $@; ./configure --prefix=$(PWD)/; make; make install


gmp.tar:
	curl "http://gmplib.org/download/gmp/gmp-6.1.2.tar.xz" -L > $@
	tar xfv $@ -C 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

mpfr.tar:
	curl "http://www.mpfr.org/mpfr-current/mpfr-3.1.5.tar.xz" -L > $@
	tar xfv $@ -C 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

mpc.tar:
	curl "ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz" -L > $@
	tar xfv $@ -C 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

elf.tar:
	curl "http://www.mr511.de/software/libelf-0.8.13.tar.gz" -L > $@
	tar xfv $@ -C 
	mv /libelf* /elf.src

gcc.tar:
	curl "http://gcc.parentingamerica.com/releases/gcc-6.3.0/gcc-6.3.0.tar.bz2" -L > $@
	tar xfv $@ -C 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

tcc.tar:
	curl "http://download.savannah.gnu.org/releases/tinycc/tcc-0.9.26.tar.bz2" -L > $@
	tar xfv $@ -C 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

python.tar:
	curl "http://www.python.org/ftp/python/2.7.13/Python-2.7.13.tar.xz" -L > $@
	tar xfv $@ -C 
	mv /Python-* /python.src



gmp: gmp.tar gmp.src
	-
mpfr: gmp mpfr.tar mpfr.src
	-
mpc: gmp mpfr mpc.tar mpc.src
	-
elf: elf.tar elf.src
	-
gcc: gmp mpfr mpc elf gcc.tar gcc.src
	-
tcc: tcc.tar tcc.src
	-
python: python.tar python.src
	-

FORCE:

