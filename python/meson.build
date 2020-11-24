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
    test(t, testenv, args: [files(t)], env: [
        'PYTHON=' + python3_path,
        'PYTHONPATH=' + meson.current_build_dir(),
    ])
endforeach
