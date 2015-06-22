## Quimby Input Method, Copyright 2013, Alexandre Muñiz.
## Build files substantially based on Multipress Input Method, Copyright (c) 2007  Openismus GmbH  <http://www.openismus.com/>

# Don't need this at all, I should think. For now at least
# Maybe this should go into $(pkgdatadir) instead, but definitely not into
# $(datadir).  Given that the filesystem where $(pkgdatadir) is located might
# be mounted read-only, $(sysconfdir) seems to be the proper choice.
# conffiledir = $(sysconfdir)
# dist_conffile_DATA = im-quimby.conf

immodulesdir = $(QUIMBY_IMMODULES_DIR)
immodules_LTLIBRARIES = im-quimby.la

im_quimby_la_SOURCES =	\
	im-quimby.c	\
	im-quimby.h	\
	chording.c	\
	chording.h	\
	im-extra-intl.h

# The location of the gettext catalogs as defined by AM_GLIB_GNU_GETTEXT().
mplocaledir = $(prefix)/$(DATADIRNAME)/locale

global_defs = -DQUIMBY_LOCALEDIR=\""$(mplocaledir)"\" -DQUIMBY_CONFDIR=\""$(conffiledir)"\"
AM_CPPFLAGS = $(global_defs) -I$(top_builddir) $(QUIMBY_MODULES_CFLAGS) $(QUIMBY_WARNING_FLAGS)

im_quimby_la_LDFLAGS = -rpath $(immodulesdir) -avoid-version -module $(no_undefined)
im_quimby_la_LIBADD  = $(QUIMBY_MODULES_LIBS) $(INTLLIBS)

# Location of the gtk.immodules file.  You might want to configure with
# --sysconfdir=/etc, in order to register quimby-gtk-input-method
# with the system's gtk.immodules configuration.
#gtk_confdir = $(sysconfdir)/gtk-2.0
#This is a little ugly, but I can't figure out the right way to do this.
gtk_immodules = $(immodulesdir)/../gtk.immodules

# After installation, update the gtk.immodules configuration to include
# quimby-gtk-input-method.  Prepend the module installation prefix
# to GTK_PATH to make sure gtk-query-immodules will find it.
install-data-hook:
	@$(POST_INSTALL)
	@if test -z "$(DESTDIR)"; then \
	  echo " $(install_sh) -d $(gtk_confdir)"; \
	  $(install_sh) -d "$(gtk_confdir)" || exit 1; \
	  dir=`expr "X$(immodulesdir)" : 'X\(.*\)[\\/].*[\\/]'`; \
	  if test -z "$$GTK_PATH"; then GTK_PATH=$$dir; else GTK_PATH=$$dir:$$GTK_PATH; fi; \
	  echo " GTK_PATH=$$GTK_PATH $(GTK_QUERY_IMMODULES) --update-cache"; \
	  export GTK_PATH; $(GTK_QUERY_IMMODULES) --update-cache || exit 1; \
	else :; fi

uninstall-hook:
	@$(POST_UNINSTALL)
	if test -z "$(DESTDIR)"; then \
	  $(GTK_QUERY_IMMODULES) --update-cache; \
	else :; fi
