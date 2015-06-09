## Quimby Input Method, Copyright 2013, Alexandre Muñiz. <http://puzzlezapper.com
## Build files based on Multipress Input Method, Copyright (c) 2007  Openismus GmbH  <http://www.openismus.com/>

AC_INIT([quimby-gtk-input-method], [0.0.1], [pz@puzzlezapper.com], [quimby-gtk-input-method])
AC_PREREQ([2.58])

AC_CONFIG_SRCDIR([src/im-quimby.c])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.8 -Wall gnu no-define])
AC_ARG_VAR([ACLOCAL_FLAGS], [aclocal flags, e.g. -I <macro dir>])

AC_PROG_CC()
# Check whether the compiler accepts -c -o, which allows for non-recursive
# make.  This simplifies the build machinery for the examples quite a bit.
AM_PROG_CC_C_O()

# Don't build static libraries as that's pointless for loadable modules.
AC_DISABLE_STATIC()
AC_PROG_LIBTOOL()

AC_SUBST([GETTEXT_PACKAGE], [quimby-gtk-input-method])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])
IT_PROG_INTLTOOL([0.35])
AM_GLIB_GNU_GETTEXT()

PKG_CHECK_MODULES([QUIMBY_MODULES], [gtk+-2.0 >= 2.6.0])

AC_ARG_VAR([GTK_QUERY_IMMODULES], [path to gtk-query-immodules-2.0 utility])
AC_CHECK_PROGS([GTK_QUERY_IMMODULES], [gtk-query-immodules-2.0])

AS_IF([test "x$GTK_QUERY_IMMODULES" = x],
      [AC_MSG_FAILURE([gtk-query-immodules-2.0 not found.])])

#GTK2_REQUIRED=2.24.0
#PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= GTK2_REQUIRED,
#			[have_gtk2=yes], [have_gtk2=no])
#AM_CONDITIONAL(HAVE_GTK2, test $have_gtk2 = "yes")
#if test $have_gtk2 = "yes"; then
#   gtk2_binary_version=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
#   gtk2_libdir=`$PKG_CONFIG --variable=libdir gtk+-2.0`
#   GTK2_IM_MODULEDIR="$gtk2_libdir"/gtk-2.0/$gtk2_binary_version/immodules
#   AC_SUBST(GTK2_IM_MODULEDIR)
#fi


QUIMBY_ARG_WITH_IMMODULES_DIR()

# The -w1 is there for the benefit of the Intel compiler.
DK_ARG_ENABLE_WARNINGS([QUIMBY_WARNING_FLAGS],
                       [-Wall -w1],
                       [-pedantic -Wall -Wextra -w1],
                       [G PANGO ATK GDK GTK])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  po/Makefile.in
])
AC_OUTPUT()
