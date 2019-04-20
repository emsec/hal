AC_PREREQ([2.57])
AC_INIT([buddy], [2.4])
AC_CONFIG_AUX_DIR([tools])
AM_INIT_AUTOMAKE([foreign nostdinc no-define 1.7.3])

MAJOR_VERSION=`echo "$VERSION" | sed -e 's/^\([[0-9]]*\)\.[[0-9]]*.*$/\1/'`
MINOR_VERSION=`echo "$VERSION" | sed -e 's/^[[0-9]]*\.\([[0-9]]*\).*$/\1/'`
AC_DEFINE_UNQUOTED([MAJOR_VERSION], [$MAJOR_VERSION], [BuDDy's major version.])
AC_DEFINE_UNQUOTED([MINOR_VERSION], [$MINOR_VERSION], [BuDDy's minor version.])

AC_PROG_CC

AM_PROG_LEX
AC_PROG_YACC

AC_PROG_CXX

AC_PROG_LIBTOOL

buddy_DEBUG_FLAGS

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  doc/Makefile
  examples/Makefile
  examples/adder/Makefile
  examples/bddcalc/Makefile
  examples/bddtest/Makefile
  examples/cmilner/Makefile
  examples/fdd/Makefile
  examples/milner/Makefile
  examples/money/Makefile
  examples/queen/Makefile
  examples/solitare/Makefile
])

AC_OUTPUT
