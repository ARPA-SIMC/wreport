Error handling
==============

.. toctree::
    :maxdepth: 2


Error management
----------------

wreport defines its own exception hierarchy, rooted at :class:`wreport::error`.

Each wreport exception has a numeric identification code:

.. doxygenenum:: wreport::ErrorCode

The exceptions are:

.. doxygenclass:: wreport::error
   :members:

.. doxygenclass:: wreport::error_alloc

.. doxygenclass:: wreport::error_notfound

.. doxygenclass:: wreport::error_type

.. doxygenclass:: wreport::error_handles

.. doxygenclass:: wreport::error_toolong

.. doxygenclass:: wreport::error_system

.. doxygenclass:: wreport::error_consistency

.. doxygenclass:: wreport::error_parse

.. doxygenclass:: wreport::error_regexp

.. doxygenclass:: wreport::error_unimplemented

.. doxygenclass:: wreport::error_domain

