.. _concepts:

========
Concepts
========

This is a high level explanation of the various parts that compose wreport.

.. _concepts-table:

Table
=====

BUFR and CREX messages are split into a data descriptor section, that describes
the contents of the message, and a data section that contains encoded data.

The data descriptor section contains a sequence of opcodes, that reference
`external tables published by WMO <https://community.wmo.int/site/knowledge-hub/programmes-and-initiatives/wmo-information-system-wis/about-manual-codes-volume-i2/latest-version>`_.

Tables are versioned, and in order to encode and decode messages, wreport needs
access to a database with the tables used by the messages it needs to process.

.. _concepts-vartable:

Vartable
--------

A *Vartable* is a table with BUFR/CREX table B information, that is,
descriptors about how one value is encoded: number of bits or characters,
reference value, scale, unit, description.

.. _concepts-dtable:

Dtable
------

A *D table* is a table with BUFR/CREX table B information, that is, shortcuts
to sequences of codes (BUFR/CREX table B, C or D codes) that drive the encoding
and decoding of a message.

.. _concepts-varinfo:

Varinfo
=======

A *varinfo* is a parsed BUFR/CREX table B entry, that works as the metadata for
one value for a message.

.. _concepts-var:

Var
---

A *var* is a value for a message. It contains a :ref:`concepts-var`, a value, and
zero or more :ref:`concepts-attribute` entries.

.. _concepts-attribute:

Attribute
---------

An *attribute* is a value associated to another value. It is normally a value
from the ``B33yyy`` table section, representing meta-information about a value
such as quality information.

This is modeled in wreport as zero or more :ref:`concepts-var` associated with
another Var. This is not recursive, in that attributes cannot have attributes.

.. _concepts-bulletin:

Bulletin
--------

A *bulletin* is the structure that holds a decoded BUFR or CREX message. It
contains header information, data descriptor information, and one or more
:ref:`concepts-subset` structures with the decoded data, correspoding to BUFR
subsets.

CREX messages do not support subsets, and a corresponding bulletin will always
have one, and only one, subset.

.. _concepts-subset:

Subsets
-------

A *subset* is a sequence of :ref:`concepts-var` that represents one subset in a
BUFR message or the data contents of a CREX message.

The sequence of Var in a subset correspond to the containing bulletin's data
descriptor section.

.. _concepts-interpreter:

Interpreter
-----------

Encoding and decoding of a message happens by using the contents of a data
descriptor sections as opcodes for an interpreter, that produces or consumes
encoded data.

.. _concepts-unit-conversion:

Unit conversion
---------------

wreport supports assignment between :ref:`concepts-var` with different
:ref:`concepts-varinfo`, as long as their units can be converted using a simple
internal unit conversion engine.

