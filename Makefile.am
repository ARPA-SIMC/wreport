## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = wreport tables testdata
if DO_DOCS
SUBDIRS += doc
endif
SUBDIRS += .

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=libwreport.pc

EXTRA_DIST = libwreport.pc.in run-local
