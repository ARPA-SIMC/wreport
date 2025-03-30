toplevel_inc = include_directories('..')

executable('wrep', 'options.cc', 'wrep.cc',
       link_with: [libwreport],
       include_directories: toplevel_inc,
       install: true,
)

executable('wrep-make-testjson', 'wrep-make-testjson.cc',
       link_with: [libwreport],
       include_directories: toplevel_inc,
       install: false,
)

install_data('wrep-importtable', install_dir: get_option('bindir'))
