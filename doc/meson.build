## Process this file with automake to produce Makefile.in

doxygen_doc_sources = ['../wreport/main.dox',  '../wreport/style.dox', '../wreport/features.dox']

sphinx_doc_sources = [
    'src/conf.py',
    'src/index.rst',
    'src/python/wreport.rst',
    'src/cpp/wreport.rst',
    'src/cpp/errors.rst',
    'src/cpp/conv.rst',
    'src/cpp/var.rst',
    'src/cpp/bulletin.rst',
]

# Generate doxygen configuration
libwreport_dox = configure_file(output: 'libwreport.dox', input: 'libwreport.dox.in', configuration: {
    'VERSION': meson.project_version(),
    'top_srcdir': meson.project_source_root(),
    'builddir': meson.current_build_dir(),
    'DOXYGEN_OUTPUT_DIR': meson.current_build_dir(),
})

# Generate documentation
#
# Because of issues like https://github.com/mesonbuild/meson/issues/2320
# we cannot express the chain of what's going on here using meson.
# As a workaround, we build a script that runs doxygen and sphinx, and run it
# in a custom target.

build_docs = configure_file(output: 'build-docs', input: 'build-docs.in', configuration: {
    'doxygen': doxygen.full_path(),
    'sphinx_build': sphinx.full_path(),
    'doc_src': meson.current_source_dir(),
    'doc_build': meson.current_build_dir(),
    'top_builddir': meson.project_build_root(),
    'pythonpath_src': meson.project_source_root() / 'python',
    'pythonpath_build': meson.project_build_root() / 'python',
})

built_docs = custom_target('built_docs',
    input: [libwreport_dox, sphinx_doc_sources, doxygen_doc_sources],
    output: ['libwreport.doxytags', 'xml', 'html'],
    depends: [libwreport, py_wreport],
    command: [build_docs],
    build_by_default: true,
    install_dir: docdir,
    install: true,
)

install_subdir('examples', install_dir: docdir)
