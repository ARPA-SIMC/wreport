project('wreport', 'cpp', version: '3.25', license : 'GPL-2.0-or-later')

version_array = meson.project_version().split('.')

conf_data = configuration_data()

# Dependencies
lua_dep = dependency('lua5.2', required: false)
conf_data.set('HAVE_LUA', lua_dep.found())

# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

table_dir = get_option('datadir') / 'wreport'

subdir('wreport')
