#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

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


#defines additional macros

AC_DEFINE([DEFAULT_STACK_LEN], [20], [Default stack length])
AC_DEFINE([DEFAULT_DICT_LEN], [20], [Default dictionary keys and values length])

AC_DEFINE([MAX_INTERPERET_LEN], [10000], [The maximum size for an operator string])
AC_DEFINE([MAX_OP_LENGTH], [5], [The maximum size for an operator string])

AC_DEFINE([PRINT_OFFSET], [25], [The offset of printing])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_SUBST(GMPCFLAGS, $GMP_CFLAGS)
AC_SUBST(GMPLDFLAGS, $LIBGMP_PATH)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
