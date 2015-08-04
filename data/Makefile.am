## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps

desktopdir = $(datadir)/applications
desktop_in_files = fpm2.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =                            \
        $(desktop_in_files)

CLEANFILES =                            \
        $(desktop_DATA)
