## Process this file with automake to produce Makefile.in

INCLUDES = -DTABLE_DIR=\"$(tabledir)\" -I$(top_srcdir) $(LUA_CFLAGS)

#
# Shared library
#

wreportincludedir = $(includedir)/wreport

# Include the .cc files that contain template definitions
nobase_dist_wreportinclude_HEADERS =
	codetables.h \
	conv.h \
	dtable.h \
	error.h \
	notes.h \
	fast.h \
	bulletin.h \
	bulletin/buffers.h \
	bulletin/internals.h \
	bulletin/dds-validator.h \
	bulletin/dds-printer.h \
	opcode.h \
	subset.h \
	var.h \
	varinfo.h \
	vartable.h \
	tests.h

lib_LTLIBRARIES = libwreport.la
libwreport_la_SOURCES = \
	error.cc \
	notes.cc \
	uniconv.cc \
	conv.cc \
	varinfo.cc \
	vartable.cc \
	var.cc \
	opcode.cc \
	dtable.cc \
	subset.cc \
	bulletin.cc \
	bulletin/buffers.cc \
	bulletin/internals.cc \
	bulletin/dds-validator.cc \
	bulletin/dds-printer.cc \
	bufr_decoder.cc \
	bufr_encoder.cc \
	crex_decoder.cc \
	crex_encoder.cc

libwreport_la_LDFLAGS = -version-info @LIBWREPORT_VERSION_INFO@

if LUA
libwreport_la_SOURCES += var-lua.cc
libwreport_la_LDFLAGS += $(LUA_LIBS)
endif

uniconv.cc: uniconv.gperf
	if ! gperf < $< > $@; then rm $@; /bin/false; fi

BUILT_SOURCES = uniconv.cc
EXTRA_DIST = compat.h uniconv.gperf uniconv.cc main.dox style.dox features.dox examples.dox

#
# Unit testing
#

if DO_TESTS

check_PROGRAMS = tut_test
TESTS_ENVIRONMENT = $(top_srcdir)/runtest
TESTS = $(check_PROGRAMS)

CXXFLAGS += -O0

dist_noinst_HEADERS = test-utils-wreport.h

tut_test_SOURCES = \
	test-utils-wreport.cc \
	error-tut.cc \
	conv-tut.cc \
	fast-tut.cc \
	varinfo-tut.cc \
	vartable-tut.cc \
	var-tut.cc \
	opcode-tut.cc \
	dtable-tut.cc \
	bulletin-tut.cc \
	bufr_decoder-tut.cc \
	bufr_encoder-tut.cc \
	crex_decoder-tut.cc \
	bulletin/buffers-tut.cc \
	bulletin/internals-tut.cc \
	bulletin/dds-validator-tut.cc \
	tut-main.cpp
tut_test_LDADD = \
	libwreport.la \
	$(WIBBLE_LIBS)

if LUA
dist_noinst_HEADERS += test-utils-lua.h
tut_test_SOURCES += \
	lua-tut.cc \
	test-utils-lua.cc
tut_test_LDADD += $(LUA_LIBS)
endif


endif
