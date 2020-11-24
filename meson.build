project('wreport', 'cpp', version: '3.25', license : 'GPL-2.0-or-later')

conf_data = configuration_data()

# Dependencies
lua_dep = dependency('lua5.2', required: false)
conf_data.set('HAVE_LUA', lua_dep.found())

# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

subdir('wreport')
