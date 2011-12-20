## Process this file with automake to produce Makefile.in

INCLUDES = -DTABLE_DIR=\"$(tabledir)\" -I$(top_srcdir)

WREPLIBS =  ../wreport/libwreport.la

dist_noinst_HEADERS = options.h info.cc input.cc output.cc iterate.cc makebuoy.cc

bin_PROGRAMS = wrep
noinst_PROGRAMS = examples
dist_bin_SCRIPTS = wrep-importtable

wrep_SOURCES = options.cc wrep.cc
wrep_LDFLAGS = $(WREPLIBS)
wrep_DEPENDENCIES = $(WREPLIBS)

examples_SOURCES = examples.cc
examples_LDFLAGS = $(WREPLIBS)
examples_DEPENDENCIES = $(WREPLIBS)

