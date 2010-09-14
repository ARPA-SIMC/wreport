## Process this file with automake to produce Makefile.in

cdoxdeps = libwreport.dox \
	  ../wreport/libwreport.la \
	  ../wreport/aliases.dox

doc_DATA =
noinst_DATA =

if DO_DOCS
doc_DATA += libwreport.doxytags
noinst_DATA += apidocs/index.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)/apidocs
	find apidocs -type f -exec $(INSTALL_DATA) '{}' $(DESTDIR)$(docdir)/apidocs/ \; ;

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/apidocs

mainpages = ../wreport/main.dox 

libwreport.doxytags apidocs/index.html: $(cdoxdeps) $(mainpages)
	doxygen $<
endif

EXTRA_DIST = libwreport.dox

CLEANFILES = libwreport.doxytags

clean-local:
	rm -rf apidocs
