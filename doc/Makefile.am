## Process this file with automake to produce Makefile.in

cdoxdeps = libwreport.dox \
	  ../wreport/libwreport.la

sphinx_doc_sources = \
    src/conf.py \
    src/index.rst \
    src/python/wreport.rst \
    src/cpp/wreport.rst \
    src/cpp/errors.rst \
    src/cpp/conv.rst \
    src/cpp/var.rst \
    src/cpp/bulletin.rst

doc_DATA =
noinst_DATA =

if DO_DOCS
doc_DATA += libwreport.doxytags
noinst_DATA += html/index.html

# See https://stackoverflow.com/questions/19822435/multiple-targets-from-one-recipe-and-parallel-execution#19822767
.INTERMEDIATE: build-docs

build-docs: $(cdoxdeps) $(sphinx_doc_sources) $(mainpages)
	rm -rf html
	mkdir html
	@test $(DOXYGEN_WARN_IF_UNDOCUMENTED) = NO && \
		echo "No doxygen warnings about undocumented functions. Use ./configure --enable-doxy-undoc-warnings to turn them on" \
		|| true
	$(DOXYGEN) $<
	top_builddir=$(abs_top_builddir) $(top_srcdir)/run-local $(SPHINX_BUILD) -b html src html/

libwreport.doxytags: build-docs
html/index.html: build-docs

install-data-local:
	find html -type d -exec $(mkinstalldirs) '$(DESTDIR)$(docdir)/{}'  \; ;
	find html -type f -not -path '*/.*' -exec $(INSTALL_DATA) '{}' '$(DESTDIR)$(docdir)/{}' \; ;
	$(mkinstalldirs) $(DESTDIR)$(docdir)/examples
	find examples -type f -exec $(INSTALL_DATA) '{}' $(DESTDIR)$(docdir)/examples/ \; ;

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/html
	rm -rf $(DESTDIR)$(docdir)/examples

mainpages = ../wreport/main.dox  ../wreport/style.dox ../wreport/features.dox

endif

EXTRA_DIST = libwreport.dox examples 20160914-american-fuzzy-lop.txt

CLEANFILES = libwreport.doxytags

clean-local:
	rm -rf xml html
