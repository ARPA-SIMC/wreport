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
    include_directories: toplevel_inc,
    dependencies: python3.dependency(),
    link_with: libwreport,
    install: true,
    subdir: 'wreport',
)

python3.install_sources('wreport/__init__.py', pure: false, subdir: 'wreport')

python_tests = [
    'test-version.py',
    'test-varinfo.py',
    'test-vartable.py',
    'test-var.py',
    'test-wreport.py',
]

testenv = find_program('../testenv')

foreach t: python_tests
    # TODO: python3.path() is only available from meson 0.50: this is a workaround
    python3_path = python3.get_variable('BINDIR') / python3.get_variable('PYTHON') + python3.get_variable('VERSION')
    test(t, testenv, args: [files(t)], env: [ 'PYTHON=' + python3_path ])
endforeach
