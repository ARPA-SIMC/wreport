Bulletin
========

.. toctree::
    :maxdepth: 2


:cpp:class:`wreport::Bulletin` is the structure encoding a BUFR or CREX weather
report message. It contains data organised in multiple subsets
(:cpp:class:`wreport::Subset`) that share a common structure.

Messages are encoded and decoded using the various read/write/create/decode
functions in the format-specific subclasses of :cpp:class:`wreport::Bulletin`:
:cpp:class:`wreport::BufrBulletin` and :cpp:class:`wreport::CrexBulletin`.


.. doxygenclass:: wreport::Bulletin
   :members:

.. doxygenclass:: wreport::BufrCodecOptions
   :members:

.. doxygenclass:: wreport::BufrBulletin
   :members:

.. doxygenclass:: wreport::CrexBulletin
   :members:

.. doxygenclass:: wreport::Subset
   :members:
