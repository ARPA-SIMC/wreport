from setuptools import Extension, setup


wreport_module = Extension(
    '_wreport',
    sources=[
        "common.cc", "varinfo.cc", "vartable.cc", "var.cc", "wreport.cc",
    ],
    language="c++",
    extra_compile_args=['-std=c++11'],
    libraries=['wreport', 'lua'],
)

setup(
    name="wreport",
    version="3.3.0",
    author="Enrico Zini",
    author_email="enrico@enricozini.org",
    maintainer="Emanuele Di Giacomo",
    maintainer_email="edigiacomo@arpa.emr.it",
    description="C++ library for working with weather reports",
    long_description="C++ library for working with weather reports",
    url="http://github.com/arpa-simc/wreport",
    license="GPLv2+",
    py_modules=[],
    packages=['wreport'],
    data_files=[],
    zip_safe=False,
    include_package_data=True,
    exclude_package_data={},
    ext_modules=[wreport_module],
)
