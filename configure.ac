dnl Process this file with autoconf to produce a configure script.

AC_INIT(wreport, [3.13], [enrico@enricozini.org])
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_HEADER(config.h)
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

AC_ARG_ENABLE([python],
    [AS_HELP_STRING(
        [--enable-python],
        [build Python bindings])],
    [],
    [enable_python="yes"])

dnl To use subdirs
AC_PROG_MAKE_SET

AC_LANG([C++])

dnl Use c++11
AX_CXX_COMPILE_STDCXX_11

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX

AC_SYS_LARGEFILE

dnl See http://www.gossamer-threads.com/lists/xen/devel/275132
FILE_OFFSET_BITS_64=
if test $ac_cv_sys_file_offset_bits = 64
then
    FILE_OFFSET_BITS_64=yes
fi
AM_CONDITIONAL([FILE_OFFSET_BITS_64], [test x"$FILE_OFFSET_BITS_64" = x"yes"])

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

PKGCONFIG_LIBS="-lm"
PKGCONFIG_REQUIRES=""
AC_SUBST(PKGCONFIG_LIBS)
AC_SUBST(PKGCONFIG_REQUIRES)

dnl Check for lua
PKGCONFIG_LUA_MODULE="lua >= 5.1.1"
PKG_CHECK_EXISTS([lua5.2], [have_lua=yes], [have_lua=no])
if test x$have_lua = xyes
then
        PKG_CHECK_MODULES(LUA,lua5.2,,[have_lua=no])
        PKGCONFIG_REQUIRES="$PKGCONFIG_REQUIRES $PKGCONFIG_LUA_MODULE"
else
	PKG_CHECK_EXISTS([lua5.1], [have_lua=yes], [have_lua=no])
	if test x$have_lua = xyes
	then
		PKG_CHECK_MODULES(LUA,lua5.1,,[have_lua=no])
        PKGCONFIG_REQUIRES="$PKGCONFIG_REQUIRES $PKGCONFIG_LUA_MODULE"
	else
		PKG_CHECK_EXISTS([lua], [have_lua=yes], [have_lua=no])
		if test x$have_lua = xyes
		then
			PKG_CHECK_MODULES(LUA,[$PKGCONFIG_LUA_MODULE],,[have_lua=no])
            PKGCONFIG_REQUIRES="$PKGCONFIG_REQUIRES $PKGCONFIG_LUA_MODULE"
		else
			dnl We don't always have the luxury of .pc files for lua, it seems
			have_lua=yes
			AC_CHECK_LIB([lua], [lua_gettop], [true], [have_lua=no])
			AC_CHECK_HEADER([lua.h], [true], [have_lua=no])
			LUA_CFLAGS=
			LUA_LIBS="-llua"
            PKGCONFIG_LIBS="$PKGCONFIG_LIBS $LUA_LIBS"
		fi
	fi
fi
if test x$have_lua = xyes
then
        AC_DEFINE([HAVE_LUA], 1, [lua is available])
fi
AM_CONDITIONAL([LUA], [test x"$have_lua" = x"yes"])

dnl Check for python
if test x$enable_python = xyes
then
    dnl Check for python
    AM_PATH_PYTHON([], [], [enable_python=no])
fi
if test x$enable_python = xyes
then
    PYTHON_CFLAGS=`unset CFLAGS; $PYTHON-config --cflags`
    AC_SUBST(PYTHON_CFLAGS)
    AC_DEFINE(HAVE_PYTHON,[],[Have Python bindings])
fi
AM_CONDITIONAL([PYTHON], [test x"$enable_python" = x"yes"])

AC_CHECK_PROG(have_rst2html, rst2html, yes, no)
AM_CONDITIONAL([HAVE_RST2HTML], [test x$have_rst2html = xyes])

LIBS="$LIBS -lm"

confdir='${sysconfdir}'"/$PACKAGE"
AC_SUBST(confdir)

tabledir='${datadir}'"/$PACKAGE"
AC_SUBST(tabledir)
dnl def_tabledir=`echo "${sharedstatedir}/lib/$PACKAGE/tables"`
dnl AC_DEFINE_UNQUOTED(TABLE_DIR, "$def_tabledir", "Default location for table files")

dnl From libtool's documentation:
dnl
dnl This flag accepts an argument of the form `CURRENT[:REVISION[:AGE]]'.
dnl
dnl CURRENT
dnl      The most recent interface number that this library implements.
dnl 
dnl REVISION
dnl      The implementation number of the CURRENT interface.
dnl 
dnl AGE
dnl      The difference between the newest and oldest interfaces that this
dnl      library implements.  In other words, the library implements all the
dnl      interface numbers in the range from number `CURRENT - AGE' to
dnl      `CURRENT'.
dnl
dnl If two libraries have identical CURRENT and AGE numbers, then the
dnl dynamic linker chooses the library with the greater REVISION number.
dnl
dnl  3. If the library source code has changed at all since the last
dnl     update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
dnl
dnl  4. If any interfaces have been added, removed, or changed since the
dnl     last update, increment CURRENT, and set REVISION to 0.
dnl
dnl  5. If any interfaces have been added since the last public release,
dnl     then increment AGE.
dnl
dnl  6. If any interfaces have been removed since the last public release,
dnl     then set AGE to 0.
LIBWREPORT_VERSION_INFO="3:3:0"
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

AC_CONFIG_FILES([
Makefile
wreport/Makefile
src/Makefile
tables/Makefile
python/Makefile
testdata/Makefile
doc/Makefile
doc/libwreport.dox
libwreport.pc
])
#tools/Makefile
AC_OUTPUT

AC_MSG_NOTICE([
===================================================
$PACKAGE_NAME-$PACKAGE_VERSION configuration:
AS_HELP_STRING([lua:], [$have_lua])
AS_HELP_STRING([python:], [$enable_python])
AS_HELP_STRING([rst2html:], [$have_rst2html])
AS_HELP_STRING([documentation:], [$enable_docs])
===================================================])
