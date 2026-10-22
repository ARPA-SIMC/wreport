# Manipulating BUFR from Python

This code shows how to read BUFR messages, change their contents and write them
out.

This shows the use of:

* {py:class}`wreport.Bulletin`, that represents a decoded BUFR or CREX message
* iterating a Bulletin to access its subsets as {py:class}`wreport.Subset` instances
* modifying the data descriptor table of a Bulletin
* creating {py:class}`wreport.Var` instances using the bulletin's B table
* modifying the variables in a Subset, prepending new ones
* encoding the modified bulletin

Note that you can only create variables that are known to the B table used by
the bulletin. This means, for example, that it is not possible to add WIGOS
metadata to BUFR messages encoded using master tables earlier than version 28.

:::{literalinclude} /../examples/wigos
    :language: python
