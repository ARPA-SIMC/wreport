#!/usr/bin/python3
import wreport
import unittest


class Var(unittest.TestCase):
    def setUp(self):
        self.table = wreport.Vartable.get_bufr(master_table_version_number=24)

    def testUndefCreation(self):
        var = wreport.Var(self.table["B01001"])
        self.assertEqual(var.code, "B01001")
        self.assertEqual(var.info.code, "B01001")
        self.assertFalse(var.isset)

    def testIntCreation(self):
        var = wreport.Var(self.table["B05001"], 12)
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 12)
        self.assertEqual(var.enqd(), 0.00012)
        self.assertEqual(var.enqc(), "12")

    def testFloatCreation(self):
        var = wreport.Var(self.table["B05001"], 12.4)
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 1240000)
        self.assertEqual(var.enqd(), 12.4)
        self.assertEqual(var.enqc(), "1240000")

    def testStringCreation(self):
        var = wreport.Var(self.table["B05001"], "123456")
        self.assertEqual(var.code, "B05001")
        self.assertEqual(var.isset, True)
        self.assertEqual(var.enqi(), 123456)
        self.assertEqual(var.enqd(), 1.23456)
        self.assertEqual(var.enqc(), "123456")

    def testStringification(self):
        var = wreport.Var(self.table["B01001"])
        self.assertEqual(str(var), "None")
        self.assertEqual(repr(var), "Var('B01001', None)")
        self.assertEqual(var.format(), "")
        self.assertEqual(var.format("foo"), "foo")

        var = wreport.Var(self.table["B05001"], 12.4)
        self.assertEqual(str(var), "12.40000")
        self.assertEqual(repr(var), "Var('B05001', 12.40000)")
        self.assertEqual(var.format("foo"), "12.40000")

    def testEnq(self):
        var = wreport.Var(self.table["B01001"], 1)
        self.assertEqual(type(var.enq()), int)
        self.assertEqual(var.enq(), 1)
        var = wreport.Var(self.table["B05001"], 1.12345)
        self.assertEqual(type(var.enq()), float)
        self.assertEqual(var.enq(), 1.12345)
        var = wreport.Var(self.table["B01019"], "ciao")
        self.assertIsInstance(var.enq(), str)
        self.assertEqual(var.enq(), "ciao")

    def testGet(self):
        var = wreport.Var(self.table["B01001"])
        self.assertIsNone(var.get())
        self.assertIs(var.get("foo"), "foo")
        var = wreport.Var(self.table["B01001"], 1)
        self.assertIs(var.get(), 1)
        var = wreport.Var(self.table["B05001"], 1.12345)
        self.assertEqual(var.get(), 1.12345)
        var = wreport.Var(self.table["B01019"], "ciao")
        self.assertEqual(var.get(), "ciao")

    def testEq(self):
        var = wreport.Var(self.table["B01001"], 1)
        self.assertEqual(var, var)
        self.assertEqual(var, wreport.Var(self.table["B01001"], 1))
        self.assertNotEqual(var, wreport.Var(self.table["B01001"], 2))
        self.assertNotEqual(var, wreport.Var(self.table["B01002"], 1))
        self.assertIsNot(var, None)
        self.assertIsNot(wreport.Var(self.table["B01001"]), None)

    def testAttrs(self):
        var = wreport.Var(self.table["B01001"], 1)

        # Querying a nonexisting attribute returns None
        self.assertIsNone(var.enqa("B33007"))

        # Querying attributes when there are none returns an empty list
        self.assertEqual(var.get_attrs(), [])

        # Removing a non existing attribute does nothing
        var.unseta("B33007")

        # Add an attribute
        var.seta(wreport.Var(self.table["B33007"], 50))

        # Query it
        attr = var.enqa("B33007")
        self.assertEqual(attr.code, "B33007")
        self.assertEqual(attr.enqd(), 50.0)

        # List attributes
        attrs = var.get_attrs()
        self.assertEqual(len(attrs), 1)
        self.assertEqual(attrs[0].code, "B33007")
        self.assertEqual(attrs[0].enqd(), 50.0)

        # Remove it
        var.unseta("B33007")
        self.assertIsNone(var.enqa("B33007"))
        self.assertEqual(var.get_attrs(), [])


if __name__ == "__main__":
    from testlib import main
    main("var")
