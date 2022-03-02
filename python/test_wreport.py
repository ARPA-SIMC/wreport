#!/usr/bin/python3
import wreport
import unittest


class Wreport(unittest.TestCase):
    def test_convert_units(self):
        self.assertAlmostEqual(wreport.convert_units("K", "C", 273.15), 0.0, places=4)
