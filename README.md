[![Build Status](https://simc.arpae.it/moncic-ci/wreport/rocky8.png)](https://simc.arpae.it/moncic-ci/wreport/)
[![Build Status](https://simc.arpae.it/moncic-ci/wreport/rocky9.png)](https://simc.arpae.it/moncic-ci/wreport/)
[![Build Status](https://simc.arpae.it/moncic-ci/wreport/fedora38.png)](https://simc.arpae.it/moncic-ci/wreport/)
[![Build Status](https://simc.arpae.it/moncic-ci/wreport/fedora40.png)](https://simc.arpae.it/moncic-ci/wreport/)
[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/wreport/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/wreport/)

# WREPORT


## Introduction

wreport is a C++ library for working with weather reports.

wreport is a powerful decoder and encoder for the `BUFR` and `CREX` formats.

It also provides a useful abstraction to handle values found in weather
reports, with awareness of significant digits, measurement units, variable
descriptions, unit conversion and attributes on variables.

Features provided:

- Read and write `BUFR` version 2, 3, and 4
- Read and write `CREX`
- Unit conversion
- Handling of physical variables

[C++ and Python API documentation](https://arpa-simc.github.io/wreport/).

## Installing wreport

wreport is already packaged in both .rpm and .deb formats, and that provides
easy installation for most Linux distributions.

For CentOS and Fedora, rpm files are hosted in a copr repo:
https://copr.fedorainfracloud.org/coprs/simc/stable/

For Debian, wreport is available in the testing distribution:
https://packages.debian.org/testing/libwreport3

Using docker images with wreport preinstalled is also possible:

```
docker run -it arpaesimc/fedora:36 /bin/bash
docker run -it arpaesimc/centos:8 /bin/bash
```

If you want to build and install wreport yourself, you'll need to install
[Meson](https://mesonbuild.com/) and run the following commands:

```
meson setup builddir && cd builddir
meson compile
meson test
meson install
```

For more details about Meson see the official documentation https://mesonbuild.com/.

If you want to build the package yourself:
- rpm: the packaging files are in `fedora` directory of the `master` branch.
- deb: the packaging files are in the debian branches (e.g. `debian/sid`, `ubuntu/jammy`, etc.)

## Environment variables


These environment variables can be used to control wreport's behaviour at runtime:

* `WREPORT_TABLES`: Table directory to search before the builtin one
* `WREPORT_EXTRA_TABLES`: Extra table directory to search before
  `WREPORT_TABLES` or the builtin one


## AFL instrumentation

To run wreport using [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl/):

    CXX=/usr/bin/afl-g++ meson --default-library=static builddir && cd builddir
    AFL_HARDEN=1 meson compile
    afl-cmin  -i testdata/bufr/ -o afl-bufr -- src/afl-test @@
    afl-fuzz -t 100 -i afl-bufr -o afl-bufr-out src/afl-test @@

## Contact and copyright information

The author of wreport is Enrico Zini <enrico@enricozini.com>

wreport is Copyright (C) 2005-2024 ARPAE-SIMC <urpsim@arpae.it>

wreport is licensed under the terms of the GNU General Public License version
2.
