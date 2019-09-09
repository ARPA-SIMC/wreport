## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(top_srcdir)/testenv

#export PYTHONPATH=.

python_tests = test-varinfo.py test-vartable.py test-var.py test-wreport.py

pythonincludedir = $(includedir)/wreport/python
EXTRA_DIST = \
    utils/core.h \
    utils/dict.h \
    utils/methods.h \
    utils/type.h \
    utils/values.h \
    common.h \
    vartable.h \
    varinfo.h \
    var.h

pkgpython_PYTHON = wreport/__init__.py

pyexec_LTLIBRARIES = _wreport.la

_wreport_la_SOURCES = \
    utils/core.cc \
    utils/methods.cc \
    utils/values.cc \
    common.cc \
    varinfo.cc \
    vartable.cc \
    var.cc \
    wreport.cc
_wreport_la_CPPFLAGS = $(PYTHON_CFLAGS)
_wreport_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_wreport
_wreport_la_LIBADD = ../wreport/libwreport.la

if HAVE_SPHINX
noinst_DATA = apidocs

install-data-hook:
	rm -rf "$(DESTDIR)$(docdir)/python"
	install -d "$(DESTDIR)$(docdir)/python"
	tar -C "$(builddir)/apidocs" --exclude='*/.*' -cf - . | tar -C "$(DESTDIR)$(docdir)/python" -xf -
	-chown -R root:root "$(DESTDIR)$(docdir)/python"
	chmod u=rwX,g=rX,o=rX "$(DESTDIR)$(docdir)/python"

apidocs: _wreport.so doc/index.rst doc/wreport.rst
	rm -rf $@
	$(top_srcdir)/run-local $(SPHINX_BUILD) -b html doc $@

endif

_wreport.so: _wreport.la
	ln -fs .libs/_wreport.so $@

clean-local:
	rm -f *.pyc wreport/*.pyc _wreport.so
	rm -rf __pycache__ wreport/__pycache__
	rm -rf apidocs

EXTRA_DIST += testlib.py $(python_tests)

AM_CPPFLAGS = -I/usr/include/python$(PYTHON_VERSION) -I$(top_srcdir)

check-local: _wreport.so
	for test in $(python_tests); do \
		PYTHON="$(PYTHON)" $(TESTS_ENVIRONMENT) $$test ; \
	done
