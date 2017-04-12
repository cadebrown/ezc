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
AC_CHECK_HEADERS([stdlib.h stdio.h stdbool.h string.h math.h])

# Checks for libraries.

AC_CHECK_LIB(m, log)
GMP_DIR=""
MPFR_DIR=""


# GMP can be supplied to use multiprecision support

AC_ARG_WITH(gmp,
  AS_HELP_STRING([--with-gmp],
                 [Use GMP. Default: auto-detect]), [
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
GMP_DIR="$withval"
], [])

AH_TEMPLATE(USE_GMP,[GMP support is available])


# MPFR for multiprecision floats
# TODO: detect which functions are present

AC_ARG_WITH(mpfr,
  AS_HELP_STRING([--with-mpfr],
                 [Use MPFR. Default: auto-detect]), [
case "$with_mpfr" in
  yes)
    withval=/usr/local
    ;;
  no)
    : #dont use it
    ;;
esac
AC_MSG_CHECKING(MPFR)

if test ! -d "$withval" ; then
	AC_MSG_RESULT(no ($withval not a directory))
	AC_MSG_ERROR([--with-mpfr path does not point to a directory])
fi
if test ! -e "$withval/include/mpfr.h" ; then
	AC_MSG_RESULT(no ($withval/include/mpfr.h not found))
	AC_MSG_ERROR([--with-mpfr path does not include `include/mpfr.h` header file ($withval/include/mpfr.h not found)])
fi
AC_MSG_RESULT(yes ($withval))
AC_DEFINE([USE_MPFR], [1], [Use MPFR library code])
MPFR_DIR="$withval"
], [])

AH_TEMPLATE(USE_MPFR,[MPFR support is available])

# readline support for terminal

AC_ARG_WITH(readline,
  AS_HELP_STRING([--with-readline],
                 [Use readline. Default: auto-detect]), [
case "$with_readline" in
  yes)
    withval=/usr/local
    ;;
  no)
    : #dont use it
    ;;
esac
AC_MSG_CHECKING(GNU Readline)

if test ! -d "$withval" ; then
	AC_MSG_RESULT(no ($withval not a directory))
	AC_MSG_ERROR([--with-readline path does not point to a directory])
fi
if test ! -e "$withval/include/readline/readline.h" ; then
	AC_MSG_RESULT(no ($withval/include/readline/readline.h not found))
	AC_MSG_ERROR([--with-readline path does not include `readline/readline.h` header file ($withval/include/readline/readline.h not found)])
fi
AC_MSG_RESULT(yes ($withval))
AC_DEFINE([USE_READLINE], [1], [Use GNU Readline code])
READLINE_DIR="$withval"
], [])

AH_TEMPLATE(USE_READLINE,[GNU Readline support is available])




AC_ARG_ENABLE([static],
    AS_HELP_STRING([--enable-static], [Enable static building]))
AS_IF([test "x$enable_static" = "xyes"], [ 
  AS_IF([test "x$GMP_DIR" != "x"], [
    GMP_LDFLAGS="$GMP_DIR/lib/libgmp.a"
    GMP_CFLAGS="-I$GMP_DIR/include"
  ])
  AS_IF([test "x$MPFR_DIR" != "x"], [
    MPFR_LDFLAGS="$MPFR_DIR/lib/libmpfr.a"
    MPFR_CFLAGS="-I$MPFR_DIR/include"
  ])
  AS_IF([test "x$READLINE_DIR" != "x"], [
    READLINE_LDFLAGS="$READLINE_DIR/lib/libreadline.a"
    READLINE_CFLAGS="-I$READLINE_DIR/include"
    if test ! -e "$READLINE_DIR/lib/libreadline.a" ; then
      READLINE_LDFLAGS="-L$READLINE_DIR/lib -lreadline"
      AC_MSG_WARN([--with-readline path does not have a static library, using dynamic])
    fi
  ])
], [
  #dynamic libs
  AS_IF([test "x$GMP_DIR" != "x"], [
    GMP_LDFLAGS="-L$GMP_DIR/lib -lgmp"
    GMP_CFLAGS="-I$GMP_DIR/include"
  ])
  AS_IF([test "x$MPFR_DIR" != "x"], [
    MPFR_LDFLAGS="-L$MPFR_DIR/lib -lmpfr"
    MPFR_CFLAGS="-I$MPFR_DIR/include"
  ])
  AS_IF([test "x$READLINE_DIR" != "x"], [
    READLINE_LDFLAGS="-L$READLINE_DIR/lib -lreadline"
    READLINE_CFLAGS="-I$READLINE_DIR/include"
  ])
])

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

# macro subs
AC_SUBST(GMPCFLAGS, $GMP_CFLAGS)
AC_SUBST(GMPLDFLAGS, $GMP_LDFLAGS)

AC_SUBST(MPFRCFLAGS, $MPFR_CFLAGS)
AC_SUBST(MPFRLDFLAGS, $MPFR_LDFLAGS)

AC_SUBST(READLINECFLAGS, $READLINE_CFLAGS)
AC_SUBST(READLINELDFLAGS, $READLINE_LDFLAGS)


# todo, add tests
AC_CONFIG_FILES([Makefile src/Makefile mini.src/Makefile])
AC_OUTPUT
