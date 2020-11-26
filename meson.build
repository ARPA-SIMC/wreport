project('wreport', 'cpp', version: '3.27', license : 'GPL-2.0-or-later')

version_array = meson.project_version().split('.')
libwreport_so_version = '3.0.3'

table_dir = get_option('datadir') / 'wreport'

conf_data = configuration_data()
conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
conf_data.set_quoted('TABLE_DIR', get_option('prefix') / table_dir)

# Dependencies
lua_dep = dependency('lua5.2', required: false)
conf_data.set('HAVE_LUA', lua_dep.found())

# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

pymod = import('python')
python3 = pymod.find_installation('python3', required: false)

if python3.found()
    # FIXME: python3.path() is only available from meson 0.50: this is a workaround
    python3_path = python3.get_variable('BINDIR') / python3.get_variable('PYTHON') + python3.language_version()

    sphinx = find_program('sphinx-build', 'sphinx-build-3', 'sphinx-build-' + python3.language_version(), required: false)

    # TODO: check if breathe is installed
    # TODO AX_PYTHON_MODULE(breathe)

    doxygen = find_program('doxygen', required : false)

    build_docs = sphinx.found() and doxygen.found()
    docdir = get_option('datadir') / 'doc' / meson.project_name()
else
    warning('Documentation disabled, requires doxygen, sphinx, and the breathe python module')
    build_docs = false
endif

subdir('wreport')

if python3.found()
    subdir('python')
endif

if build_docs
    subdir('doc')
endif

subdir('tables')

# Generate pkg-config metadata
pkg = import('pkgconfig')
pkg.generate(libwreport,
    description: 'Weather report library',
    filebase: 'libwreport',
    requires: [lua_dep],
)
