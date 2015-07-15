#!/usr/bin/python
# coding: utf-8
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import wreport
import unittest
from six import string_types

class Vartable(unittest.TestCase):
    def testEmpty(self):
        with self.assertRaises(TypeError):
            wreport.Vartable()

    def testCreate(self):
        table = wreport.Vartable("dballe")
        self.assertEqual(table.id, "dballe")
        self.assertEqual(str(table), "dballe")
        self.assertEqual(repr(table), "Vartable('dballe')")

    def testContains(self):
        table = wreport.Vartable("dballe")
        self.assertIn("B01001", table)
        self.assertNotIn("B63254", table)

    def testIndexing(self):
        table = wreport.Vartable("dballe")
        info = table[0]
        self.assertEqual(info.var, "B01001")

    def testQuery(self):
        table = wreport.Vartable("dballe")
        info = table.query("B01001")
        self.assertEqual(info.is_string, False)
        self.assertEqual(info.len, 3)
        self.assertEqual(info.unit, "NUMERIC")

        info = table["B01001"]
        self.assertEqual(info.is_string, False)
        self.assertEqual(info.len, 3)
        self.assertEqual(info.unit, "NUMERIC")

    def testQueryMissing(self):
        table = wreport.Vartable("dballe")
        self.assertRaises(KeyError, table.query, "B63254")

    def testIterate(self):
        table = wreport.Vartable("dballe")
        selected = None
        count = 0
        for entry in table:
            if entry.var == "B12101":
                selected = entry
            count += 1
        self.assertGreater(count, 100)
        self.assertEqual(count, len(table))
        self.assertIsNotNone(selected)


if __name__ == "__main__":
    from testlib import main
    main("test_vartable")
