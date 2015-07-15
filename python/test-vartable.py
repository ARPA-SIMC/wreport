#!/usr/bin/python
# coding: utf-8
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import wreport
import unittest
import os

class Vartable(unittest.TestCase):
    def testEmpty(self):
        with self.assertRaises(NotImplementedError):
            wreport.Vartable()

    def testLoadBufr(self):
        t = wreport.Vartable.load_bufr(os.path.join(os.environ["WREPORT_TABLES"], "B0000000000000024000.txt"))
        self.assertEqual(t["B12101"].unit, "K");

    def testLoadCrex(self):
        t = wreport.Vartable.load_crex(os.path.join(os.environ["WREPORT_TABLES"], "B0000000000000024000.txt"))
        self.assertEqual(t["B12101"].unit, "C");

    def testGetBufr(self):
        t = wreport.Vartable.get_bufr(basename="B0000000000000024000");
        self.assertEqual(t["B12101"].unit, "K");

        t = wreport.Vartable.get_bufr(master_table_version_number=24)
        self.assertEqual(t["B12101"].unit, "K");

    def testGetCrex(self):
        t = wreport.Vartable.get_crex(basename="B0000000000000024000");
        self.assertEqual(t["B12101"].unit, "C");

        t = wreport.Vartable.get_crex(master_table_version_number_bufr=24)
        self.assertEqual(t["B12101"].unit, "C");

    def testCreate(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        self.assertEqual(os.path.basename(table.pathname), "B0000000000000024000.txt")
        self.assertEqual(os.path.basename(str(table)), "B0000000000000024000.txt")
        expected = "Vartable('{}')".format(table.pathname)
        self.assertEqual(repr(table), expected)

    def testContains(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        self.assertIn("B01001", table)
        self.assertNotIn("B63254", table)

#    def testIndexing(self):
#        table = wreport.Vartable("dballe")
#        info = table[0]
#        self.assertEqual(info.var, "B01001")

    def testLookup(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        info = table["B01001"]
        self.assertEqual(info.type, "integer")
        self.assertEqual(info.len, 3)
        self.assertEqual(info.unit, "NUMERIC")

    def testLookupMissing(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        with self.assertRaises(KeyError):
            table["B63254"]

#    def testIterate(self):
#        table = wreport.Vartable("dballe")
#        selected = None
#        count = 0
#        for entry in table:
#            if entry.var == "B12101":
#                selected = entry
#            count += 1
#        self.assertGreater(count, 100)
#        self.assertEqual(count, len(table))
#        self.assertIsNotNone(selected)


if __name__ == "__main__":
    from testlib import main
    main("vartable")
