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
    BufrIterator,
    CrexIterator,
    BufrHeaderIterator,
    CrexHeaderIterator,
    Bulletin,
)

read_bufr_raw = RawBufrIterator
read_crex_raw = RawCrexIterator
read_bufr = BufrIterator
read_crex = CrexIterator
read_bufr_headers = BufrHeaderIterator
read_crex_headers = CrexHeaderIterator

__all__ = (
    "convert_units",
    "Var",
    "Varinfo",
    "Vartable",
    "RawBufrIterator",
    "RawCrexIterator",
    "BufrIterator",
    "CrexIterator",
    "BufrHeaderIterator",
    "CrexHeaderIterator",
    "Bulletin",
    "read_bufr_raw",
    "read_crex_raw",
    "read_bufr",
    "read_crex",
    "read_bufr_headers",
    "read_crex_headers",
)
