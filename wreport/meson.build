# Generate header with version information
configure_file(output: 'version.h', configuration: {
        'WREPORT_VERSION_MAJOR': version_array[0],
        'WREPORT_VERSION_MINOR': version_array[1],
}, install_dir: join_paths(get_option('includedir'), 'wreport'))

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
        version: libwreport_so_version,
        install: true,
        include_directories: toplevel_inc,
        dependencies: [
                lua_dep,
        ])

install_headers(
        'fwd.h',
        'codetables.h',
        'conv.h',
        'dtable.h',
        'error.h',
        'notes.h',
        'bulletin.h',
        'opcodes.h',
        'options.h',
        'subset.h',
        'tableinfo.h',
        'tables.h',
        'var.h',
        'varinfo.h',
        'vartable.h',
        'tests.h',
        'benchmark.h',
        'python.h',
        subdir: 'wreport',
)

install_headers(
        'buffers/bufr.h',
        'buffers/crex.h',
        subdir: 'wreport/buffers',
)

install_headers(
        'bufr/trace.h',
        'bufr/input.h',
        'bufr/decoder.h',
        subdir: 'wreport/bufr',
)

install_headers(
        'bulletin/associated_fields.h',
        'bulletin/bitmaps.h',
        'bulletin/interpreter.h',
        'bulletin/internals.h',
        'bulletin/dds-validator.h',
        'bulletin/dds-printer.h',
        'bulletin/dds-scanfeatures.h',
        subdir: 'wreport/bulletin',
)

install_headers(
        'internals/tabledir.h',
        subdir: 'wreport/internals',
)

install_headers(
        'utils/lua.h',
        'utils/string.h',
        'utils/sys.h',
        'utils/subprocess.h',
        'utils/term.h',
        'utils/testrunner.h',
        'utils/tests.h',
        subdir: 'wreport/utils',
)


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
        link_with: [
                libwreport,
        ],
        dependencies: [
                lua_dep,
        ])

testenv = find_program('../testenv')

test('wreport', testenv, args: [test_wreport])
