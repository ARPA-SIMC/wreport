toplevel_inc = include_directories('..')

executable('wrep', 'options.cc', 'wrep.cc',
    link_with: [libwreport],
    include_directories: toplevel_inc,
    install: true,
)

wrep_make_testjson = executable('wrep-make-testjson', 'wrep-make-testjson.cc',
    link_with: [libwreport],
    include_directories: toplevel_inc,
    install: false,
)

install_data('wrep-importtable', install_dir: get_option('bindir'))

if python3.found()
    test_decoder = files('test-decoder')
    runtest = find_program('../runtest')

    test('decoder', runtest, args: [test_decoder], depends: [py_wreport, wrep_make_testjson], timeout: 600)
endif
