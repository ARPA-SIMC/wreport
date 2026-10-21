"""
wreport provides access to weather data in BUFR and CREX formats.

This is the Python interface to it
"""

from _wreport import (
    __version__,
    convert_units,
    Var,
    Varinfo,
    Vartable,
    RawBufrIterator,
    RawCrexIterator,
    Bulletin,
)

read_bufr = RawBufrIterator
read_crex = RawCrexIterator

__all__ = (
    "convert_units",
    "Var",
    "Varinfo",
    "Vartable",
    "RawBufrIterator",
    "RawCrexIterator",
    "Bulletin",
    "read_bufr",
    "read_crex",
)
