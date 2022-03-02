#!/usr/bin/python3
import wreport
import unittest


class Varinfo(unittest.TestCase):
    def testEmptyVarinfo(self):
        self.assertRaises(NotImplementedError, wreport.Varinfo)

    def testData(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        info = table["B01001"]
        self.assertEqual(info.code, "B01001")
        self.assertEqual(info.desc, "WMO block number")
        self.assertEqual(info.unit, "NUMERIC")
        self.assertEqual(info.scale, 0)
        self.assertEqual(info.len, 3)
        self.assertEqual(info.bit_ref, 0)
        self.assertEqual(info.bit_len, 7)
        self.assertEqual(info.type, "integer")

    def testStringification(self):
        table = wreport.Vartable.get_bufr(master_table_version_number=24)
        info = table["B01001"]
        self.assertTrue(str(info).startswith("B01001"))
        self.assertTrue(repr(info).startswith("Varinfo('B01001"))
