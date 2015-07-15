#!/usr/bin/python
# coding: utf-8
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import wreport
import unittest
from six import string_types

class Varinfo(unittest.TestCase):
    def testEmptyVarinfo(self):
        self.assertRaises(NotImplementedError, wreport.Varinfo)

    def testQueryLocal(self):
        info = wreport.varinfo("t")
        self.assertEqual(info.unit, "K")

    def testData(self):
        info = wreport.varinfo("B01001")
        self.assertEqual(info.var, "B01001")
        self.assertEqual(info.desc, "WMO BLOCK NUMBER")
        self.assertEqual(info.unit, "NUMERIC")
        self.assertEqual(info.scale, 0)
        self.assertEqual(info.ref, 0)
        self.assertEqual(info.len, 3)
        self.assertEqual(info.is_string, False)

    def testStringification(self):
        info = wreport.varinfo("B01001")
        self.assertTrue(str(info).startswith("B01001"))
        self.assertTrue(repr(info).startswith("Varinfo('B01001"))

    def testFromAlias(self):
        info = wreport.varinfo("t")
        self.assertEqual(info.var, "B12101")


if __name__ == "__main__":
    from testlib import main
    main("test_varinfo")
