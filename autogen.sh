#! /bin/sh
# Copyright (c) 2007  Openismus GmbH  <http://www.openismus.com/>
# This file is derived from multipress-gtk-input-method,
# for use in Quimby

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

if test "$#$NOCONFIGURE" = 0
then
  echo "I am going to run $srcdir/configure with no arguments -- if you"
  echo "wish to pass any to it, please specify them on the $0 command line."
fi

# Let the user override the default choice of tools.
if test -z "$ACLOCAL" || test -z "$AUTOMAKE"
then
  # Prefer explicitely versioned executables.
  for version in 1.10 1.9 1.8
  do
    if { "aclocal-$version" --version && "automake-$version" --version; } </dev/null >/dev/null 2>&1
    then
      ACLOCAL=aclocal-$version
      AUTOMAKE=automake-$version
      break
    fi
  done
fi

( # Enter a subshell to temporarily change the working directory.
  cd "$srcdir" || exit 1

  # Explicitely delete some old cruft, which seems to be
  # more reliable than --force options and the like.
  rm -f	acconfig.h config.cache config.guess config.rpath config.sub depcomp \
	install-sh intltool-extract.in intltool-merge.in intltool-update.in \
	missing mkinstalldirs po/Makefile.in.in
  rm -rf autom4te.cache

  #WARNINGS=all; export WARNINGS
  # Trace commands and exit if a command fails.
  set -ex

  glib-gettextize --copy --force
  intltoolize --automake --copy --force
  ${LIBTOOLIZE-libtoolize} --copy --force
  ${ACLOCAL-aclocal} -I macros $ACLOCAL_FLAGS
  ${AUTOCONF-autoconf}
  ${AUTOHEADER-autoheader}
  ${AUTOMAKE-automake} --add-missing --copy $AUTOMAKE_FLAGS
) || exit 1

if test -z "$NOCONFIGURE"
then
  echo "+ $srcdir/configure $*"
  "$srcdir/configure" "$@" || exit 1
  echo
  echo 'Now type "make" to compile quimby.'
fi

exit 0
