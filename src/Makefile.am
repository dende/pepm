## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_PIXMAP_DIR=\""$(pixmapdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = pepm

pepm_SOURCES = \
	main.c \
	fpm.c fpm.h \
	support.c support.h \
	interface.c interface.h \
	fpm_file.c fpm_file.h \
	callbacks.c callbacks.h \
	blowfish.c gpg_trans.h \
	fpm_crypt.c fpm_crypt.h \
	fpm_gpw.c fpm_gpw.h \
	fpm_clist.c \
	bithelp.h fpm_launcher.c \
	rijndael.c sha256.c

pepm_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

