## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(top_srcdir)/testenv

#export PYTHONPATH=.

python_tests = test-varinfo.py test-vartable.py test-var.py

pythonincludedir = $(includedir)/wreport/python
nobase_dist_pythoninclude_HEADERS = \
    doc-wreport \
    common.h \
    vartable.h \
    varinfo.h \
    var.h

pkgpython_PYTHON = wreport/__init__.py

pyexec_LTLIBRARIES = _wreport.la

_wreport_la_SOURCES = \
    common.cc \
    varinfo.cc \
    vartable.cc \
    var.cc \
    wreport.cc
_wreport_la_CPPFLAGS = $(PYTHON_CFLAGS)
_wreport_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_wreport
_wreport_la_LIBADD = ../wreport/libwreport.la

doc_DATA = python-wreport.rst

if HAVE_RST2HTML
doc_DATA += python-wreport.html
endif

_wreport.so: _wreport.la
	ln -fs .libs/_wreport.so $@

python-wreport.rst: doc-wreport $(pkgpython_PYTHON) _wreport.so
	$(PYTHON) $(srcdir)/doc-wreport > $@ || (rm -f $@; false)

python-wreport.html: python-wreport.rst
	rst2html $< > $@ || (rm -f $@; false)

clean-local:
	rm -f *.pyc wreport/*.pyc _wreport.so

EXTRA_DIST = doc-wreport testlib.py $(python_tests)

AM_CPPFLAGS = -I/usr/include/python$(PYTHON_VERSION) -I$(top_srcdir)

check-local: _wreport.so
	for test in $(python_tests); do \
		PYTHON="$(PYTHON)" $(TESTS_ENVIRONMENT) $$test ; \
	done

CLEANFILES = python-wreport.rst python-wreport.html
