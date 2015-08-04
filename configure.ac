dnl Process this file with autoconf to produce a configure script.

AC_INIT([fpm2], [0.79], [als@regnet.cz], [fpm2], [http://als.regnet.cz/fpm2/])
AM_INIT_AUTOMAKE([no-dist-gzip dist-bzip2])
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AC_C_BIGENDIAN(
    [AC_DEFINE(BIG_ENDIAN_HOST, 1, [Big endian host])], 
    [AC_DEFINE(LITTLE_ENDIAN_HOST, 1, [Little endian host])]
)

AC_CHECK_LIB([m], [pow])
pkg_modules="x11 glib-2.0 >= 2.16 gtk+-2.0 libxml-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext package.])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

pixmapdir="${datadir}/pixmaps/${PACKAGE}"
AC_SUBST(pixmapdir)

# Debug mode
AC_ARG_ENABLE(debug,
	    AC_HELP_STRING([--enable-debug],
	    [Compile binaries in debug mode]))

if test "$enable_debug" = "yes"; then
  echo "enabling debug compile mode"
  AC_DEFINE(FPM_DEBUG, 1, [Enable debug compile mode])
else
  AC_DEFINE(G_DISABLE_ASSERT, 1, [Disable Glib assertion checks])
  AC_DEFINE(G_DISABLE_CHECKS, 1, [Disable Glib check functions])
fi

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
data/Makefile
data/pixmaps/Makefile
data/fpm2.desktop.in
])
