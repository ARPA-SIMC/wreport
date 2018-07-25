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

EXTRA_DIST = libwreport.pc.in run-local run-bench testenv TODO-threads \
	     INSTALL BENCHMARKS LICENSE README.md \
	     fedora/README fedora/SPECS/wreport.spec \
	     python/MANIFEST.in python/setup.py fabfile.py

