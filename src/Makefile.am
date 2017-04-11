###  src/Makefile.am -- Makefile for automake compiling main executables
#  
#    Copyright 2016-2017 ChemicalDevelopment
#  
#    This file is part of the EZC project.
#  
#    EZC, EC, EZC Documentation, and any other resources in this project are 
#  free software; you are free to redistribute it and/or modify them under 
#  the terms of the GNU General Public License; either version 3 of the 
#  license, or any later version.
#  
#    These programs are hopefully useful and reliable, but it is understood 
#  that these are provided WITHOUT ANY WARRANTY, or MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE. See the GPLv3 or email at 
#  <info@chemicaldevelopment.us> for more info on this.
#  
#    Here is a copy of the GPL v3, which this software is licensed under. You 
#  can also find a copy at http://www.gnu.org/licenses/.
###  

# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=c99 -O2 @GMPCFLAGS@ @MPFRCFLAGS@
AM_LDFLAGS = -I./

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = ec

ec_SOURCES = ezc.h ezc_colors.h ezc.c helper.c obj.c stack.c dict.c getargs.c eval.c gmp_addon.c str_addon.c
ec_LDADD = @GMPLDFLAGS@ @MPFRLDFLAGS@

