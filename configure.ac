dnl Process this file with autoconf to produce a configure script.

AC_INIT(wreport, [2.15], [enrico@enricozini.org])
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

dnl Use libtool
dnl AM_ENABLE_STATIC
dnl AM_DISABLE_SHARED
LT_INIT()
LT_LANG([C++])
AM_PROG_LIBTOOL

AC_ARG_ENABLE([docs],
    [AS_HELP_STRING(
        [--enable-docs],
        [enable building of the documentation])],
    [],
    [enable_docs="yes"])

AC_ARG_ENABLE([doxy-undoc-warnings],
    [AS_HELP_STRING(
	[--enable-doxy-undoc-warnings],
        [enable building of the documentation])],
    [],
    [enable_doxy_undoc_warnings="no"])

dnl To use subdirs
AC_PROG_MAKE_SET

AC_LANG([C++])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX

AM_PROG_CC_STDC
AC_HEADER_STDC

dnl Needed by subdir-objects
AM_PROG_CC_C_O

dnl Check for missing functions in libc
AC_CHECK_FUNC(vasprintf, , [
	AC_DEFINE([USE_OWN_VASPRINTF], 1, [we need to use our own vasprintf])
])

dnl Check for missing functions in libc
AC_CHECK_FUNC(getopt_long, [
	AC_DEFINE([HAS_GETOPT_LONG], 1, [we can use long options])
])

if test x$enable_docs = xyes
then
	dnl Check for doxygen
	AC_CHECK_PROG(have_doxygen, doxygen, true)
	if test x$have_doxygen != xtrue
	then
		AC_MSG_WARN([doxygen was not found: documentation will not be built])
		enable_docs=no
	fi
fi

dnl Check for gperf
AC_CHECK_PROG(have_gperf, gperf, true)
if test x$have_gperf != xtrue
then
	AC_MSG_WARN([gperf was not found: relying on prebuilt wreport/uniconv.cc])
	if ! test -f wreport/uniconv.cc
	then
		AC_MSG_ERROR([please extract wreport/uniconv.cc from the distribution tarball])
	fi
fi

dnl Check if std::vector has a std::vector::data method
AC_CACHE_CHECK(whether the compiler has std::vector::data (),
	ac_cv_cxx_have_vector_data,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <vector>
using namespace std;
],[vector<int> v; v.data(); return 0;],
 ac_cv_cxx_have_vector_data=yes, ac_cv_cxx_have_vector_data=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_vector_data" = yes; then
 AC_DEFINE(HAVE_VECTOR_DATA,,[define if the compiler has the method
std::vector::data ()])
fi

if test x$docdir = x
then
	docdir='${datadir}/doc/${PACKAGE_TARNAME}'
	AC_SUBST(docdir)
fi

DEP_LIBS=""
AC_SUBST(DEP_LIBS)

dnl Check for lua
PKG_CHECK_EXISTS([lua5.2], [have_lua=yes], [have_lua=no])
if test x$have_lua = xyes
then
        PKG_CHECK_MODULES(LUA,lua5.2,,[have_lua=no])
else
	PKG_CHECK_EXISTS([lua5.1], [have_lua=yes], [have_lua=no])
	if test x$have_lua = xyes
	then
		PKG_CHECK_MODULES(LUA,lua5.1,,[have_lua=no])
	else
		PKG_CHECK_EXISTS([lua], [have_lua=yes], [have_lua=no])
		if test x$have_lua = xyes
		then
			PKG_CHECK_MODULES(LUA,lua,,[have_lua=no])
		else
			dnl We don't always have the luxury of .pc files for lua, it seems
			have_lua=yes
			AC_CHECK_LIB([lua], [lua_gettop], [true], [have_lua=no])
			AC_CHECK_HEADER([lua.h], [true], [have_lua=no])
			LUA_CFLAGS=
			LUA_LIBS="-llua"
		fi
	fi
fi
if test x$have_lua = xyes
then
        AC_DEFINE([HAVE_LUA], 1, [lua is available])
        DEP_LIBS="$DEP_LIBS $LUA_LIBS"
fi
AM_CONDITIONAL([LUA], [test x"$have_lua" = x"yes"])

dnl Check for wibble
PKG_CHECK_EXISTS([libwibble], [have_wibble=yes], [have_wibble=no])
if test x$have_wibble = xyes
then
	PKG_CHECK_MODULES(WIBBLE,libwibble)
	enable_tests=yes
	DEP_LIBS="$DEP_LIBS $WIBBLE_LIBS"
else
	AC_MSG_WARN([libwibble not found, tests will be disabled])
	enable_tests=no
fi

LIBS="$LIBS -lm"

confdir='${sysconfdir}'"/$PACKAGE"
AC_SUBST(confdir)

tabledir='${datadir}'"/$PACKAGE"
AC_SUBST(tabledir)
dnl def_tabledir=`echo "${sharedstatedir}/lib/$PACKAGE/tables"`
dnl AC_DEFINE_UNQUOTED(TABLE_DIR, "$def_tabledir", "Default location for table files")

LIBWREPORT_VERSION_INFO=`echo $VERSION | awk -F. '{ printf "%d:%d:%d", $1+$2, $3, $2 }'`
AC_SUBST(LIBWREPORT_VERSION_INFO)

dnl Give me warnings
AX_CXXFLAGS_WARN_ALL

if test x"$enable_doxy_undoc_warnings" = x"no"
then
	DOXYGEN_WARN_IF_UNDOCUMENTED=NO
else
	DOXYGEN_WARN_IF_UNDOCUMENTED=YES
fi
AC_SUBST(DOXYGEN_WARN_IF_UNDOCUMENTED)

AM_CONDITIONAL([DO_DOCS], [test x"$enable_docs" = x"yes"])
AM_CONDITIONAL([DO_TESTS], [test x"$enable_tests" = x"yes"])
AM_CONDITIONAL([WIBBLE], [test x"$have_wibble" = x"yes"])

AC_CONFIG_FILES([
Makefile
wreport/Makefile
src/Makefile
tables/Makefile
testdata/Makefile
doc/Makefile
doc/libwreport.dox
libwreport.pc
libwreport-test.pc
])
#tools/Makefile
AC_OUTPUT

AC_MSG_NOTICE([
===================================================
$PACKAGE_NAME-$PACKAGE_VERSION configuration:
AS_HELP_STRING([lua bindings:], [$have_lua])
AS_HELP_STRING([tests:], [$enable_tests])
AS_HELP_STRING([documentation:], [$enable_docs])
===================================================])
