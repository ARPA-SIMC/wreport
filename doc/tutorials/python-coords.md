# Readomg BUFR from Python

This code shows how to read BUFR messages and examine the data they contain.

This shows the use of:

* {py:class}`wreport.Bulletin`, that represents a decoded BUFR or CREX message
* iterating a Bulletin to access its subsets as {py:class}`wreport.Subset` instances
* iterating the subset variables as {py:class}`wreport.Var` instances

:::{literalinclude} /../examples/coords
    :language: python
