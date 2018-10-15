# WREPORT

[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=centos:7&label=centos7)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:27&label=fedora27)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:28&label=fedora28)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:29&label=fedora29)](https://travis-ci.org/ARPA-SIMC/wreport)
[![Build Status](https://badges.herokuapp.com/travis/ARPA-SIMC/wreport?branch=master&env=DOCKER_IMAGE=fedora:rawhide&label=fedorarawhide)](https://travis-ci.org/ARPA-SIMC/wreport)

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

## Building wreport

You need to install the automake/autoconf/libtool packages, which are usually 
available on any linux distribution, and, in the wreport directory, execute 
the commands:

    autoreconf -if 
    ./configure
    make
    make install

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

wreport is Copyright (C) 2005-2018 ARPAE-SIMC <urpsim@arpae.it>

wreport is licensed under the terms of the GNU General Public License version
2.
