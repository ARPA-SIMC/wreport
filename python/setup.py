from setuptools import Extension, setup
import subprocess


def pkg_config_flags(options):
    return [
        s for s in subprocess.check_output(['pkg-config'] + options + ['libwreport']).decode().strip().split(" ") if s
    ]


wreport_module = Extension(
    '_wreport',
    sources=[
        "common.cc", "varinfo.cc", "vartable.cc", "var.cc", "wreport.cc"
    ],
    language="c++",
    extra_compile_args=pkg_config_flags(["--cflags"]) + ["-std=c++11"],
    extra_link_args=pkg_config_flags(["--libs"]),
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
