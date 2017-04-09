###  configure.ac -- Autoconf configuration script
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


AC_PREREQ([2.55])
AC_INIT([EZC], [1.4.0], [ezc@chemicaldevelopment.us])
AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([src/ezc.h])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h stdio.h stdbool.h string.h])

# Checks for libraries.


# GMP can be supplied to use multiprecision support

AC_ARG_WITH(gmp,
  AS_HELP_STRING([--with-gmp],
                 [Use GMP. Default: auto-detect]), [
#AC_MSG_ERROR("$with_gmp")
case "$with_gmp" in
  yes)
    withval=/usr/local
    ;;
  no)
    : #dont use it
    ;;
esac
AC_MSG_CHECKING(GMP)

if test ! -d "$withval" ; then
	AC_MSG_RESULT(no ($withval not a directory))
	AC_MSG_ERROR([--with-gmp path does not point to a directory])
fi
if test ! -e "$withval/include/gmp.h" ; then
	AC_MSG_RESULT(no ($withval/include/gmp.h not found))
	AC_MSG_ERROR([--with-gmp path does not include `include/gmp.h` header file ($withval/include/gmp.h not found)])
fi
AC_MSG_RESULT(yes ($withval))
AC_DEFINE([USE_GMP], [1], [Use GMP library code])
LIBGMP_PATH="-L$with_gmp/lib -lgmp"
GMP_CFLAGS="-I$with_gmp/include"
], [])

AH_TEMPLATE(USE_GMP,[GMP support is available])


# defines additional macros

AC_DEFINE([DEFAULT_STACK_LEN], [20], [Default stack length])
AC_DEFINE([DEFAULT_DICT_LEN], [20], [Default dictionary keys and values length])

AC_DEFINE([MAX_INTERPERET_LEN], [10000], [The maximum size for an operator string])
AC_DEFINE([MAX_OP_LENGTH], [5], [The maximum size for an operator string])

AC_DEFINE([MAX_ERR_STR], [2000], [Maxmimum length of an error string])

AC_DEFINE([PRINT_OFFSET], [4], [The offset of printing])
AC_DEFINE([ARGS_PRINT_OFFSET], [25], [The offset of printing for args])
AC_DEFINE([PRINT_COL_OFFSET], [35], [The offset of columm printing])


# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_SUBST(GMPCFLAGS, $GMP_CFLAGS)
AC_SUBST(GMPLDFLAGS, $LIBGMP_PATH)

# todo, add tests
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
