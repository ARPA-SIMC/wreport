## Process this file with automake to produce Makefile.in

cdoxdeps = libwreport.dox \
	  ../wreport/libwreport.la

doc_DATA =
noinst_DATA =

if DO_DOCS
doc_DATA += libwreport.doxytags
noinst_DATA += apidocs/index.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)/apidocs
	find apidocs -type f -exec $(INSTALL_DATA) '{}' $(DESTDIR)$(docdir)/apidocs/ \; ;
	$(mkinstalldirs) $(DESTDIR)$(docdir)/examples
	find examples -type f -exec $(INSTALL_DATA) '{}' $(DESTDIR)$(docdir)/examples/ \; ;


uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/apidocs

mainpages = ../wreport/main.dox 

libwreport.doxytags apidocs/index.html: $(cdoxdeps) $(mainpages)
	@test $(DOXYGEN_WARN_IF_UNDOCUMENTED) = NO && \
		echo "No doxygen warnings about undocumented functions. Use ./configure --enable-doxy-undoc-warnings to turn them on"
	doxygen $<
endif

EXTRA_DIST = libwreport.dox examples

CLEANFILES = libwreport.doxytags

clean-local:
	rm -rf apidocs
