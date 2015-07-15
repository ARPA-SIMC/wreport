#!/usr/bin/python
# coding: utf-8
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import wreport
import unittest
from six import string_types

class Var(unittest.TestCase):
    def testUndefCreation(self):
        var = wreport.var("B01001")
        self.assertEqual(var.code, "B01001")
        self.assertEqual(var.info.var, "B01001")
        self.assertFalse(var.isset)
    def testIntCreation(self):
        var = wreport.var("B05001", 12)
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 12)
        self.assertEqual(var.enqd(), 0.00012)
        self.assertEqual(var.enqc(), "12")
    def testFloatCreation(self):
        var = wreport.var("B05001", 12.4)
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 1240000)
        self.assertEqual(var.enqd(), 12.4)
        self.assertEqual(var.enqc(), "1240000")
    def testStringCreation(self):
        var = wreport.var("B05001", "123456")
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 123456)
        self.assertEqual(var.enqd(), 1.23456)
        self.assertEqual(var.enqc(), "123456")
    def testAliasCreation(self):
        var = wreport.var("t", 280.3)
        self.assertEqual(var.code, "B12101")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 28030)
        self.assertEqual(var.enqd(), 280.3)
        self.assertEqual(var.enqc(), "28030")
    def testStringification(self):
        var = wreport.var("B01001")
        self.assertEqual(str(var), "None")
        self.assertEqual(repr(var), "Var('B01001', None)")
        self.assertEqual(var.format(), "")
        self.assertEqual(var.format("foo"), "foo")

        var = wreport.var("B05001", 12.4)
        self.assertEqual(str(var), "12.40000")
        self.assertEqual(repr(var), "Var('B05001', 12.40000)")
        self.assertEqual(var.format("foo"), "12.40000")
    def testEnq(self):
        var = wreport.var("B01001", 1)
        self.assertEqual(type(var.enq()), int)
        self.assertEqual(var.enq(), 1)
        var = wreport.var("B05001", 1.12345)
        self.assertEqual(type(var.enq()), float)
        self.assertEqual(var.enq(), 1.12345)
        var = wreport.var("B01019", "ciao")
        self.assertIsInstance(var.enq(), string_types)
        self.assertEqual(var.enq(), "ciao")
    def testGet(self):
        var = wreport.var("B01001")
        self.assertIsNone(var.get())
        self.assertIs(var.get("foo"), "foo")
        var = wreport.var("B01001", 1)
        self.assertIs(var.get(), 1)
        var = wreport.var("B05001", 1.12345)
        self.assertEqual(var.get(), 1.12345)
        var = wreport.var("B01019", "ciao")
        self.assertEqual(var.get(), "ciao")
    def testEq(self):
        var = wreport.var("B01001", 1)
        self.assertEqual(var, var)
        self.assertEqual(var, wreport.var("B01001", 1))
        self.assertNotEqual(var, wreport.var("B01001", 2))
        self.assertNotEqual(var, wreport.var("B01002", 1))
        self.assertIsNot(var, None)
        self.assertIsNot(wreport.var("B01001"), None)

if __name__ == "__main__":
    from testlib import main
    main("var")
