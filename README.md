WREPORT
===============================================================

Introduction
------------

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

Building wreport
----------------

you need to install the automake/autoconf/libtool packages, which are usually 
available on any linux distribution, and, in the wreport directory, execute 
the commands:

  autoreconf -if 
  ./configure
  make
  make install

if you're familiar with .rpm and .deb packaging you'll find the packaging 
files in the `debian` and `fedora` directories

Contact and copyright information
---------------------------------

The author of wreport is Enrico Zini <enrico@enricozini.com>

wreport is Copyright (C) 2005-2015 ARPA-SIMC <urpsim@arpa.emr.it>

wreport is licensed under the terms of the GNU General Public License version
2.
