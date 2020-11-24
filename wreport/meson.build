libwreport_sources = [
	'options.cc',
	'error.cc',
	'notes.cc',
	'conv.cc',
	'tableinfo.cc',
	'varinfo.cc',
	'vartable.cc',
	'var.cc',
	'opcodes.cc',
	'dtable.cc',
	'tables.cc',
	'utils/string.cc',
	'utils/sys.cc',
	'utils/subprocess.cc',
	'utils/term.cc',
	'utils/tests.cc',
	'utils/testrunner.cc',
	'internals/tabledir.cc',
	'subset.cc',
	'buffers/bufr.cc',
	'buffers/crex.cc',
	'bufr/input.cc',
	'bufr/decoder.cc',
	'bulletin.cc',
	'bulletin/associated_fields.cc',
	'bulletin/bitmaps.cc',
	'bulletin/interpreter.cc',
	'bulletin/internals.cc',
	'bulletin/dds-validator.cc',
	'bulletin/dds-printer.cc',
	'bulletin/dds-scanfeatures.cc',
	'bufr_encoder.cc',
	'crex_decoder.cc',
	'crex_encoder.cc',
	'tests.cc',
	'benchmark.cc',
]

if lua_dep.found()
libwreport_sources += [
	'utils/lua.cc',
	'var-lua.cc',
]
endif

# libwreport_la_LDFLAGS = -version-info @LIBWREPORT_VERSION_INFO@

toplevel_inc = include_directories('..')

libwreport = both_libraries('wreport',
        libwreport_sources,
        install: true,
        include_directories: toplevel_inc,
	# TODO: set the right table_dir
        cpp_args: '-DTABLE_DIR="."',
        dependencies: [
                lua_dep,
        ])


#
# Unit testing
#

test_wreport_sources = [
	'options-test.cc',
	'error-test.cc',
	'conv-test.cc',
	'tableinfo-test.cc',
	'varinfo-test.cc',
	'vartable-test.cc',
	'var-test.cc',
	'opcodes-test.cc',
	'dtable-test.cc',
	'tables-test.cc',
	'internals/tabledir-test.cc',
	'subset-test.cc',
	'bulletin-test.cc',
	'bufr/input-test.cc',
	'bufr/decoder-test.cc',
	'bufr_encoder-test.cc',
	'crex_decoder-test.cc',
	'buffers/bufr-test.cc',
	'buffers/crex-test.cc',
	'bulletin/associated_fields-test.cc',
	'bulletin/bitmaps-test.cc',
	'bulletin/interpreter-test.cc',
	'bulletin/internals-test.cc',
	'bulletin/dds-validator-test.cc',
	'tests-test.cc',
	'utils/tests-main.cc',
]

if lua_dep.found()
test_wreport_sources += [
	'lua-test.cc',
	'test-utils-lua.cc',
]
endif

test_wreport = executable('test-wreport', test_wreport_sources,
        include_directories: toplevel_inc,
	# TODO: set the right table_dir
        cpp_args: '-DTABLE_DIR="."',
        link_with: [
                libwreport,
        ],
        dependencies: [
		lua_dep,
        ])

testenv = find_program('../testenv')

test('wreport', testenv, args: [test_wreport])
