toplevel_inc = include_directories('..')

py_wreport = python3.extension_module('_wreport',
    'utils/core.cc',
    'utils/methods.cc',
    'utils/values.cc',
    'common.cc',
    'varinfo.cc',
    'vartable.cc',
    'var.cc',
    'wreport.cc',
    install: true,
    include_directories: toplevel_inc,
    dependencies: python3.dependency(),
    link_with: libwreport,
)
