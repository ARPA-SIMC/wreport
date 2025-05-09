toplevel_inc = include_directories('..')

extra_warnings = cpp.get_supported_arguments([
  '-Wno-redundant-decls',
])

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
    cpp_args: extra_warnings,
    link_with: libwreport,
    install: true,
    # Meson does not currently autodetect the right target for python modules:
    # https://github.com/mesonbuild/meson/issues/6331
    install_dir: python3.get_path('platlib'),
)

python3.install_sources('wreport/__init__.py', pure: false, subdir: 'wreport')

python_tests = [
    'test-version.py',
    'test-varinfo.py',
    'test-vartable.py',
    'test-var.py',
    'test-wreport.py',
]

runtest = find_program('../runtest')

test('python', runtest, args: ['PYTHON'],
     env: [
         'PYTHON=' + python3_path,
         'PYTHONPATH=' + meson.current_source_dir() + ':' + meson.current_build_dir(),
     ],
     depends:[py_wreport]
)
