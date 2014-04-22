## copy comment header here.

ACLOCAL_AMFLAGS = -I macros $(ACLOCAL_FLAGS)
DISTCHECK_CONFIGURE_FLAGS = --enable-warnings=max

SUBDIRS = po src

dist_noinst_DATA =		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	debian/changelog	\
	debian/compat		\
	debian/control		\
	debian/copyright	\
	debian/docs		\
	debian/quimby-gtk-immodule.install	\
	debian/quimby-gtk-immodule.postinst	\
	debian/quimby-gtk-immodule.postrm

dist_noinst_SCRIPTS = autogen.sh debian/rules

DISTCLEANFILES = intltool-extract intltool-merge intltool-update
