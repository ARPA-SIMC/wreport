Variables
=========

.. toctree::
    :maxdepth: 2


Values in wreport are represented as *variables* (:cpp:class:`wreport::Var`),
which contain the value annotated with metadata (:cpp:type:`wreport::Varcode`
and :cpp:type:`wreport::Varinfo`) about its measurement unit, description and
encoding/decoding information.

Variables are described in BUFR/CREX *B* tables
(:cpp:class:`wreport::Vartable`), defined by WMO and shipped with the wreport
library.


Variable metadata
-----------------

Each variable type is represented by a numeric WMO B table variable code:

.. doxygentypedef:: wreport::Varcode

There are a few accessor macros for :cpp:type:`wreport::Varcode`:

.. doxygendefine:: WR_VAR
.. doxygendefine:: WR_STRING_TO_VAR
.. doxygendefine:: WR_VAR_F
.. doxygendefine:: WR_VAR_X
.. doxygendefine:: WR_VAR_Y
.. doxygendefine:: WR_VAR_FXY

There are functions for parsing/formatting :cpp:type:`wreport::Varcode` values:

.. doxygenfunction:: varcode_format
.. doxygenfunction:: varcode_parse

Detailed information on a variable are accessed via :cpp:type:`wreport::Varinfo`:

.. doxygentypedef:: wreport::Varinfo

which is a pointer to a :cpp:class:`wreport::_Varinfo`:

.. doxygenstruct:: wreport::_Varinfo
   :members:


Variable
--------

.. doxygenclass:: wreport::Var
   :members:


Variable metadata tables
------------------------

.. doxygenclass:: wreport::Vartable
   :members:
