###                     EZC req/Makfile v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Installs requirements and dependencies for EZC.
#
#  Currently, to build EZC, you will need python, which will need a full featured C compiler.
#
#  If you already have a C compiler, use `make cc="CCOMPILER" python` to install python.
#  Otherwise, you can run `make gcc`, or if you know how to bootstrap it with tcc, do that.
#
#  To use EZC -> C code generation, you will need a (simple) C compiler. tcc will work for this.
#  Run `make tcc` to install tcc locally.
#
#  TODO:
#    * In the future, add a `make bundle-req` to compile all requirements needed to distribute EZC.
#
###

cc=cc
export CC=$(cc)


gmp.src: FORCE
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/; make install

mpfr.src: FORCE
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/ --with-gmp=$(PWD)/; make install

mpc.src: FORCE
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/ --with-gmp=$(PWD)/ --with-mpfr=$(PWD)/; make install

elf.src: FORCE
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/; make install

isl.src: FORCE
	cd $@; ./configure --disable-shared --enable-static --prefix=$(PWD)/; make install

gcc.src: FORCE
	cd $@; $(PWD)/$@/configure         \
	    --disable-multilib             \
		--enable-languages=c           \
		--disable-shared               \
		--enable-static                \
		--prefix=$(PWD)/               \
		--with-gmp=$(PWD)/             \
		--with-mpfr=$(PWD)/            \
		--with-mpc=$(PWD)/             \
		--with-libelf=$(PWD)/          \
		--with-isl=$(PWD)/;            \
		make; make install

tcc.src:
	cd $@; ./configure --prefix=$(PWD)/; make install

python.src:
	cd $@; ./configure --prefix=$(PWD)/; make; make install


gmp.tar:
	curl "http://gmplib.org/download/gmp/gmp-6.1.2.tar.xz" -L > $@
	tar xfv $@
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

mpfr.tar:
	curl "http://www.mpfr.org/mpfr-current/mpfr-3.1.5.tar.xz" -L > $@
	tar xfv $@ 
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

mpc.tar:
	curl "ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz" -L > $@
	tar xfv $@
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

elf.tar:
	curl "http://www.mr511.de/software/libelf-0.8.13.tar.gz" -L > $@
	tar xfv $@
	mv libelf* elf.src

isl.tar:
	curl "ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2" -L > $@
	tar xfv $@
	mv isl-* isl.src

gcc.tar:
	curl "http://gcc.parentingamerica.com/releases/gcc-6.3.0/gcc-6.3.0.tar.bz2" -L > $@
	tar xfv $@
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

tcc.tar:
	curl "http://download.savannah.gnu.org/releases/tinycc/tcc-0.9.26.tar.bz2" -L > $@
	tar xfv $@
	mv $(subst .tar,,$@)-* $(subst .tar,,$@).src

python.tar:
	curl "http://www.python.org/ftp/python/2.7.13/Python-2.7.13.tar.xz" -L > $@
	tar xfv $@
	mv Python-* python.src



gmp: gmp.tar gmp.src
	-
mpfr: gmp mpfr.tar mpfr.src
	-
mpc: gmp mpfr mpc.tar mpc.src
	-
elf: elf.tar elf.src
	-
isl: isl.tar isl.src
	-
gcc: gmp mpfr mpc elf gcc.tar gcc.src
	-
tcc: tcc.tar tcc.src
	-
python: python.tar python.src
	-

FORCE:

