## Process this file with automake to produce Makefile.in

INCLUDES = -DTABLE_DIR=\"$(tabledir)\" -I$(top_srcdir)

WREPLIBS =  ../wreport/libwreport.la

dist_noinst_HEADERS = options.h

bin_PROGRAMS = wrep

wrep_SOURCES = wrep.cc
wrep_LDFLAGS = $(WREPLIBS)
wrep_DEPENDENCIES = $(WREPLIBS)
