## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = wreport src tables testdata
if DO_DOCS
SUBDIRS += doc
endif
if PYTHON
SUBDIRS += python
endif
SUBDIRS += .

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libwreport.pc

if WIBBLE
pkgconfig_DATA += libwreport-test.pc
endif

EXTRA_DIST = libwreport.pc.in libwreport-test.pc.in run-local run-bench testenv wreport.spec TODO-threads INSTALL
