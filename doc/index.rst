.. wreport documentation master file, created by
   sphinx-quickstart on Thu Sep  5 20:14:01 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

**wreport** documentation
=========================

Welcome to wreport's documentation. The documentation follows the `Diataxis
<https://diataxis.fr/>`_ structure of tutorials, how-to guides, explanations
and reference.

Introduction
------------

``wreport`` is a library for parsing weather information encoded in
`BUFR <https://en.wikipedia.org/wiki/BUFR>`_ and
`CREX <https://community.wmo.int/site/knowledge-hub/programmes-and-initiatives/wmo-information-system-wis/about-manual-codes-volume-i3/bufr-edition-3-and-crex-edition-1>`_
formats.

``wreport`` is designed to be the encoding and decoding library for code that
processes a wide range of BUFR and CREX messages, focusing on providing access
to their structure and data as a whole.

To extract specific, well-known information from BUFR and CREX files you most
likely want to use `ecCodes <https://github.com/ecmwf/eccodes>`_ instead.

If you instead need to do more generic transformations like `converting BUFR to
similarly-structured NetCDF <https://github.com/ARPA-SIMC/bufr2netcdf>`_, or
convert messages between different data description templates, wreport may be
an ideal backend.


Documentation index
===================

.. toctree::
   :maxdepth: 2
   :caption: Tutorials

   tutorials/tutorial-template
   tutorials/cpp-read-files
   tutorials/python-wigos
   tutorials/python-coords

.. toctree::
   :maxdepth: 2
   :caption: Howtos

.. toctree::
   :maxdepth: 2
   :caption: Explanations

   explanations/concepts

.. toctree::
   :maxdepth: 2
   :caption: References

   references/python/wreport
   references/cpp/wreport

.. toctree::
   :maxdepth: 2
   :caption: Index

   genindex
   search
