project('wreport', 'cpp', version: '3.40', license : 'GPL-2.0-or-later', default_options: ['warning_level=3', 'cpp_std=c++17'], meson_version: '>= 0.58')
# TODO: use warning_level=everything in newer meson

cpp = meson.get_compiler('cpp')

warning_control = [
  # Turn some warning classes to errors
  '-Werror=format',
  '-Werror=suggest-override',
  '-Werror=deprecated-copy-dtor',
  '-Werror=missing-declarations',
  '-Werror=overloaded-virtual',
  '-Werror=cast-qual',
  '-Werror=duplicated-branches',
  '-Werror=logical-op',
  '-Werror=catch-value',
  '-Werror=conditionally-supported',
  '-Werror=noexcept',
  '-Werror=c++23-extensions',
  '-Werror=dangling-else',
  '-Werror=suggest-attribute=format',
  '-Werror=deprecated-declarations',
  # '-Werror=cast-align',

  '-Wno-padded',
  '-Wno-abi-tag',
  '-Wno-unused-macros',
  '-Wno-sign-promo',
  '-Wswitch',
  '-Wno-switch-enum',
  '-Wno-effc++',

  # TODO: remove the following ones over time
  '-Wno-shadow',
  '-Wno-zero-as-null-pointer-constant',
  '-Wno-mismatched-tags',
  '-Wno-unused-const-variable',
  '-Wno-redundant-tags',
  '-Wno-useless-cast',
  '-Wno-switch-default',
  '-Wno-old-style-cast',
  '-Wno-unused-parameter',

  # These ones can be activated from time to time
  '-Wno-float-equal',
  '-Wno-suggest-attribute=noreturn',
  '-Wno-format-truncation',
  '-Wno-arith-conversion',
  '-Wno-conversion',
]

add_project_arguments(
  cpp.get_supported_arguments(warning_control),
  language : 'cpp')

version_array = meson.project_version().split('.')
libwreport_so_version = '3.1.0'

table_dir = get_option('datadir') / 'wreport'

conf_data = configuration_data()
conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
conf_data.set_quoted('TABLE_DIR', get_option('prefix') / table_dir)

# Dependencies
foreach name : ['lua', 'lua5.1', 'lua5.2', 'lua5.3']
    lua_dep = dependency(name, version: '>=5.1.1', required: false)
    if lua_dep.found()
        break
    endif
endforeach
if not lua_dep.found()
    error('Lua could not be found!')
endif
conf_data.set('HAVE_LUA', lua_dep.found())

compiler = meson.get_compiler('cpp')
if compiler.has_function('getopt_long')
    conf_data.set('HAS_GETOPT_LONG', true)
endif

# std::filesystem library needs to be explicitly linked with g++ < 9.0
if cpp.find_library('stdc++fs').found()
  # without this, std::filesystem is not present for some compilers
  add_project_link_arguments(['-lstdc++fs'], language : 'cpp')
endif

# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

pymod = import('python')
python3 = disabler()
if get_option('python')
    python3 = pymod.find_installation('python3', required: false)
endif

if python3.found()
    # FIXME: python3.path() is only available from meson 0.50: this is a workaround
    python3_path = python3.get_variable('BINDIR') / python3.get_variable('PYTHON') + python3.language_version()

    sphinx = find_program('sphinx-build', 'sphinx-build-3', 'sphinx-build-' + python3.language_version(), required: false)

    # TODO: check if breathe is installed
    # TODO AX_PYTHON_MODULE(breathe)
    # See https://gitlab.freedesktop.org/wayland/weston/-/blob/master/doc/sphinx/meson.build

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

subdir('src')

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
#
# Generate the builddir's version of run-local
run_local_cfg = configure_file(output: 'run-local', input: 'run-local.in', configuration: {
    'top_srcdir': meson.project_source_root(),
    'top_builddir': meson.project_build_root(),
})
