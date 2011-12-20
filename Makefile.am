## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib wreport src tables testdata
if DO_DOCS
SUBDIRS += doc
endif
SUBDIRS += .

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=libwreport.pc

EXTRA_DIST = m4/gnulib-cache.m4 libwreport.pc.in run-local runtest wreport.spec TODO-threads INSTALL
