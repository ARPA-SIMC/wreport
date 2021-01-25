# WREPORT

[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=centos:7&label=centos7)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=centos:8&label=centos8)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:31&label=fedora31)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:32&label=fedora32)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:33&label=fedora33)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:rawhide&label=fedorarawhide)](https://travis-ci.org/ARPA-SIMC/wreport)

[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/wreport/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/wreport/)

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
docker run -it arpaesimc/fedora:31 /bin/bash
docker run -it arpaesimc/centos:8 /bin/bash
```

If you want to build and install wreport yourself, you'll need to install the
automake/autoconf/libtool packages then you can proceed as in most other Unix 
software:

```
autoreconf -if 
./configure
make
make install
```

if you're familiar with .rpm and .deb packaging you'll find the packaging 
files in the `debian` and `fedora` directories

### AFL instrumentation

To run wreport using [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl/):

    CXX=/usr/bin/afl-g++ ./configure --disable-shared
    make AFL_HARDEN=1
    afl-cmin  -i testdata/bufr/ -o afl-bufr -- src/afl-test @@
    afl-fuzz -t 100 -i afl-bufr -o afl-bufr-out src/afl-test @@

## Contact and copyright information

The author of wreport is Enrico Zini <enrico@enricozini.com>

wreport is Copyright (C) 2005-2021 ARPAE-SIMC <urpsim@arpae.it>

wreport is licensed under the terms of the GNU General Public License version
2.
