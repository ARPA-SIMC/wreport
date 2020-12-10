toplevel_inc = include_directories('..')

executable('wrep', 'options.cc', 'wrep.cc',
       link_with: [libwreport],
       include_directories: toplevel_inc,
       install: true,
)

install_data('wrep-importtable', install_dir: get_option('bindir'))
