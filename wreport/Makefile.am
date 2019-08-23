## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -DTABLE_DIR=\"$(tabledir)\" -I$(top_srcdir) $(LUA_CFLAGS)
if FILE_OFFSET_BITS_64
AM_CPPFLAGS += -D_FILE_OFFSET_BITS=64
endif

#
# Shared library
#

wreportincludedir = $(includedir)/wreport

# Include the .cc files that contain template definitions
nobase_dist_wreportinclude_HEADERS = \
	fwd.h \
	codetables.h \
	conv.h \
	dtable.h \
	error.h \
	notes.h \
	buffers/bufr.h \
	buffers/crex.h \
	bufr/trace.h \
	bufr/input.h \
	bufr/decoder.h \
	bulletin.h \
	bulletin/associated_fields.h \
	bulletin/bitmaps.h \
	bulletin/interpreter.h \
	bulletin/internals.h \
	bulletin/dds-validator.h \
	bulletin/dds-printer.h \
	bulletin/dds-scanfeatures.h \
	opcodes.h \
	options.h \
	subset.h \
	internals/tabledir.h \
	tableinfo.h \
	tables.h \
	var.h \
	varinfo.h \
	vartable.h \
	utils/lua.h \
	utils/string.h \
	utils/sys.h \
	utils/term.h \
	utils/testrunner.h \
	utils/tests.h \
	tests.h \
	benchmark.h
if PYTHON
nobase_dist_wreportinclude_HEADERS += python.h
endif

lib_LTLIBRARIES = libwreport.la
libwreport_la_LIBADD =
libwreport_la_SOURCES = \
	options.cc \
	error.cc \
	notes.cc \
	conv.cc \
	tableinfo.cc \
	varinfo.cc \
	vartable.cc \
	var.cc \
	opcodes.cc \
	dtable.cc \
	tables.cc \
	utils/string.cc \
	utils/sys.cc \
	utils/term.cc \
	utils/tests.cc \
	utils/testrunner.cc \
	internals/tabledir.cc \
	subset.cc \
	buffers/bufr.cc \
	buffers/crex.cc \
	bufr/input.cc \
	bufr/decoder.cc \
	bulletin.cc \
	bulletin/associated_fields.cc \
	bulletin/bitmaps.cc \
	bulletin/interpreter.cc \
	bulletin/internals.cc \
	bulletin/dds-validator.cc \
	bulletin/dds-printer.cc \
	bulletin/dds-scanfeatures.cc \
	bufr_encoder.cc \
	crex_decoder.cc \
	crex_encoder.cc \
	tests.cc \
	benchmark.cc
libwreport_la_LDFLAGS = -version-info @LIBWREPORT_VERSION_INFO@
if LUA
libwreport_la_SOURCES += utils/lua.cc var-lua.cc
libwreport_la_LIBADD += $(LUA_LIBS)
endif

EXTRA_DIST = internals/compat.h main.dox style.dox features.dox examples.dox

#
# Unit testing
#

check_PROGRAMS = test-wreport
TESTS_ENVIRONMENT = $(top_srcdir)/testenv
#TESTS = $(check_PROGRAMS)

check-local:
	for test in $(check_PROGRAMS); do \
		$(TESTS_ENVIRONMENT) $$test ; \
	done

#CXXFLAGS += -O0

dist_noinst_HEADERS = tests.h

test_wreport_SOURCES = \
	options-test.cc \
	error-test.cc \
	conv-test.cc \
	tableinfo-test.cc \
	varinfo-test.cc \
	vartable-test.cc \
	var-test.cc \
	opcodes-test.cc \
	dtable-test.cc \
	tables-test.cc \
	internals/tabledir-test.cc \
	subset-test.cc \
	bulletin-test.cc \
	bufr/input-test.cc \
	bufr/decoder-test.cc \
	bufr_encoder-test.cc \
	crex_decoder-test.cc \
	buffers/bufr-test.cc \
	buffers/crex-test.cc \
	bulletin/associated_fields-test.cc \
	bulletin/bitmaps-test.cc \
	bulletin/interpreter-test.cc \
	bulletin/internals-test.cc \
	bulletin/dds-validator-test.cc \
	tests-test.cc \
	utils/tests-main.cc
test_wreport_LDADD = libwreport.la


if LUA
dist_noinst_HEADERS += test-utils-lua.h
test_wreport_SOURCES += \
	lua-test.cc \
	test-utils-lua.cc
test_wreport_LDADD += $(LUA_LIBS)
endif

#
# Benchmark
#

EXTRA_PROGRAMS = benchmark

dist_noinst_HEADERS += \
	benchmark.h

benchmark_SOURCES = \
	conv-bench.cc \
	var-bench.cc \
	bulletin-bench.cc \
	benchmark-main.cc
benchmark_LDADD = \
	libwreport.la
