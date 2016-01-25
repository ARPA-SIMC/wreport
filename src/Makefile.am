## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -DTABLE_DIR=\"$(tabledir)\" -I$(top_srcdir)
if FILE_OFFSET_BITS_64
AM_CPPFLAGS += -D_FILE_OFFSET_BITS=64
endif

WREPLIBS =  ../wreport/libwreport.la

dist_noinst_HEADERS = options.h info.cc input.cc output.cc iterate.cc makebuoy.cc unparsable.cc

bin_PROGRAMS = wrep
noinst_PROGRAMS = examples
dist_bin_SCRIPTS = wrep-importtable

wrep_SOURCES = options.cc wrep.cc
wrep_LDFLAGS = $(WREPLIBS)
wrep_DEPENDENCIES = $(WREPLIBS)

examples_SOURCES = examples.cc
examples_LDFLAGS = $(WREPLIBS)
examples_DEPENDENCIES = $(WREPLIBS)

